import subprocess
from pathlib import Path
from typing import Tuple

from lumifit.cluster import (
    Job,
    JobResourceRequest,
    make_test_job_resource_request,
)
from lumifit.reconstruction import generateRecoDirSuffix
from lumifit.types import ExperimentParameters, SimulationGeneratorType


def create_simulation_and_reconstruction_job(
    experiment: ExperimentParameters,
    force_level: int = 0,
    debug: bool = False,
    use_devel_queue: bool = False,
) -> Tuple[Job, Path]:
    print(
        "preparing simulations in index range "
        + f"{experiment.recoParams.low_index} - "
        + f"{experiment.recoParams.low_index + experiment.recoParams.num_samples - 1}"
    )

    dirname = experiment.softwarePaths.simData
    recoCutAndAlignSuffix = generateRecoDirSuffix(experiment.recoParams, experiment.alignParams)

    low_index_used = experiment.simParams.low_index
    num_samples = experiment.simParams.num_samples
    if debug and experiment.simParams.num_samples > 1:
        print("Warning: number of samples in debug mode is limited to 1! Setting to 1!")
        num_samples = 1

    pathname_base = experiment.softwarePaths.baseDataDir
    dirname_full = dirname / recoCutAndAlignSuffix
    pathname_full = lmdfit_data_dir / dirname_full

    # pathname_full.mkdir(exist_ok=True, parents=True)

    # generate simulation config parameter file
    if experiment.simParams.simGeneratorType == SimulationGeneratorType.PBARP_ELASTIC:
        lmdfit_build_dir = experiment.softwarePaths.LmdFitBinaries

        # determine the elastic cross section in the theta range
        bashcommand = (
            f"{lmdfit_build_dir}/bin/generatePbarPElasticScattering "
            + f"{experiment.simParams.lab_momentum} 0 "
            + f"-l {experiment.simParams.theta_min_in_mrad}"
            + f" -u {experiment.simParams.theta_max_in_mrad}"
            + f" -n {experiment.simParams.phi_min_in_rad}"
            + f" -g {experiment.simParams.phi_max_in_rad}"
            + f" -o {pathname_base}/elastic_cross_section.txt"
        )
        subprocess.call(bashcommand.split())

    resource_request = JobResourceRequest(walltime_in_minutes=12 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 3500
    resource_request.node_scratch_filesize_in_mb = 0

    if use_devel_queue:
        resource_request = make_test_job_resource_request()

    job = Job(
        resource_request,
        application_url=experiment.simParams.simulationCommand,
        name="simreco_" + experiment.simParams.simGeneratorType.value,
        logfile_url=str(pathname_full / "simreco-%a.log"),
        array_indices=list(range(low_index_used, low_index_used + num_samples)),
    )
    # TODO: these won't be needed anymore the're in the config file
    job.exported_user_variables.update(
        {
            "BaseDir": experiment.softwarePaths.baseDataDir,
            # "dirname": dirname_full,
            # "path_mc_data": path_mc_data,
            # "pathname": pathname_full,
            # "macropath": macropath_full,
            "force_level": str(force_level),
        }
    )

    return (job, pathname_full)
