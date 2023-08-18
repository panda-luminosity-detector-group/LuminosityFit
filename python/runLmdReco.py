#!/usr/bin/env python3

"""
This script is typically run on a compute node. Therefore ALL neccessary info
must be in env variables or in the config file. It is not possible to convert it to
a module (sadly).

The good thing is that this script can not change the experiment config inadvertently.
"""

import os
from pathlib import Path

from lumifit.config import load_params_from_file
from lumifit.general import (
    envPath,
    isFilePresentAndValid,
    matrixMacroFileName,
    toCbool,
)
from lumifit.paths import generateAbsoluteROOTDataPath
from lumifit.types import DataMode, ExperimentParameters

# * ------------------- Experiment Parameters -------------------
experimentDir = envPath("ExperimentDir")
thisMode = DataMode(os.environ["DataMode"])
force_level = int(os.environ["force_level"])

experiment: ExperimentParameters = load_params_from_file(experimentDir / "experiment.config", ExperimentParameters)

if thisMode == DataMode.DATA:
    configPackage = experiment.dataPackage
elif thisMode == DataMode.VERTEXDATA:
    configPackage = experiment.dataPackage
    configPackage.recoParams.disableCuts()
elif thisMode == DataMode.RESACC:
    configPackage = experiment.resAccPackage
else:
    raise NotImplementedError(f"DataMode {thisMode} not implemented")

assert configPackage.simParams is not None
assert configPackage.MCDataDir is not None

MCDataDir = configPackage.MCDataDir
recoParams = configPackage.recoParams
alignParams = configPackage.alignParams
pathToTrkQAFiles = generateAbsoluteROOTDataPath(configPackage=configPackage)
PNDmacropath = experiment.softwarePaths.PandaRootMacroPath
relativeDirToTrksQAFilesOnComputeNode = "LMD-TempRootFiles"

debug = True
filename_index = 1
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

pathToTrkQAFiles.mkdir(parents=True, exist_ok=True)
(pathToTrkQAFiles / "Pairs").mkdir(parents=True, exist_ok=True)

verbositylvl: int = 0
start_evt: int = recoParams.num_events_per_sample * filename_index

if debug:
    workingDirOnComputeNode = pathToTrkQAFiles
    MCDataDir = workingDirOnComputeNode
else:
    workingDirOnComputeNode = Path(f"/localscratch/{os.environ['SLURM_JOB_ID']}/{relativeDirToTrksQAFilesOnComputeNode}")

generatedMCdataFilePath = workingDirOnComputeNode / "gen_mc.root"

# switch on "missing plane" search algorithm
missingPlanes = True

# use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trackCuts = True
if alignParams.alignment_matrices_path is None:
    trackCuts = False

# merge hits on sensors from different sides. true=yes
mergedHits = True
# BOX cut before back-propagation
boxCut = False
# Write all MC info in TrkQA array
writeAllMCinfoToTrkQA = True

radLength = 0.32

preFilter = False

# TODO: in the original, this was called KinematicsCut, but what is that?
KinematicsCut = recoParams.use_m_cut  # off by default?
if recoParams.use_xy_cut:  # off by default?
    preFilter = True

# TODO: get all these from config file
CleanSig = recoParams.use_m_cut  # off by default?

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
workingDirOnComputeNode.mkdir(parents=True, exist_ok=True)

# we always need the Lumi_(Params|MC) file, no matter what (except if it already exists)
if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_Params_{start_evt}.root")):
    os.system(f"cp {MCDataDir}/Lumi_Params_{start_evt}.root {workingDirOnComputeNode}/Lumi_Params_{start_evt}.root ")
    os.system(f"cp {MCDataDir}/Lumi_MC_{start_evt}.root {workingDirOnComputeNode}/Lumi_MC_{start_evt}.root ")


if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_digi_{start_evt}.root")):
    if os.path.exists(f"{MCDataDir}/Lumi_digi_{start_evt}.root"):
        # copy the Lumi_Digi data from permanent storage, it's needed for IP cut for the LumiFit
        os.system(f"cp {MCDataDir}/Lumi_digi_{start_evt}.root {workingDirOnComputeNode}/Lumi_digi_{start_evt}.root")


os.chdir(PNDmacropath)
# * ------------------- Reco Step -------------------
if not isFilePresentAndValid(Path(pathToTrkQAFiles / f"Lumi_reco_{start_evt}.root")) or force_level == 1:
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel2Reco.C({recoParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", "{matrixMacroFileName(alignParams.alignment_matrices_path)}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
    )

# * ------------------- Hit Merge Step -------------------
if not isFilePresentAndValid(Path(workingDirOnComputeNode / f"Lumi_recoMerged_{start_evt}.root")) or force_level == 1:
    os.chdir(PNDmacropath)
    os.system(f"""root -l -b -q 'runLumiPixel2bHitMerge.C({recoParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", {verbositylvl})'""")

    # copy Lumi_recoMerged_ for module aligner
    os.system(f"cp {workingDirOnComputeNode}/Lumi_recoMerged_{start_evt}.root {pathToTrkQAFiles}/Lumi_recoMerged_{start_evt}.root")

# * ------------------- Pair Finder Step -------------------

# TODO: store this in configuration, we don't always need hit pairs
if True:
    os.chdir(PNDmacropath)
    os.system(f"""root -l -b -q 'runLumiPixel2ePairFinder.C({recoParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", {verbositylvl})'""")

    os.system(f"""cp {workingDirOnComputeNode}/Lumi_Pairs_{start_evt}.root {pathToTrkQAFiles}/Lumi_Pairs_{start_evt}.root""")


# * ------------------- Pixel Finder Step -------------------
if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_TCand_{start_evt}.root")) or force_level == 1:
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel3Finder.C({recoParams.num_events_per_sample},{start_evt},"{workingDirOnComputeNode}",{verbositylvl},"{recoParams.track_search_algo}",{int(missingPlanes)},{int(mergedHits)}, {int(trackCuts)}, {recoParams.lab_momentum})'"""
    )


