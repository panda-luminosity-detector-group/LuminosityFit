#!/usr/bin/env python3

import argparse
import socket

from lumifit.alignment import AlignmentParameters
from lumifit.cluster import ClusterJobManager
from lumifit.general import addDebugArgumentsToParser, load_params_from_file
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import (
    SimulationParameters,
    create_simulation_and_reconstruction_job,
)


def run_simulation_and_reconstruction(sim_params, align_params, reco_params):
    job, dir_path = create_simulation_and_reconstruction_job(
        sim_params,
        align_params,
        reco_params,
        force_level=args.force_level,
        debug=args.debug,
        use_devel_queue=args.use_devel_queue,
    )
    full_hostname = socket.getfqdn()
    if args.debug:
        pass
    else:
        if "gsi.de" in full_hostname:
            job_handler = create_virgo_job_handler("long")
        else:
            if args.use_devel_queue:
                job_handler = create_himster_job_handler("devel")
            else:
                job_handler = create_himster_job_handler("himster2_exp")

    # job threshold of this type (too many jobs could generate to much io load
    # as quite a lot of data is read in from the storage...)
    job_manager = ClusterJobManager(job_handler, 2000, 3600)
    job_manager.append(job)
    job_manager.manage_jobs(debug=args.debug)


parser = argparse.ArgumentParser(
    description="Script to run a simulation and reconstruction of the PANDA "
    "Luminosity Detector from parameter config files.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser.add_argument(
    "sim_params",
    metavar="sim_params",
    type=str,
    nargs=1,
    help="Path to simulation properties json file\n",
)
parser.add_argument(
    "reco_params",
    metavar="reco_params",
    type=str,
    nargs=1,
    help="Path to reconstruction properties json file\n",
)
parser.add_argument(
    "--align_params",
    metavar="align_params",
    type=str,
    default="",
    help="Path to alignment properties json file\n",
)

parser = addDebugArgumentsToParser(parser)

args = parser.parse_args()

sim_params = SimulationParameters(**load_params_from_file(args.sim_params[0]))

reco_params = ReconstructionParameters(
    **load_params_from_file(args.reco_params[0])
)

align_params = AlignmentParameters()

if args.align_params:
    align_params = AlignmentParameters(
        **load_params_from_file(args.align_params)
    )


run_simulation_and_reconstruction(sim_params, align_params, reco_params)
