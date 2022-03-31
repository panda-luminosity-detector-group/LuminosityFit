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

# TODO: check if params are loaded correctly, shouldn't be the specified file name be uses?
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

os.chdir(macropath)
if (
    not check_stage_success(pathname + f"/Lumi_MC_{start_evt}.root")
    or force_level == 1
):
    os.system(
        f"""root -l -b -q 'runLumiPixel2Reco.C({reco_params.num_events_per_sample},{start_evt}, "{workpathname}", "{ali_params.alignment_matrices_path}", "{ali_params.misalignment_matrices_path}", {ali_params.use_point_transform_misalignment}, {verbositylvl})'"""
    )

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

os.chdir(scriptpath)
os.system("./runLmdPairFinder.sh")
os.system("./runLmdTrackFinder.sh")
