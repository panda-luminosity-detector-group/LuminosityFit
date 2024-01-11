"""
This module generates the paths for the reconstruction and alignment output folders.
Since there are too many possible combinations of cuts and alignments, it is not elegant
to store them all in the experiment config file. 

Instead, the paths are generated *deterministically from* the experiment config. 

They should ONLY be generated with this module.
"""
import copy
from pathlib import Path

from lumifit.types import (
    AlignmentParameters,
    ConfigPackage,
    DataMode,
    ReconstructionParameters,
)


def generateCutKeyword(recoParams: ReconstructionParameters) -> str:
    """
    Generates the cut-part of the subdirectory for the reconstruction data directory.
    This does not include alignment information and is NOT a complete subdirectory.

    Examples: xy_m_cut, uncut, xy_m_cut_real
    """
    cutKeyword = ""

    if recoParams.use_xy_cut:
        cutKeyword += "xy_"
    if recoParams.use_m_cut:
        cutKeyword += "m_"
    if not recoParams.use_xy_cut and not recoParams.use_m_cut:
        cutKeyword += "un"
    cutKeyword += "cut"
    if recoParams.use_xy_cut and recoParams.use_ip_determination:
        cutKeyword += "_real"
    return cutKeyword


def __generateAlignPathKeyword(alignParams: AlignmentParameters) -> str:
    alignPathKeyword = ""
    if alignParams.use_point_transform_misalignment:
        if not alignParams.misalignment_matrices_path:
            alignPathKeyword += "_no_data_misalignment"
        else:
            alignPathKeyword += "_" + alignParams.misalignment_matrices_path.stem
    if alignParams.alignment_matrices_path:
        alignPathKeyword += "/aligned-" + alignParams.alignment_matrices_path.stem
    else:
        alignPathKeyword += "/no_alignment_correction"
    return alignPathKeyword


def generateRecoDirSuffix(recoParams: ReconstructionParameters, alignParams: AlignmentParameters) -> str:
    """
    This generates the relative path suffix for the reconstruction data directory, including the cut and alignment information.

    Example: reco_xy_n_cut/no_alignment_correction/
    """

    reco_dirname_suffix = "reco_"
    reco_dirname_suffix += generateCutKeyword(recoParams)
    reco_dirname_suffix += __generateAlignPathKeyword(alignParams)

    return reco_dirname_suffix


def generateAbsoluteROOTDataPath(configPackage: ConfigPackage, dataMode=DataMode.DATA) -> Path:
    """
    Generates the absolute path to the ROOT files for this config package.
    That includes lmdFitDataDir/scenarioDir/cut/align/

    Example: /lustre/klasen/LMD-xkcdsmbc/sim/1-100_xy_n_cut/no_alignment_correction/

    dataMode decides if the recoParams should really use cuts or not. 
    if dataMode == DataMode.VERTEXDATA, cuts are disabled.
    (not ideal design, but works for now)
    """

    if dataMode == DataMode.VERTEXDATA:
        # remember, recoParams is passed by reference, NEVER change the original
        recoParams = copy.deepcopy(configPackage.recoParams)
        recoParams.disableCuts()
    else:
        recoParams = configPackage.recoParams

    return configPackage.baseDataDir / generateRecoDirSuffix(recoParams, configPackage.alignParams)


def generateRelativeBunchesDir() -> Path:
    """
    Generates a relative path to a bunches subdirectory.
    """
    return Path("bunches")


def generateRelativeBinningDir() -> Path:
    """
    Generates a relative path to a bunches subdirectory.
    """
    return Path("binning")


def generateRelativeMergeDir() -> Path:
    """
    Generates a relative path to the merge_data path,
    where res/acc/lmd data should be stored
    """
    return Path("merge_data")


def generateAbsoluteMergeDataPath(configPackage: ConfigPackage, dataMode=DataMode.DATA) -> Path:
    return (
        generateAbsoluteROOTDataPath(configPackage=configPackage, dataMode=dataMode)
        / generateRelativeBunchesDir()
        / generateRelativeBinningDir()
        / generateRelativeMergeDir()
    )
