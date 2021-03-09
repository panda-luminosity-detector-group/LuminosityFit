#!/usr/bin/env python3

import argparse
import json

from .alignment import AlignmentParameters
from .cluster import ClusterJobManager
from .general import addDebugArgumentsToParser
from .simulation import create_simulation_and_reconstruction_job


def run_simulation_and_reconstruction(sim_params, align_params, reco_params):
    job = create_simulation_and_reconstruction_job(
        sim_params,
        align_params,
        reco_params,
        force_level=args.force_level,
        debug=args.debug,
        use_devel_queue=args.use_devel_queue,
    )

    # job threshold of this type (too many jobs could generate to much io load
    # as quite a lot of data is read in from the storage...)
    job_manager = ClusterJobManager(2000, 3600)
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

with open(args.sim_params[0], "r") as json_file:
    sim_params = json.load(json_file)

with open(args.reco_params[0], "r") as json_file:
    reco_params = json.load(json_file)

align_params = AlignmentParameters()

if args.align_params:
    with open(args.align_params, "r") as json_file:
        align_params = json.load(json_file)

run_simulation_and_reconstruction(sim_params, align_params, reco_params)
