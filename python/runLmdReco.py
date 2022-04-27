#!/usr/bin/env python3

import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import (
    load_params_from_file,
    check_stage_success,
)
from lumifit.reconstruction import ReconstructionParameters
from lumifit.general import toCbool


lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]
LMDScriptPath = os.environ["LMDFIT_SCRIPTPATH"]
PNDmacropath = os.environ["LMDFIT_MACROPATH"]

pathToTrkQAFiles = os.environ["pathname"]
relativeDirToTrksQAFiles = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
force_level = int(os.environ["force_level"])

debug = True
filename_index = 1
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

if not os.path.isdir(pathToTrkQAFiles):
    os.makedirs(pathToTrkQAFiles)

# TODO: check if params are loaded correctly, shouldn't be the specified file name be used?
reco_params = ReconstructionParameters(
    **load_params_from_file(pathToTrkQAFiles + "/reco_params.config")
)

# TODO: read alignment parameters correctly
ali_params = AlignmentParameters()

verbositylvl: int = 0
start_evt: int = reco_params.num_events_per_sample * filename_index

if debug:
    workpathname = pathToTrkQAFiles
    path_mc_data = workpathname
else:
    workpathname = f"/localscratch/{os.environ['SLURM_JOB_ID']}/{relativeDirToTrksQAFiles}"

gen_filepath = workpathname + "/gen_mc.root"

# switch on "missing plane" search algorithm
misspl = True

# use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trkcut = True
if ali_params.alignment_matrices_path == "":
    trkcut = False

# merge hits on sensors from different sides. true=yes
mergedHits = True
# BOX cut before back-propagation
BoxCut = False
# Write all MC info in TrkQA array
WrAllMC = True

radLength = 0.32


prefilter = False

# TODO: in the original, this was called KinematicsCut, but what is that?
KinematicsCut = reco_params.use_m_cut  # off by default?
if reco_params.use_xy_cut:  # off by default?
    prefilter = True

# TODO: get all these from config file
CleanSig = reco_params.use_m_cut  # off by default?

# echo "Kinematics cut (@LMD): $KinematicsCut"
# echo "Momentum cut (after backpropagation): $CleanSig"

# TODO check if the prefilter is off at any time, add to recoparams in that case


# track fit:
# Possible options: "Minuit", "KalmanGeane", "KalmanRK"
trackFitAlgorithm = "Minuit"

# back-propgation GEANE
# Possible options: "Geane", "RK"
backPropAlgorithm = "Geane"


# * ------------------- Special Case for xy Cut: -------------------
# * the second time this script is run, the Lumi_Digi files are needed and must be copied
# this is pretty ugly, but I don't know a better way yet. If they're not already here, the
# macro will fail either way, so I guess it's no harm to copy it.
# searchPath = Path(pathToTrkQAFiles).parent.parent
# candidates = sorted(searchPath.glob(f"**/Lumi_digi_{start_evt}.root"))
# pathToLumiDigi = candidates[0]

# I'm not sure target path exists yet, but we need it to copy the lumu params file to
if not os.path.isdir(workpathname):
    os.makedirs(workpathname)

# we always need the Lumi_(Params|MC) file, no matter what (except if it already exists)
if not check_stage_success(f"{workpathname}/Lumi_Params_{start_evt}.root"):
    os.system(
        f"cp {path_mc_data}/Lumi_Params_{start_evt}.root {workpathname}/Lumi_Params_{start_evt}.root "
    )
    os.system(
        f"cp {path_mc_data}/Lumi_MC_{start_evt}.root {workpathname}/Lumi_MC_{start_evt}.root "
    )


if not check_stage_success(f"{workpathname}/Lumi_digi_{start_evt}.root"):
    if os.path.exists(f"{path_mc_data}/Lumi_digi_{start_evt}.root"):
        # copy the Lumi_Digi data from permanent storage, it's needed for IP cut for the LumiFit
        os.system(
            f"cp {path_mc_data}/Lumi_digi_{start_evt}.root {workpathname}/Lumi_digi_{start_evt}.root"
        )


os.chdir(PNDmacropath)
# * ------------------- Reco Step -------------------
if (
    not check_stage_success(pathToTrkQAFiles + f"/Lumi_reco_{start_evt}.root")
    or force_level == 1
):
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel2Reco.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", "{ali_params.alignment_matrices_path}", "{ali_params.misalignment_matrices_path}", {toCbool(ali_params.use_point_transform_misalignment)}, {verbositylvl})'"""
    )

# * ------------------- Hit Merge Step -------------------
if (
    not check_stage_success(
        workpathname + f"/Lumi_recoMerged_{start_evt}.root"
    )
    or force_level == 1
):
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel2bHitMerge.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl})'"""
    )

    # copy Lumi_recoMerged_ for module aligner
    os.system(
        f"cp {workpathname}/Lumi_recoMerged_{start_evt}.root {pathToTrkQAFiles}/Lumi_recoMerged_{start_evt}.root"
    )

# * ------------------- Pair Finder Step -------------------

# TODO: store this in configuration, we don't always need hit pairs
if True:
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel2ePairFinder.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl})'"""
    )

    os.system(
        f"""cp {workpathname}/Lumi_Pairs_{start_evt}.root {pathToTrkQAFiles}/Lumi_Pairs_{start_evt}.root"""
    )


