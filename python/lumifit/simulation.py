import subprocess

from lumifit.cluster import (
    Job,
    JobResourceRequest,
    make_test_job_resource_request,
)
from lumifit.types import (
    DataMode,
    ExperimentParameters,
    SimulationGeneratorType,
)


def create_simulation_and_reconstruction_job(
    experiment: ExperimentParameters,
    dataMode: DataMode,
    force_level: int = 0,
    debug: bool = False,
    use_devel_queue: bool = False,
) -> Job:
    """
    parameters:
    experiment: ExperimentParameters
    dataMode: is this real/simulation data (just called DATA) or resAcc data (RESACC)?
    force_level: int
    debug: bool
    use_devel_queue: bool
    """

    # case switch for dataMode
    if dataMode == DataMode.DATA:
        configPackage = experiment.dataPackage
    elif dataMode == DataMode.RESACC:
        configPackage = experiment.resAccPackage
    else:
        raise ValueError("dataMode must be either DATA or RESACC")

    dataDir = configPackage.baseDataDir
    simParams = configPackage.simParams
    recoParams = configPackage.recoParams
    # alignParams = configPackage.alignParams

    assert simParams is not None
    assert configPackage.MCDataDir is not None

    print("preparing simulations in index range " + f"{recoParams.low_index} - " + f"{recoParams.low_index + recoParams.num_samples - 1}")

    low_index_used = simParams.low_index
    num_samples = simParams.num_samples
    if debug and simParams.num_samples > 1:
        print("Warning: number of samples in debug mode is limited to 1! Setting to 1!")
        num_samples = 1

    lmdfit_build_dir = experiment.softwarePaths.LmdFitBinaries

    # generate simulation config parameter file
    if simParams.simGeneratorType == SimulationGeneratorType.PBARP_ELASTIC:
        # determine the elastic cross section in the theta range
        bashcommand = (
            f"{lmdfit_build_dir}/bin/generatePbarPElasticScattering "
            + f"{simParams.lab_momentum} 0"
            + f" -l {simParams.theta_min_in_mrad}"
            + f" -u {simParams.theta_max_in_mrad}"
            + f" -n {simParams.phi_min_in_rad}"
            + f" -g {simParams.phi_max_in_rad}"
            + f" -o {experiment.experimentDir}/elastic_cross_section.txt"
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
        # simulation command is one of {runLmdSimReco, runLmdReco, createLumiFitData}
        # or the KOALA equivalent. set during config generation, i.e. in
        # create_sim_reco_pars.py (or by hand)
        application_url=simParams.simulationCommand,
        # name is only for the batch system, so that we can see what jobs are running
        name="simreco_" + simParams.simGeneratorType.value,
        # log file is very important, but it doesn't have to be in the data dir
        logfile_url=str(dataDir / "simlogs/simreco-%a.log"),
        array_indices=list(range(low_index_used, low_index_used + num_samples)),
    )
    job.exported_user_variables.update(
        {
            "ExperimentDir": experiment.experimentDir,
            "DataMode": dataMode.value,
            "force_level": str(force_level),
        }
    )

    return job
