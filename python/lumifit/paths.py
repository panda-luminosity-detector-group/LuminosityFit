from lumifit.types import AlignmentParameters, ReconstructionParameters


def generateCutKeyword(reco_params: ReconstructionParameters) -> str:
    cutKeyword = ""

    if reco_params.use_xy_cut:
        cutKeyword += "xy_"
    if reco_params.use_m_cut:
        cutKeyword += "m_"
    if not reco_params.use_xy_cut and not reco_params.use_m_cut:
        cutKeyword += "un"
    cutKeyword += "cut"
    if reco_params.use_xy_cut and reco_params.use_ip_determination:
        cutKeyword += "_real"
    return cutKeyword


def generateAlignPathKeyword(align_params: AlignmentParameters) -> str:
    alignPathKeyword = ""
    if align_params.use_point_transform_misalignment:
        if not align_params.misalignment_matrices_path:
            alignPathKeyword += "_no_data_misalignment"
        else:
            alignPathKeyword += "_" + align_params.misalignment_matrices_path.stem
    if align_params.alignment_matrices_path:
        alignPathKeyword += "/aligned-" + align_params.alignment_matrices_path.stem
    else:
        alignPathKeyword += "/no_alignment_correction"
    return alignPathKeyword


def generateRecoDirSuffix(reco_params: ReconstructionParameters, align_params: AlignmentParameters) -> str:
    reco_dirname_suffix = f"{reco_params.low_index}-" + f"{reco_params.low_index + reco_params.num_samples - 1}_"

    reco_dirname_suffix += generateCutKeyword(reco_params)
    reco_dirname_suffix += generateAlignPathKeyword(align_params)

    return reco_dirname_suffix
