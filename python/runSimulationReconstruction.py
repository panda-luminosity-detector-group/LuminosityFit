#!/usr/bin/env python3

"""
This script is only run by a user, not any other script!
"""

import argparse
from pathlib import Path

from lumifit.cluster import ClusterJobManager, DebugJobHandler, JobHandler
from lumifit.config import load_params_from_file, write_params_to_file
from lumifit.general import addDebugArgumentsToParser
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler
from lumifit.types import ClusterEnvironment, DataMode, ExperimentParameters
from wrappers.createSimRecoJob import create_simulation_and_reconstruction_job


def run_simulation_and_reconstruction(thisExperiment: ExperimentParameters) -> None:
    assert thisExperiment.dataPackage.simParams is not None
    assert thisExperiment.dataPackage.MCDataDir is not None

    thisExperiment.dataPackage.MCDataDir.mkdir(parents=True, exist_ok=True)
    thisExperiment.dataPackage.baseDataDir.mkdir(parents=True, exist_ok=True)

    job = create_simulation_and_reconstruction_job(
        thisExperiment,
        thisMode=DataMode.VERTEXDATA,
        force_level=args.force_level,
        debug=args.debug,
        use_devel_queue=args.use_devel_queue,
    )

    job_handler: JobHandler
    if args.debug:
        job_handler = DebugJobHandler()

    else:
        if thisExperiment.cluster == ClusterEnvironment.VIRGO:
            job_handler = create_virgo_job_handler("long")
        elif thisExperiment.cluster == ClusterEnvironment.HIMSTER:
            if args.use_devel_queue:
                job_handler = create_himster_job_handler("devel")
            else:
                job_handler = create_himster_job_handler("himster2_exp")
        else:
            raise NotImplementedError("Cluster not implemented")

    # job threshold of this type (too many jobs could generate to much io load
    # as quite a lot of data is read in from the storage...)
    job_manager = ClusterJobManager(job_handler, 2000, 3600)
    job_manager.enqueue(job)

    # copy this experiment's config file to the target path


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Script to run a simulation and reconstruction of the PANDA Luminosity Detector from parameter config files.",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "-e",
        metavar="experiment-config",
        dest="experimentConfig",
        type=Path,
        required=True,
        help="Path to experiment.config file\n",
    )

    parser = addDebugArgumentsToParser(parser)
    args = parser.parse_args()

    thisExperiment: ExperimentParameters = load_params_from_file(file_path=args.experimentConfig, asType=ExperimentParameters)

    # make a copy before any changes
    write_params_to_file(thisExperiment, thisExperiment.experimentDir, "experiment.config")

    run_simulation_and_reconstruction(thisExperiment)
