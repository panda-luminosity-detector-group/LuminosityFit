#!/usr/bin/env python3

import math
import os
import random
import subprocess
from pathlib import Path
from typing import Any, Tuple, Union

import attr
import cattrs
from attr import field
from dotenv import load_dotenv

from .alignment import AlignmentParameters
from .cluster import Job, JobResourceRequest, make_test_job_resource_request
from .general import write_params_to_file
from .reconstruction import ReconstructionParameters, generateRecoDirSuffix
from .simulationTypes import SimulationType

load_dotenv(dotenv_path="../lmdEnvFile.env", verbose=True)


@attr.s
class SimulationParameters:
    def simulation_type_converter(value: Any) -> SimulationType:
        if isinstance(value, SimulationType):
            return value
        elif isinstance(value, str):
            return SimulationType(value)
        raise TypeError("sim_type has to be of type SimulationType or str.")

    sim_type: SimulationType = attr.ib(
        default=SimulationType.PBARP_ELASTIC,
        converter=simulation_type_converter,
    )
    num_events_per_sample: int = attr.ib(default=1000)
    num_samples: int = attr.ib(default=1)
    lab_momentum: float = attr.ib(default=1.5)
    low_index: int = attr.ib(default=1)
    output_dir: Union[Path, None] = field(default=None)
    lmd_geometry_filename: str = attr.ib(default="Luminosity-Detector.root")
    theta_min_in_mrad: float = attr.ib(default=2.7)
    theta_max_in_mrad: float = attr.ib(default=13.0)
    phi_min_in_rad: float = attr.ib(default=0.0)
    phi_max_in_rad: float = attr.ib(default=2 * math.pi)
    neglect_recoil_momentum: bool = attr.ib(default=False)
    random_seed: int = attr.ib(factory=lambda: random.randint(10, 9999))

    ip_offset_x: float = attr.ib(default=0.0)  # in cm
    ip_offset_y: float = attr.ib(default=0.0)  # in cm
    ip_offset_z: float = attr.ib(default=0.0)  # in cm
    ip_spread_x: float = attr.ib(default=0.0)  # in cm
    ip_spread_y: float = attr.ib(default=0.0)  # in cm
    ip_spread_z: float = attr.ib(default=0.0)  # in cm
    beam_tilt_x: float = attr.ib(default=0.0)  # in rad
    beam_tilt_y: float = attr.ib(default=0.0)  # in rad
    beam_divergence_x: float = attr.ib(default=0.0)  # in rad
    beam_divergence_y: float = attr.ib(default=0.0)  # in rad


def _check_ip_params_zero(sim_params: SimulationParameters):
    return any(
        [
            value != 0.0
            for key, value in attr.asdict(sim_params).items()
            if "ip_" in key
        ]
    )


def _check_beam_params_zero(sim_params: SimulationParameters):
    return any(
        [
            value != 0.0
            for key, value in attr.asdict(sim_params).items()
            if "beam_" in key
        ]
    )


def generateDirectory(
    sim_params: SimulationParameters,
    align_params: AlignmentParameters,
    output_dir="",
) -> str:
    if output_dir == "":
        # generate output directory name
        # lets generate a folder structure based on the input
        dirname = f"plab_{sim_params.lab_momentum:.2f}GeV"
        gen_part = (
            f"{sim_params.sim_type.value}_theta_"
            + f"{sim_params.theta_min_in_mrad}-"
            + f"{sim_params.theta_max_in_mrad}mrad"
        )
        if not sim_params.neglect_recoil_momentum:
            gen_part += "_recoil_corrected"

        dirname += "/" + gen_part

        # if not _check_ip_params_zero(sim_params):
        # * always write that
        dirname += (
            "/ip_offset_XYZDXDYDZ_"
            + f"{sim_params.ip_offset_x}_{sim_params.ip_offset_y}_"
            + f"{sim_params.ip_offset_z}_{sim_params.ip_spread_x}_"
            + f"{sim_params.ip_spread_y}_{sim_params.ip_spread_z}"
        )

        # * always write that
        # if not _check_beam_params_zero(sim_params):
        dirname += (
            "/beam_grad_XYDXDY_"
            + f"{sim_params.beam_tilt_x}_{sim_params.beam_tilt_y}_"
            + f"{sim_params.beam_divergence_x}_"
            + f"{sim_params.beam_divergence_y}"
        )

        if (
            align_params.use_point_transform_misalignment
            or align_params.misalignment_matrices_path is None
        ):
            dirname += "/no_geo_misalignment"
        else:
            dirname += "/geo_misalignment" + str(
                os.path.splitext(
                    os.path.basename(align_params.misalignment_matrices_path)
                )[0]
            )

        dirname += "/" + str(sim_params.num_events_per_sample)
    else:
        dirname = output_dir

    return dirname


