#!/usr/bin/env python3

import errno
import os
from pathlib import Path
from typing import Any, Tuple, Union

import attr
import cattrs
from attr import field

from .alignment import AlignmentParameters
from .cluster import Job, JobResourceRequest, make_test_job_resource_request
from .general import write_params_to_file
from .simulationTypes import SimulationType

# TODO: solve the track_search_algorithms with Enum (or IntEnum for json serializability)
#! wait there is even an enumEncoder, IntEnums may not be neccessary

track_search_algorithms = ["CA", "Follow"]

lmdScriptPath = os.environ["LMDFIT_SCRIPTPATH"]


@attr.s
class ReconstructionParameters:
    def _validate_track_search_algo(instance: Any, _: Any, value: Any) -> None:
        if value not in track_search_algorithms:
            raise ValueError(f"Must be either of {track_search_algorithms}.")

    sim_type_for_resAcc: SimulationType = attr.ib(default=SimulationType.BOX)
    num_events_per_sample: int = attr.ib(default=1000)
    num_samples: int = attr.ib(default=1)
    lab_momentum: float = attr.ib(default=1.5)
    low_index: int = attr.ib(default=1)
    output_dir: Union[Path, None] = field(default=None)
    lmd_geometry_filename: str = attr.ib(default="Luminosity-Detector.root")
    use_ip_determination: bool = attr.ib(default=True)
    use_xy_cut: bool = attr.ib(default=True)
    use_m_cut: bool = attr.ib(default=True)
    track_search_algo: str = attr.ib(default="CA", validator=_validate_track_search_algo)
    # reco_ip_offset: Tuple[float, float, float] = attr.ib(default=(0, 0, 0))
    recoIPX: float = attr.ib(default=0.0)
    recoIPY: float = attr.ib(default=0.0)
    recoIPZ: float = attr.ib(default=0.0)

    num_box_samples: int = attr.ib(default=500)
    num_events_per_box_sample: int = attr.ib(default=10000)


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


def generateAlignpathKeyword(align_params: AlignmentParameters) -> str:
    alignpathKeyword = ""
    if align_params.use_point_transform_misalignment:
        if align_params.misalignment_matrices_path is None:
            alignpathKeyword += "_no_data_misalignment"
        else:
            alignpathKeyword += "_" + str(os.path.splitext(os.path.basename(align_params.misalignment_matrices_path))[0])
    if align_params.alignment_matrices_path:
        alignpathKeyword += "/aligned-" + str(os.path.splitext(os.path.basename(align_params.alignment_matrices_path))[0])
    else:
        alignpathKeyword += "/no_alignment_correction"
    return alignpathKeyword


def generateRecoDirSuffix(reco_params: ReconstructionParameters, align_params: AlignmentParameters) -> str:
    reco_dirname_suffix = f"{reco_params.low_index}-" + f"{reco_params.low_index + reco_params.num_samples - 1}_"

    reco_dirname_suffix += generateCutKeyword(reco_params)
    reco_dirname_suffix += generateAlignpathKeyword(align_params)

    return reco_dirname_suffix


def create_reconstruction_job(
    reco_params: ReconstructionParameters,
    align_params: AlignmentParameters,
    dirname: str,
    application_command: str,
    force_level: int = 0,
    debug: bool = False,
    use_devel_queue: bool = False,
) -> Tuple[Job, str]:
    print("preparing reconstruction in index range " + f"{reco_params.low_index} - " + f"{reco_params.low_index + reco_params.num_samples - 1}")

    dirname_filter_suffix = generateRecoDirSuffix(reco_params, align_params)

    low_index_used = reco_params.low_index
    num_samples = reco_params.num_samples
    if debug and reco_params.num_samples > 1:
        print("Warning: number of samples in debug mode is limited to 1!" " Setting to 1!")
        num_samples = 1

    print(f"dir name for this create_reconstruction_job is {dirname}")

    pathname_base = dirname
    path_mc_data = pathname_base + "/mc_data"
    dirname_full = dirname + "/" + dirname_filter_suffix
    pathname_full = dirname_full

    print("using output folder structure: " + pathname_full)

    try:
        os.makedirs(pathname_full)
        os.makedirs(pathname_full + "/Pairs")
        os.makedirs(path_mc_data)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            print("error: thought dir does not exists but it does...")

    # These must be written again so that runLmdSimReco and runLmdReco have access to them
    write_params_to_file(cattrs.unstructure(reco_params), pathname_full, "reco_params.config")
    write_params_to_file(cattrs.unstructure(align_params), pathname_full, "align_params.config")

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
        name="reco_" + reco_params.sim_type_for_resAcc.value,
        logfile_url=pathname_full + "/reco-%a.log",
        array_indices=list(range(low_index_used, low_index_used + num_samples)),
    )

    job.exported_user_variables = {
        "dirname": dirname_full,
        "path_mc_data": path_mc_data,
        "pathname": pathname_full,
        "force_level": str(force_level),
    }

    # TODO: set this in config file!
    if "force_cut_disable" in os.environ:
        if os.environ["force_cut_disable"] == "True":
            job.exported_user_variables["force_cut_disable"] = True

    return (job, pathname_full)
