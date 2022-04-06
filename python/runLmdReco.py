#!/usr/bin/env python3

import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file, check_stage_success
from lumifit.reconstruction import ReconstructionParameters

lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]
scriptpath = lmd_build_path + "/../python"
dirname = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
pathname = os.environ["pathname"]
macropath = os.environ["macropath"]
force_level = int(os.environ["force_level"])

debug = True
filename_index = 1
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

if not os.path.isdir(pathname):
    os.makedirs(pathname)

# TODO: check if params are loaded correctly, shouldn't be the specified file name be used?
reco_params = ReconstructionParameters(
    **load_params_from_file(pathname + "/reco_params.config")
)

# TODO: read alignment parameters correctly
ali_params = AlignmentParameters()

verbositylvl: int = 0
start_evt: int = reco_params.num_events_per_sample * filename_index

if debug:
    workpathname = pathname
    path_mc_data = workpathname
else:
    workpathname = f"/localscratch/{os.environ['SLURM_JOB_ID']}/{dirname}"

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

if reco_params.use_xy_cut:
    prefilter = True

# TODO: get all these from config file
CleanSig = True

# echo "Kinematics cut (@LMD): $KinematicsCut"
# echo "Momentum cut (after backpropagation): $CleanSig"

# TODO check if the prefilter is off at any time, add to recoparams in that case

prefilter = 1
KinematicsCut = 1

# track fit:
# Possible options: "Minuit", "KalmanGeane", "KalmanRK"
trackFitAlgorithm = "Minuit"

# back-propgation GEANE
# Possible options: "Geane", "RK"
backPropAlgorithm = "Geane"


os.chdir(macropath)
# * ------------------- Reco Step -------------------
if (
    not check_stage_success(pathname + f"/Lumi_MC_{start_evt}.root")
    or force_level == 1
):
    os.system(
        f"""root -l -b -q 'runLumiPixel2Reco.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", "{ali_params.alignment_matrices_path}", "{ali_params.misalignment_matrices_path}", {1 if ali_params.use_point_transform_misalignment else 0}, {verbositylvl})'"""
    )

# * ------------------- Hit Merge Step -------------------
if (
    not check_stage_success(
        workpathname + f"/Lumi_recoMerged_{start_evt}.root"
    )
    or force_level == 1
):
    os.system(
        f"""root -l -b -q 'runLumiPixel2bHitMerge.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl})'"""
    )

    # copy Lumi_recoMerged_ for module aligner
    os.system(
        f"cp {workpathname}/Lumi_recoMerged_{start_evt}.root {pathname}/Lumi_recoMerged_{start_evt}.root"
    )

# TODO: do this right here, not in some other scripts
# os.chdir(scriptpath)
# os.system("./runLmdPairFinder.sh")
# os.system("./runLmdTrackFinder.sh")

# * ------------------- Pair Finder Step -------------------

# TODO: store this in configuration, we don't always need hit pairs
if True:

    os.system(
        f"""root -l -b -q 'runLumiPixel2ePairFinder.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl})'"""
    )

    os.system(
        f"""cp {workpathname}/Lumi_Pairs_{start_evt}.root {pathname}/Lumi_Pairs_{start_evt}.root"""
    )


# * ------------------- Pixel Finder Step -------------------
if (
    not check_stage_success(f"{workpathname}/Lumi_TCand_{start_evt}.root")
    or force_level == 1
):
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

        # this script outputs a Lumi_Track_... file. Rename that to the NotFiltered..
        os.system(
            f"""root -l -b -q 'runLumiPixel4Fitter.C({reco_params.num_events_per_sample}, {start_evt},"{workpathname}", {verbositylvl}, {trackFitAlgorithm}, {int(mergedHits)})'"""
        )

        # copy track file for module alignment
        os.system(
            f"""cp {workpathname}/Lumi_Track_{start_evt}.root {pathname}/Lumi_Track_{start_evt}.root"""
        )

        if prefilter == 1:
            os.system(
                f"""mv {workpathname}/Lumi_Track_{start_evt}.root {workpathname}/Lumi_TrackNotFiltered_{start_evt}.root"""
            )

# * ------------------- Pixel Filter Step -------------------
# track filter (on number of hits and chi2 and optionally on track kinematics)
if prefilter == 1:
    if (
        not check_stage_success(f"{workpathname}/Lumi_Track_{start_evt}.root")
        or force_level == 1
    ):
        if (
            not check_stage_success(
                f"{workpathname}/Lumi_TrackNotFiltered_{start_evt}.root"
            )
            or force_level == 1
        ):

            # this macro needs Lumi_Track_... file as input so we need to link the unfiltered file
            os.system(
                f"""ln -sf {workpathname}/Lumi_TrackNotFiltered_{start_evt}.root {workpathname}/Lumi_Track_{start_evt}.root"""
            )

            reco_params.reco_ip_offset[0]

            os.system(
                f"""root -l -b -q 'runLumiPixel4aFilter.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, {int(mergedHits)}, {reco_params.lab_momentum}, {KinematicsCut}, {reco_params.reco_ip_offset[0]}, {reco_params.reco_ip_offset[1]})'"""
            )

            # now overwrite the Lumi_Track_ sym link with the filtered version
            os.system(
                f"""ln -sf {workpathname}/Lumi_TrackFiltered_{start_evt}.root {workpathname}/Lumi_Track_{start_evt}.root"""
            )

            # don't copy to permanent storage, but don't delete this comment just yet
            os.system(
                f"""cp {workpathname}/Lumi_Track_{start_evt}.root {pathname}/Lumi_TrackFiltered_{start_evt}.root"""
            )

# * ------------------- Pixel BackProp Step -------------------
if (
    not check_stage_success(f"{workpathname}/Lumi_Geane_{start_evt}.root")
    or force_level == 1
):
    os.system(
        f"""root -l -b -q 'runLumiPixel5BackProp.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}', "{backPropAlgorithm}", {int(mergedHits)}, {reco_params.lab_momentum}, {reco_params.reco_ip_offset[0]}, {reco_params.reco_ip_offset[1]}, {reco_params.reco_ip_offset[2]}, {prefilter})'"""
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
        f"""root -l -b -q 'runLumiPixel7TrksQA.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, {reco_params.lab_momentum}, {int(WrAllMC)}, {KinematicsCut}, {int(CleanSig)})'"""
    )
    if not debug:
        os.system(
            f"""cp {workpathname}/Lumi_TrksQA_{start_evt}.root {pathname}/Lumi_TrksQA_{start_evt}.root"""
        )

# * ------------------- Cleanup Step -------------------
# remove everything in the local path
if not debug:
    os.system(f"""rm -f {workpathname}/Lumi_*_{start_evt}.root""")

    if len(os.listdir(workpathname)) == 0:
        os.system(f"""rm -rf {workpathname}""")