def create_simulation_and_reconstruction_job(
    sim_params: SimulationParameters,
    align_params: AlignmentParameters,
    reco_params: ReconstructionParameters,
    output_dir="",
    application_command="",
    force_level=0,
    debug=False,
    use_devel_queue=False,
) -> Tuple[Job, str]:
    print(
        "preparing simulations in index range "
        + f"{reco_params.low_index} - "
        + f"{reco_params.low_index + reco_params.num_samples - 1}"
    )

    if application_command == "":
        print(f"ERROR! no application command given!")
        return (None, None)

    dirname = generateDirectory(sim_params, align_params, output_dir)
    dirname_filter_suffix = generateRecoDirSuffix(reco_params, align_params)

    low_index_used = sim_params.low_index
    num_samples = sim_params.num_samples
    if debug and sim_params.num_samples > 1:
        print(
            "Warning: number of samples in debug mode is limited to 1! "
            "Setting to 1!"
        )
        num_samples = 1

    lmdfit_data_dir = os.getenv("LMDFIT_DATA_DIR")
    if lmdfit_data_dir is None:
        raise ValueError("LMDFIT_DATA_DIR environment variable is not set!")

    pathname_base = lmdfit_data_dir + "/" + dirname
    path_mc_data = pathname_base + "/mc_data"
    dirname_full = dirname + "/" + dirname_filter_suffix
    pathname_full = lmdfit_data_dir + "/" + dirname_full
    # this is stored in lmdEnvVar and read with dotenv
    macropath_full = os.environ["LMDFIT_MACROPATH"]

    print("using output folder structure: " + pathname_full)

    try:
        os.makedirs(pathname_full)
        os.makedirs(path_mc_data)
    except OSError as exception:
        print(exception)

    # generate simulation config parameter file
    if sim_params.sim_type == SimulationType.PBARP_ELASTIC:
        lmdfit_build_dir = os.getenv("LMDFIT_BUILD_PATH")
        if lmdfit_build_dir is None:
            raise ValueError(
                "LMDFIT_BUILD_PATH environment variable is not set!"
            )
        # determine the elastic cross section in the theta range
        bashcommand = (
            f"{lmdfit_build_dir}/bin/generatePbarPElasticScattering "
            + f"{sim_params.lab_momentum} 0 "
            + f"-l {sim_params.theta_min_in_mrad}"
            + f" -u {sim_params.theta_max_in_mrad}"
            + f" -n {sim_params.phi_min_in_rad}"
            + f" -g {sim_params.phi_max_in_rad}"
            + f" -o {pathname_base}/elastic_cross_section.txt"
        )
        print(f"\n\nGREPLINE:PBARPGEN:\n{bashcommand}\n")
        subprocess.call(bashcommand.split())

    # These must be written again so that runLmdSimReco and runLmdReco have access to them
    write_params_to_file(
        cattrs.unstructure(sim_params), pathname_base, "sim_params.config"
    )
    write_params_to_file(
        cattrs.unstructure(reco_params), pathname_full, "reco_params.config"
    )
    write_params_to_file(
        cattrs.unstructure(align_params), pathname_full, "align_params.config"
    )

    resource_request = JobResourceRequest(walltime_in_minutes=12 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 3500
    resource_request.node_scratch_filesize_in_mb = 0

    if use_devel_queue:
        resource_request = make_test_job_resource_request()

    job = Job(
        resource_request,
        application_url=application_command,
        name="simreco_" + sim_params.sim_type.value,
        logfile_url=pathname_full + "/simreco-%a.log",
        array_indices=list(
            range(low_index_used, low_index_used + num_samples)
        ),
    )

    job.exported_user_variables.update(
        {
            "force_level": str(force_level),
            "dirname": dirname_full,
            "path_mc_data": path_mc_data,
            "pathname": pathname_full,
            "macropath": macropath_full,
        }
    )

    return (job, pathname_full)
