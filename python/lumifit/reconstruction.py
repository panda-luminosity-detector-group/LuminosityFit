#!/usr/bin/env python3

from pathlib import Path
from typing import Tuple

from lumifit.cluster import (
    Job,
    JobResourceRequest,
    make_test_job_resource_request,
)
from lumifit.config import write_params_to_file
from lumifit.general import envPath
from lumifit.types import AlignmentParameters, ReconstructionParameters

# TODO: solve the track_search_algorithms with Enum (or IntEnum for json serialize-ability)
#! wait there is even an enumEncoder, IntEnums may not be necessary

track_search_algorithms = ["CA", "Follow"]

lmdScriptPath = envPath("LMDFIT_SCRIPTPATH")


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


def create_reconstruction_job(
    reco_params: ReconstructionParameters,
    align_params: AlignmentParameters,
    dirname: Path,
    application_command: str,
    force_level: int = 0,
    debug: bool = False,
    use_devel_queue: bool = False,
) -> Tuple[Job, Path]:
    print("preparing reconstruction in index range " + f"{reco_params.low_index} - " + f"{reco_params.low_index + reco_params.num_samples - 1}")

    dirname_filter_suffix = generateRecoDirSuffix(reco_params, align_params)

    low_index_used = reco_params.low_index
    num_samples = reco_params.num_samples
    if debug and reco_params.num_samples > 1:
        print("Warning: number of samples in debug mode is limited to 1!" " Setting to 1!")
        num_samples = 1

    print(f"dir name for this create_reconstruction_job is {dirname}")

    pathname_base = dirname
    path_mc_data = pathname_base / "mc_data"
    dirname_full = dirname / dirname_filter_suffix
    pathname_full = dirname_full

    print(f"using output folder structure: {pathname_full}")

    pathname_full.mkdir(exist_ok=True, parents=True)
    Path(pathname_full / "Pairs").mkdir(exist_ok=True, parents=True)

    # These must be written again so that runLmdSimReco and runLmdReco have access to them
    write_params_to_file(reco_params, pathname_full, "reco_params.config")
    write_params_to_file(align_params, pathname_full, "align_params.config")

    resource_request = JobResourceRequest(2 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 3500
    resource_request.node_scratch_filesize_in_mb = 0

    if use_devel_queue:
        resource_request = make_test_job_resource_request()

    job = Job(
        resource_request,
        application_url=application_command,
        name="reco_" + reco_params.simGenTypeForResAcc.value,
        logfile_url=str(pathname_full / "reco-%a.log"),
        array_indices=list(range(low_index_used, low_index_used + num_samples)),
    )

    # TODO: these won't be needed anymore the're in the config file
    job.exported_user_variables = {
        "dirname": dirname_full,
        "path_mc_data": path_mc_data,
        "pathname": pathname_full,
        "force_level": str(force_level),
    }

    if reco_params.force_cut_disable:
        job.exported_user_variables["force_cut_disable"] = True

    return (job, pathname_full)