# * ------------------- Pixel Finder Step -------------------
if (
    not check_stage_success(f"{workpathname}/Lumi_TCand_{start_evt}.root")
    or force_level == 1
):
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel3Finder.C({reco_params.num_events_per_sample},{start_evt},"{workpathname}",{verbositylvl},"{reco_params.track_search_algo}",{int(misspl)},{int(mergedHits)}, {int(trkcut)}, {reco_params.lab_momentum})'"""
    )


# * ------------------- Pixel Fitter Step -------------------
if (
    not check_stage_success(
        f"{workpathname}/Lumi_TrackNotFiltered_{start_evt}.root"
    )
    or force_level == 1
):
    if (
        not check_stage_success(f"{workpathname}/Lumi_Track_{start_evt}.root")
        or force_level == 1
    ):
        os.chdir(PNDmacropath)
        # this script outputs a Lumi_Track_... file. Rename that to the NotFiltered..
        os.system(
            f"""root -l -b -q 'runLumiPixel4Fitter.C({reco_params.num_events_per_sample}, {start_evt},"{workpathname}", {verbositylvl}, "{trackFitAlgorithm}", {toCbool(mergedHits)})'"""
        )

        # copy track file for module alignment
        os.system(
            f"""cp {workpathname}/Lumi_Track_{start_evt}.root {pathToTrkQAFiles}/Lumi_Track_{start_evt}.root"""
        )

# * ------------------- Pixel Filter Step -------------------
# track filter (on number of hits and chi2 and optionally on track kinematics)
# so yes, this is the kinematics and xy filter step. It's a little convoluted:
if prefilter:
    """
    This is a little convoluted, so here are the steps:
    1. move (rename) Tracks file to TrackNotFiltered
    2, --removed--
    3. check if TrackFiltered (NOT TrackNotFiltered) is already there
    4. if not:
        1. create new symlink called TrackNotFiltered which points to Lumi_Track (root macro needs this name)
        2. run root macro, it creates a Lumi_TrackFiltered_ file
        3. overwrite the previous symlink Lumi_Track so that it now points to TrackFiltered_
    """

    os.system(
        f"""mv {workpathname}/Lumi_Track_{start_evt}.root {workpathname}/Lumi_TrackNotFiltered_{start_evt}.root"""
    )

    if (
        not check_stage_success(
            f"{workpathname}/Lumi_TrackFiltered_{start_evt}.root"
        )
        or force_level == 1
    ):
        # this macro needs Lumi_Track_... file as input so we need to link the unfiltered file
        os.system(
            f"""ln -sf {workpathname}/Lumi_TrackNotFiltered_{start_evt}.root {workpathname}/Lumi_Track_{start_evt}.root"""
        )

        reco_params.reco_ip_offset[0]
        os.chdir(PNDmacropath)
        os.system(
            f"""root -l -b -q 'runLumiPixel4aFilter.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, {toCbool(mergedHits)}, {reco_params.lab_momentum}, {toCbool(KinematicsCut)}, {reco_params.reco_ip_offset[0]}, {reco_params.reco_ip_offset[1]})'"""
        )

        # now overwrite the Lumi_Track_ sym link with the filtered version
        os.system(
            f"""ln -sf {workpathname}/Lumi_TrackFiltered_{start_evt}.root {workpathname}/Lumi_Track_{start_evt}.root"""
        )

        # don't copy to permanent storage, but don't delete this comment just yet
        if False:
            os.system(
                f"""cp {workpathname}/Lumi_Track_{start_evt}.root {pathToTrkQAFiles}/Lumi_TrackFiltered_{start_evt}.root"""
            )

# * ------------------- Pixel BackProp Step -------------------
if (
    not check_stage_success(f"{workpathname}/Lumi_Geane_{start_evt}.root")
    or force_level == 1
):
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel5BackProp.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, "{backPropAlgorithm}", {toCbool(mergedHits)}, {reco_params.lab_momentum}, {reco_params.reco_ip_offset[0]}, {reco_params.reco_ip_offset[1]}, {reco_params.reco_ip_offset[2]}, {toCbool(prefilter)})'"""
    )

# * ------------------- Pixel CleanSig Step -------------------
# filter back-propagated tracks (momentum cut)
if CleanSig:
    os.system(
        f"""root -l -b -q 'runLumiPixel5bCleanSig.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, {reco_params.lab_momentum}, {reco_params.reco_ip_offset[0]}, {reco_params.reco_ip_offset[1]})'"""
    )

# * ------------------- Pixel Track QA Step -------------------
# Quality assurance task(s)
# combine MC and reco information
# the last parameter is mc all write flag and needs to be true
# so that all mc events are written even if geometrically missing the sensors
# this is required for the acceptance calculation
if (
    not check_stage_success(f"{workpathname}/Lumi_TrksQA_{start_evt}.root")
    or force_level == 1
):
    os.system(
        f"""root -l -b -q 'runLumiPixel7TrksQA.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, {reco_params.lab_momentum}, {toCbool(WrAllMC)}, {toCbool(reco_params.use_xy_cut)}, {toCbool(reco_params.use_m_cut)}, {toCbool(CleanSig)})'"""
    )
    if not debug:
        os.system(
            f"""cp {workpathname}/Lumi_TrksQA_{start_evt}.root {pathToTrkQAFiles}/Lumi_TrksQA_{start_evt}.root"""
        )

# * ------------------- Cleanup Step -------------------
# remove everything in the local path
if not debug:
    os.system(f"""rm -f {workpathname}/Lumi_*_{start_evt}.root""")

    if len(os.listdir(workpathname)) == 0:
        os.system(f"""rm -rf {workpathname}""")
