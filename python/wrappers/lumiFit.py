#!/usr/bin/env python3
"""
Module to call the runLmdFit binary from an experiment config.

Can either be run as script with the experiment config as argument, or imported as module.
"""

from lumifit.cluster import Job, JobResourceRequest
from lumifit.paths import generateAbsoluteMergeDataPath
from lumifit.types import ExperimentParameters


def createLumiFitJob(experiment: ExperimentParameters) -> Job:
    # allocate two times this many cores on a compute node, or we'll get a segfault.
    # 16 seems to work when we allocate 32 cores, so just leave it like that. the fit
    # only takes like five minutes anyway
    number_of_threads: int = 16

    LMDscriptpath = experiment.softwarePaths.LmdFitScripts
    LMDbinPath = experiment.softwarePaths.LmdFitBuildDir
    elasticMergeDataPath = generateAbsoluteMergeDataPath(experiment.dataPackage)
    resAccMergeDataPath = generateAbsoluteMergeDataPath(experiment.resAccPackage)
    config_url = experiment.fitConfigPath
    lumiFileName = experiment.lumiFileName
    recoIPfile = experiment.recoIPpath

    resource_request = JobResourceRequest(walltime_in_minutes=12 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = number_of_threads
    resource_request.memory_in_mb = 2000

    job = Job(
        resource_request,
        application_url=f"{LMDscriptpath}/singularityJob.sh '{LMDbinPath}/bin/runLmdFit -c {config_url} -d {elasticMergeDataPath} -a {resAccMergeDataPath} -m {number_of_threads} -o {lumiFileName} -i {recoIPfile}'",
        name="runLmdFit",
        logfile_url=str(elasticMergeDataPath / "runLmdFit.log"),
        array_indices=[1],
    )
    return job


if __name__ == "__main__":
    """
    Script part of this module. Just supply the experiment config file as argument.
    """
    import argparse
    from pathlib import Path

    from lumifit.cluster import ClusterJobManager
    from lumifit.config import load_params_from_file
    from lumifit.gsi_virgo import create_virgo_job_handler
    from lumifit.himster import create_himster_job_handler
    from lumifit.types import ClusterEnvironment, ExperimentParameters

    parser = argparse.ArgumentParser(
        description="Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "-e",
        "--experiment_config",
        dest="ExperimentConfigFile",
        type=Path,
        help="The Experiment.config file that holds all info.",
        required=True,
    )

    args = parser.parse_args()

    # load experiment config
    experiment: ExperimentParameters = load_params_from_file(args.ExperimentConfigFile, ExperimentParameters)

    lumiFitJob = createLumiFitJob(experiment)

    if experiment.cluster == ClusterEnvironment.VIRGO:
        job_handler = create_virgo_job_handler("long")
    elif experiment.cluster == ClusterEnvironment.HIMSTER:
        job_handler = create_himster_job_handler("himster2_exp")
    else:
        raise NotImplementedError(f"Cluster type {experiment.cluster} is not implemented!")

    # job threshold of this type (too many jobs could generate to much io load
    # as quite a lot of data is read in from the storage...)
    job_manager = ClusterJobManager(job_handler, total_job_threshold=2500)

    # for job in joblist:
    job_manager.enqueue(lumiFitJob)

    print("Job submitted!")
