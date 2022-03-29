#!/usr/bin/env python3

import argparse
import errno
import glob
import socket
import os

import lumifit.general as general
from lumifit.cluster import ClusterJobManager, Job, JobResourceRequest
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler

parser = argparse.ArgumentParser(
    description="Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.",
    formatter_class=argparse.RawTextHelpFormatter,
)


parser.add_argument(
    "lab_momentum",
    metavar="lab_momentum",
    type=float,
    nargs=1,
    help="lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)",
)
parser.add_argument(
    "type",
    metavar="type",
    type=str,
    nargs=1,
    help="type of data to create (a = angular, e = efficiency, r = resolution, v = vertex/ip",
)
parser.add_argument(
    "dirname",
    metavar="dirname_to_scan",
    type=str,
    nargs=1,
    help="Name of directory to scan recursively for qa files and create bunches",
)
parser.add_argument(
    "config_url",
    metavar="config_url",
    type=str,
    nargs=1,
    help="Path to data config file in json format.",
)

parser.add_argument(
    "--dir_pattern",
    metavar="path name pattern",
    type=str,
    default="bunches*",
    help="",
)
parser.add_argument(
    "--force", action="store_true", help="number of events to use"
)
parser.add_argument(
    "--num_events",
    metavar="num_events",
    type=int,
    default=0,
    help="number of events to use",
)
parser.add_argument(
    "--elastic_cross_section",
    metavar="elastic_cross_section",
    type=float,
    default=1.0,
    help="Total elastic cross section. Relevant for luminosity extraction performance tests!",
)
parser.add_argument(
    "--general_dimension_bins_low",
    metavar="general_dimension_bins_low",
    type=int,
    default=300,
    help="binning of data min",
)
parser.add_argument(
    "--general_dimension_bins_high",
    metavar="general_dimension_bins_high",
    type=int,
    default=300,
    help="binning of data max",
)
parser.add_argument(
    "--general_dimension_bins_step",
    metavar="general_dimension_bins_step",
    type=int,
    default=100,
    help="binning of data stepsize",
)


args = parser.parse_args()

failed_submit_commands = []

dir_searcher = general.DirectorySearcher([args.dir_pattern])

dir_searcher.searchListOfDirectories(args.dirname[0], ["filelist_", ".txt"])
dirs = dir_searcher.getListOfDirectories()

config_modifier = general.ConfigModifier()
config = config_modifier.loadConfig(args.config_url[0])

config_paths = []

for bins in range(
    args.general_dimension_bins_low,
    args.general_dimension_bins_high + 1,
    args.general_dimension_bins_step,
):
    if "general_data" in config:
        subconfig = config["general_data"]
        if "primary_dimension" in subconfig:
            subsubconfig = subconfig["primary_dimension"]
            if "bins" in subsubconfig:
                subsubconfig["bins"] = bins
        if "secondary_dimension" in subconfig:
            subsubconfig = subconfig["secondary_dimension"]
            if "bins" in subsubconfig:
                subsubconfig["bins"] = bins

    if "efficiency" in config:
        subconfig = config["efficiency"]
        if "primary_dimension" in subconfig:
            subsubconfig = subconfig["primary_dimension"]
            if "bins" in subsubconfig:
                subsubconfig["bins"] = bins
        if "secondary_dimension" in subconfig:
            subsubconfig = subconfig["secondary_dimension"]
            if "bins" in subsubconfig:
                subsubconfig["bins"] = bins

    for dir in dirs:
        path = dir + "/binning_" + str(bins)
        print("saving config to", path)
        try:
            os.makedirs(path)
        except OSError as exception:
            if exception.errno != errno.EEXIST:
                print("error: thought dir does not exists but it does...")
        if os.path.isfile(path + "/dataconfig.json"):
            os.remove(path + "/dataconfig.json")
        config_modifier.writeConfigToPath(config, path + "/dataconfig.json")
        config_paths.append(path)


full_hostname = socket.getfqdn()
if "gsi.de" in full_hostname:
    job_handler = create_virgo_job_handler("long")
else:
    job_handler = create_himster_job_handler("himster2_exp")

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = ClusterJobManager(job_handler, 2000, 3600)

for config_path in config_paths:
    filelist_path = os.path.split(config_path)[0]
    input_path = os.path.split(filelist_path)[0]

    num_filelists = len(glob.glob(filelist_path + "/filelist_*.txt"))

    resource_request = JobResourceRequest(3 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 2500
    # TODO: choose correct create data application
    job = Job(
        resource_request,
        "./createLumiFitData.sh",
        "createLumiFitData",
        config_path + "/createLumiFitData-%a.log",
        # TODO: what is this? Job doesn't have array_indices
        array_indices=list(range(1, num_filelists + 1)),
    )

    job.exported_user_variables["numEv"] = args.num_events
    job.exported_user_variables["pbeam"] = args.lab_momentum[0]
    job.exported_user_variables["input_path"] = input_path
    job.exported_user_variables["filelist_path"] = filelist_path
    job.exported_user_variables["output_path"] = config_path
    job.exported_user_variables["config_path"] = (
        config_path + "/dataconfig.json"
    )
    job.exported_user_variables["type"] = args.type[0]
    job.exported_user_variables[
        "elastic_cross_section"
    ] = args.elastic_cross_section

    job_manager.append(job)
