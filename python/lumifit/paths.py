"""
This module generates the paths for the reconstruction and alignment output folders.
Since there are too many possible combinations of cuts and alignments, it is not elegant
to store them all in the experiment config file. 

Instead, the paths are generated *deterministically from* the experiment config. 

They should ONLY be generated with this module.
"""

from pathlib import Path

from lumifit.types import (
    AlignmentParameters,
    ConfigPackage,
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

    Example: 1-100_xy_n_cut/no_alignment_correction/
    """

    reco_dirname_suffix = f"{recoParams.low_index}-" + f"{recoParams.low_index + recoParams.num_samples - 1}_"

    reco_dirname_suffix += generateCutKeyword(recoParams)
    reco_dirname_suffix += __generateAlignPathKeyword(alignParams)

    return reco_dirname_suffix


def generateAbsoluteROOTDataPath(configPackage: ConfigPackage) -> Path:
    """
    Generates the absolute path to the ROOT files for this config package.
    That includes lmdFitDataDir/scenarioDir/cut/align/

    Example: /lustre/klasen/LMD-xkcdsmbc/sim/1-100_xy_n_cut/no_alignment_correction/
    """

    return configPackage.baseDataDir / generateRecoDirSuffix(configPackage.recoParams, configPackage.alignParams)
