#!/usr/bin/env python3

"""
This script is only run by a user, not any other script!
"""

import argparse
from pathlib import Path

from lumifit.cluster import ClusterJobManager, DebugJobHandler, JobHandler
from lumifit.config import load_params_from_file
from lumifit.general import addDebugArgumentsToParser, envPath
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler
from lumifit.scenario import Scenario
from lumifit.simulation import create_simulation_and_reconstruction_job
from lumifit.types import ClusterEnvironment, ExperimentParameters


def run_simulation_and_reconstruction(thisExperiment: ExperimentParameters) -> None:
    if thisExperiment.recoParams.use_xy_cut or thisExperiment.recoParams.use_m_cut:
        print("Attention! This experiment configs specifies to use XY and m cuts during reconstruction.")
        print("That's reasonable for the luminosity determination, but the initial data sample must")
        print("still be generated without cuts first.")
        print("Disabling all cuts for this run!")
        thisExperiment.recoParams.use_xy_cut = False
        thisExperiment.recoParams.use_m_cut = False

    # temporary to get sim command
    lmdScriptPath = envPath("LMDFIT_SCRIPTPATH")
    scen = Scenario(trackDirectory_=Path(), experiment_type=thisExperiment.experimentType, lmdScriptPath=lmdScriptPath)

    job, _ = create_simulation_and_reconstruction_job(
        thisExperiment.simParams,
        thisExperiment.alignParams,
        thisExperiment.recoParams,
        force_level=args.force_level,
        debug=args.debug,
        use_devel_queue=args.use_devel_queue,
        application_command=scen.Sim,
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
    job_manager.append(job)


parser = argparse.ArgumentParser(
    description="Script to run a simulation and reconstruction of the PANDA " "Luminosity Detector from parameter config files.",
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

thisExperiment: ExperimentParameters = load_params_from_file(args.experimentConfig, asType=ExperimentParameters)

run_simulation_and_reconstruction(thisExperiment)