# * ------------------- Pixel Fitter Step -------------------
if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_TrackNotFiltered_{start_evt}.root")) or force_level == 1:
    if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_Track_{start_evt}.root")) or force_level == 1:
        os.chdir(PNDmacropath)
        # this script outputs a Lumi_Track_... file. Rename that to the NotFiltered..
        os.system(
            f"""root -l -b -q 'runLumiPixel4Fitter.C({recoParams.num_events_per_sample}, {start_evt},"{workingDirOnComputeNode}", {verbositylvl}, "{trackFitAlgorithm}", {toCbool(mergedHits)})'"""
        )

        # copy track file for module alignment
        os.system(f"""cp {workingDirOnComputeNode}/Lumi_Track_{start_evt}.root {pathToTrkQAFiles}/Lumi_Track_{start_evt}.root""")

# * ------------------- Pixel Filter Step -------------------
# track filter (on number of hits and chi2 and optionally on track kinematics)
# so yes, this is the kinematics and xy filter step. It's a little convoluted:
if preFilter:
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

    os.system(f"""mv {workingDirOnComputeNode}/Lumi_Track_{start_evt}.root {workingDirOnComputeNode}/Lumi_TrackNotFiltered_{start_evt}.root""")

    if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_TrackFiltered_{start_evt}.root")) or force_level == 1:
        # this macro needs Lumi_Track_... file as input so we need to link the unfiltered file
        os.system(f"""ln -sf {workingDirOnComputeNode}/Lumi_TrackNotFiltered_{start_evt}.root {workingDirOnComputeNode}/Lumi_Track_{start_evt}.root""")

        # recoParams.recoIPX # the hell?
        os.chdir(PNDmacropath)
        os.system(
            f"""root -l -b -q 'runLumiPixel4aFilter.C({recoParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", {verbositylvl}, {toCbool(mergedHits)}, {recoParams.lab_momentum}, {toCbool(KinematicsCut)}, {recoParams.recoIPX}, {recoParams.recoIPY})'"""
        )

        # now overwrite the Lumi_Track_ sym link with the filtered version
        os.system(f"""ln -sf {workingDirOnComputeNode}/Lumi_TrackFiltered_{start_evt}.root {workingDirOnComputeNode}/Lumi_Track_{start_evt}.root""")

        # don't copy to permanent storage, but don't delete this comment just yet
        if False:
            os.system(f"""cp {workingDirOnComputeNode}/Lumi_Track_{start_evt}.root {pathToTrkQAFiles}/Lumi_TrackFiltered_{start_evt}.root""")

# * ------------------- Pixel BackProp Step -------------------
if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_Geane_{start_evt}.root")) or force_level == 1:
    os.chdir(PNDmacropath)
    os.system(
        f"""root -l -b -q 'runLumiPixel5BackProp.C({recoParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", {verbositylvl}, "{backPropAlgorithm}", {toCbool(mergedHits)}, {recoParams.lab_momentum}, {recoParams.recoIPX}, {recoParams.recoIPY}, {recoParams.recoIPZ}, {toCbool(preFilter)})'"""
    )

# * ------------------- Pixel CleanSig Step -------------------
# filter back-propagated tracks (momentum cut)
if CleanSig:
    os.system(
        f"""root -l -b -q 'runLumiPixel5bCleanSig.C({recoParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", {verbositylvl}, {recoParams.lab_momentum}, {recoParams.recoIPX}, {recoParams.recoIPY})'"""
    )

# * ------------------- Pixel Track QA Step -------------------
# Quality assurance task(s)
# combine MC and reco information
# the last parameter is mc all write flag and needs to be true
# so that all mc events are written even if geometrically missing the sensors
# this is required for the acceptance calculation
if not isFilePresentAndValid(Path(f"{workingDirOnComputeNode}/Lumi_TrksQA_{start_evt}.root")) or force_level == 1:
    os.system(
        f"""root -l -b -q 'runLumiPixel7TrksQA.C({recoParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", {verbositylvl}, {recoParams.lab_momentum}, {toCbool(writeAllMCinfoToTrkQA)}, {toCbool(recoParams.use_xy_cut)}, {toCbool(recoParams.use_m_cut)}, {toCbool(CleanSig)})'"""
    )
    if not debug:
        os.system(f"""cp {workingDirOnComputeNode}/Lumi_TrksQA_{start_evt}.root {pathToTrkQAFiles}/Lumi_TrksQA_{start_evt}.root""")

# * ------------------- Cleanup Step -------------------
# remove everything in the local path
if not debug:
    os.system(f"""rm -f {workingDirOnComputeNode}/Lumi_*_{start_evt}.root""")

    if len(os.listdir(workingDirOnComputeNode)) == 0:
        os.system(f"""rm -rf {workingDirOnComputeNode}""")
