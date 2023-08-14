from pathlib import Path

from attrs import evolve
from lumifit.cluster import (
    Job,
    JobResourceRequest,
    make_test_job_resource_request,
)
from lumifit.paths import generateAbsoluteROOTDataPath
from lumifit.types import DataMode, ExperimentParameters


def create_reconstruction_job(
    experiment: ExperimentParameters,
    thisMode: DataMode,
    force_level: int = 0,
    debug: bool = False,
    use_devel_queue: bool = False,
) -> Job:
    # case switch for dataMode
    if thisMode == DataMode.DATA:
        configPackage = experiment.dataPackage
    elif thisMode == DataMode.VERTEXDATA:
        configPackage = experiment.dataPackage
        configPackage.recoParams.disableCuts()

    elif thisMode == DataMode.RESACC:
        configPackage = experiment.resAccPackage
    else:
        raise NotImplementedError(f"DataMode {thisMode} not implemented")

    recoParams = configPackage.recoParams

    print("preparing reconstruction in index range " + f"{recoParams.low_index} - " + f"{recoParams.low_index + recoParams.num_samples - 1}")

    low_index_used = recoParams.low_index
    num_samples = recoParams.num_samples
    if debug and recoParams.num_samples > 1:
        print("Warning: number of samples in debug mode is limited to 1!" " Setting to 1!")
        num_samples = 1

    ROOTDataDir = generateAbsoluteROOTDataPath(configPackage)
    print(f"dir name for this create_reconstruction_job is {ROOTDataDir}")

    resource_request = JobResourceRequest(2 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 3500
    resource_request.node_scratch_filesize_in_mb = 0

    if use_devel_queue:
        resource_request = make_test_job_resource_request()

    job = Job(
        resource_request,
        application_url=recoParams.reconstructionCommand,
        name="reco",
        logfile_url=str(ROOTDataDir / "reco-%a.log"),
        array_indices=list(range(low_index_used, low_index_used + num_samples)),
    )

    # TODO: these won't be needed anymore the're in the config file
    # TODO: get from the paths module
    job.exported_user_variables = {
        "ExperimentDir": experiment.experimentDir,
        "DataMode": thisMode.value,
        "force_level": str(force_level),
    }

    if recoParams.force_cut_disable:
        job.exported_user_variables["force_cut_disable"] = True

    return job
