#!/usr/bin/env python3

"""
This script is a wrapper for createLumiFitData or createKoaFitData and submits it to SLURM. Then why is it so long?


TODO: shorten this down! jesus!
This is soooooo much longer than it needs to be.
- paths are searched via DirectorySearcher (regex comes from args)
- this script is called as script (and not as module)

Sooo:
- make this a module
- remove all search functions
- and instead get the paths from the experiment config
"""

import argparse
import glob
import os
import socket
from pathlib import Path

import lumifit.general as general
from lumifit.cluster import ClusterJobManager, Job, JobResourceRequest
from lumifit.general import getGoodFiles
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler
from lumifit.paths import (
    generateRelativeBinningDir,
    generateRelativeBunchesDir,
)

lmdScriptPath = os.environ["LMDFIT_SCRIPTPATH"]

parser = argparse.ArgumentParser(
    description="This script creates LMD Data objects from filelists in a given directory.",
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
    "pathToRootFiles",
    metavar="pathToRootFiles",
    type=str,
    nargs=1,
    help="The directory where the ROOT files are.",
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
    default="bunches",
    help="",
)
parser.add_argument("--force", action="store_true", help="number of events to use")
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

parser.add_argument(
    "--jobCommand",
    help="either createLumiFitData or createKoaFitData (via singularity job)",
    required=True,
)

args = parser.parse_args()

# okay we need "only" three paths:
# - path to .root files (i.e. data/1-100_uncut/no_alignment/)
# - path to filelists (i.e. data/1-100_uncut/no_alignment/bunches/)
# - path to binning (i.e. data/1-100_uncut/no_alignment/bunches/binning)
#
# there seem to be two data configs, one input and one output?
# the input is actuallt dataconfig_xy!!! and it is in the LuminosityFit directory!
# the output is created in the binning dir by this script
# thats because this script takes one config, changes it somehow (I don't care how)
# and writes it to binning. That latter one is then read by the createLumiFitData binary
# remember, a job array is submitted here, one job per filelist (so 10 probably)
# each job reads the same config, but uses a different filelist (given by the job array indices)

pathToRootFiles = Path(args.pathToRootFiles)
fileListPath = pathToRootFiles / generateRelativeBunchesDir()
binningPath = pathToRootFiles / generateRelativeBunchesDir() / generateRelativeBinningDir()
inputConfigPath = Path(args.config_url[0])

binningPath.mkdir(parents=True, exist_ok=True)

configIO = general.ConfigReaderAndWriter()
config = configIO.loadConfig(inputConfigPath)

jobCommand = args.jobCommand

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

    print(f"saving config to {binningPath}")
    configIO.writeConfigToPath(config, binningPath / "dataconfig.json")

# TODO: read from experiment config! comes later when this is a module
full_hostname = socket.getfqdn()
if "gsi.de" in full_hostname:
    job_handler = create_virgo_job_handler("long")
else:
    job_handler = create_himster_job_handler("himster2_exp")

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = ClusterJobManager(job_handler, 2000, 3600)

fileList, _ = getGoodFiles(fileListPath, "filelist_*.txt")
numFileList = len(fileList)

resource_request = JobResourceRequest(3 * 60)
resource_request.number_of_nodes = 1
resource_request.processors_per_node = 1
resource_request.memory_in_mb = 2500

job = Job(
    resource_request,
    jobCommand,
    "createFitData",
    str(binningPath) + "/createFitData-%a.log",
    array_indices=list(range(1, numFileList + 1)),
)
job.exported_user_variables["numEv"] = args.num_events
job.exported_user_variables["pbeam"] = args.lab_momentum[0]
job.exported_user_variables["input_path"] = pathToRootFiles  # bunches
job.exported_user_variables["filelist_path"] = fileListPath  # bunches
job.exported_user_variables["output_path"] = str(binningPath)  # bunches/binning
job.exported_user_variables["config_path"] = str(binningPath) + "/dataconfig.json"
job.exported_user_variables["type"] = args.type[0]
job.exported_user_variables["elastic_cross_section"] = args.elastic_cross_section

job_manager.append(job)

print(f"All done, waiting for job completion.")
