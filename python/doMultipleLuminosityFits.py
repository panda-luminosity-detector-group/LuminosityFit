#!/usr/bin/env python3

"""
This script is a wrapper for runLmdFit.sh and submits it to SLURM. Then why is it so long?

Example call might look like this:

python doMultipleLuminosityFits.py \
--forced_box_gen_data \
/lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_1.5GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction \
/lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_1.5GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
xy_m_cut_real \
/home/roklasen/LuminosityFit/fitconfig-fast.json

TODO: shorten this down! jesus!
This is soooooo much longer than it needs to be. Also, because the wrapped binary can already be called from the command line with a million arguments, this script doen't need to be callabe from the command line. It just needs to be usable as module. 
- 100 sloc are only path search functions
- paths are searched via regex
- the regex is made up on the fly from some parameters
- this script is calle as script (and not as module)

Sooo:
- make this a module
- remove all search functions
- and instead get the paths from the experiment config
- this thing can submit multiple fit jobs at once, but we don't want that anymore. A fit job must always come from an experiment config, so we can just submit one job at a time.
- also remove the bash script and just call the binary directly!
"""


import argparse

# import glob
# import os
# import re
from pathlib import Path

# import lumifit.general as general
from lumifit.cluster import ClusterJobManager, Job, JobResourceRequest
from lumifit.config import load_params_from_file

# from lumifit.general import envPath
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler
from lumifit.paths import (
    generateAbsoluteMergeDataPath,
    generateAbsoluteROOTDataPath,
)
from lumifit.types import ClusterEnvironment, ExperimentParameters

# from typing import List, Optional


# dirs: List[Path] = []
# resAccDirs: List[Path] = []

# box_res_glob_pattern = ["lmd_res_data_", "of", ".root"]
# box_acc_glob_pattern = ["lmd_acc_data_", "of", ".root"]

# top_level_resAcc_directory = Path()

# LMDscriptpath = envPath("LMDFIT_SCRIPTPATH")


# def getListOfResAccDirectories(path: str) -> None:
#     if os.path.isdir(path):
#         print("currently looking at directory " + path)

#         if os.path.split(path)[1] == "mc_data":
#             return

#         for dir in os.listdir(path):
#             bunch_dirs = glob.glob(path + "/bunches_*/" + tail_dir_pattern)
#             if bunch_dirs:
#                 for bunch_dir in bunch_dirs:
#                     filelists = glob.glob(bunch_dir + "/" + box_acc_glob_pattern)
#                     if filelists:
#                         filelists = glob.glob(bunch_dir + "/" + box_res_glob_pattern)
#                         if filelists:
#                             resAccDirs.append(bunch_dir)
#                 return
#             else:
#                 if glob.glob(path + "Track" + "*.root"):
#                     return
#             dirpath = path + "/" + dir
#             if os.path.isdir(dirpath):
#                 getListOfResAccDirectories(dirpath)


# def getTopResAccDirectory(path: Path) -> None:
#     if os.path.isdir(path):
#         found = False
#         for directory in next(os.walk(path))[1]:
#             if re.search(f"{experiment.recoParams.simGenTypeForResAcc.value}", directory):  # read BOX/DPM string from config
#                 global top_level_resAcc_directory
#                 top_level_resAcc_directory = path / directory
#                 found = True
#                 break
#         if not found:
#             getTopResAccDirectory(path.absolute())


# # TODO: holy shit, DONT MAKE UP REGEX PATTERN ON THE FLY
# # also, this function will only find the test data if it is called "dpm" something
# #! TODO: in case data is from box gen (or FTF) this DOESN'T WORK!
# def findMatchingDirs(resAcc_data_path: Optional[Path]) -> List[List[Path]]:
#     matching_dir_pairs: List[List[Path]] = []
#     if (resAcc_data_path == Path()) or (resAcc_data_path is None):
#         for dpm_dir in dirs:
#             print(dpm_dir)
#             match = re.search(
#                 r"^(.*/)dpm_.*?/(ip_offset_XYZDXDYDZ_.*)/.*/\d*/\d*-\d*_(.*cut)/.*/(binning_\d*)/merge_data$",
#                 str(dpm_dir),
#             )
#             pattern = (
#                 "^"
#                 + match.group(1)  # type: ignore
#                 + f"{experiment.recoParams.simGenTypeForResAcc.value}_.*?"  # read BOX/DPM string from config
#                 + match.group(2)  # type: ignore
#                 + ".*"
#                 + match.group(3)  # type: ignore
#                 + "/.*"
#                 + match.group(4)  # type: ignore
#                 + "/merge_data$"
#             )
#             # print pattern
#             for resAcc_dir in resAccDirs:
#                 # print box_dir
#                 box_match = re.search(pattern, str(resAcc_dir))
#                 if box_match:
#                     matching_dir_pairs.append([dpm_dir, resAcc_dir])
#                     break
#     else:
#         for dpm_dir in dirs:
#             # attempt to find directory with same binning
#             print(f"checking for matching directory for {dpm_dir}")
#             match = re.search(r"^.*(binning_\d*)/.*$", str(dpm_dir))
#             if match:
#                 dir_searcher = general.DirectorySearcher([match.group(1)])
#                 dir_searcher.searchListOfDirectories(resAcc_data_path, box_acc_glob_pattern)
#                 correct_dirs = dir_searcher.getListOfDirectories()

#                 if correct_dirs:
#                     matching_dir_pairs.append([dpm_dir, correct_dirs[0]])
#     return matching_dir_pairs


parser = argparse.ArgumentParser(
    description="Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.",
    formatter_class=argparse.RawTextHelpFormatter,
)

# parser.add_argument(
#     "dirname",
#     metavar="dirname_to_scan",
#     type=Path,
#     nargs=1,
#     help="Name of directory to scan recursively for lmd data files and call merge!",
# )

# parser.add_argument(
#     "dirname_pattern",
#     metavar="directory pattern",
#     type=str,
#     nargs=1,
#     help="Only found directories with this pattern are used!",
# )

# parser.add_argument(
#     "config_url",
#     metavar="Path to fit config file (json)",
#     type=str,
#     nargs=1,
#     help="Specify the full path of the config file that will be used for the fit.",
# )

# parser.add_argument(
#     "--tail_dir_pattern",
#     metavar="tail directory pattern",
#     type=str,
#     default="merge_data",
#     help="Only found directories with this pattern are used!",
# )

# parser.add_argument(
#     "--ref_resacc_gen_data",
#     metavar="ref res/acc data",
#     type=Path,
#     default="",
#     help="If specified then this path will be used for all fits as the reference res/acc data. WARNING: DOES NOT WORK CURRENTLY.",
# )

# parser.add_argument(
#     "--forced_resAcc_gen_data",
#     metavar="forced res/acc data",
#     type=Path,
#     default="",
#     help="If specified then this path will be used for all fits as the res/acc data, ignoring other res/acc data directories.",
# )

# parser.add_argument(
#     "--number_of_threads",
#     metavar="number of concurrent threads",
#     type=int,
#     default="16",
#     help="Number of concurrent threads within the estimator that is used for fitting. Default: 16 threads.",
# )

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


# allocate two times this many cores on a compute node, or we'll get a segault.
# 16 seems to work when we allocate 32 cores, so just leave it like that. the fit
# only takes like five minutes anyway
number_of_threads: int = 16

# tail_dir_pattern = args.tail_dir_pattern

# dpm_glob_pattern = ["lmd_data_", "of", ".root"]

# patterns = [args.dirname_pattern[0], args.tail_dir_pattern]
# dir_searcher = general.DirectorySearcher(patterns)

# dir_searcher.searchListOfDirectories(args.dirname[0], dpm_glob_pattern)
# dirs = dir_searcher.getListOfDirectories()

# if args.forced_resAcc_gen_data == "":
#     getTopResAccDirectory(args.dirname[0])
#     print(f"box top dir: {top_level_resAcc_directory}")
#     # getListOfBoxDirectories(top_level_box_directory)
#     box_dir_searcher = general.DirectorySearcher(patterns)
#     box_dir_searcher.searchListOfDirectories(top_level_resAcc_directory, box_acc_glob_pattern)
#     resAccDirs = box_dir_searcher.getListOfDirectories()


# matches = findMatchingDirs(args.forced_resAcc_gen_data)

# print(matches)
# print(len(matches))

# joblist = []

# for match in matches:
# elastic_data_path = match[0]
# acc_res_data_path = match[1]

#! setup paths for lumifit
LMDscriptpath = experiment.softwarePaths.LmdFitScripts
LMDbinPath = experiment.softwarePaths.LmdFitBinaries
elastic_data_path = generateAbsoluteMergeDataPath(experiment.dataPackage)
acc_res_data_path = generateAbsoluteMergeDataPath(experiment.resAccPackage)
config_url = experiment.fitConfigPath

resource_request = JobResourceRequest(walltime_in_minutes=12 * 60)
resource_request.number_of_nodes = 1
resource_request.processors_per_node = number_of_threads
resource_request.memory_in_mb = 2000

# TODO: don't call the runLmdFit.sh script, but instead the runLmdFit binary directly. all the neccessary varibles are already here
# job = Job(
#     resource_request,
#     application_url=f"{LMDscriptpath}/singularityJob.sh {LMDscriptpath}/runLmdFit.sh",
#     name="runLmdFit",
#     logfile_url=str(elastic_data_path / "runLmdFit.log"),
#     array_indices=[1],
# )
job = Job(
    resource_request,
    application_url=f"{LMDscriptpath}/singularityJob.sh {LMDbinPath}/bin/runLmdFit -c {config_url} -d {elastic_data_path} -a {acc_res_data_path} -m {number_of_threads}",
    name="runLmdFit",
    logfile_url=str(elastic_data_path / "runLmdFit.log"),
    array_indices=[1],
)

# job.exported_user_variables = {
#     "config_url": str(config_url),
#     "data_path": str(elastic_data_path),
#     "acceptance_resolution_path": str(acc_res_data_path),
#     "number_of_threads": str(number_of_threads),
# }

# # TODO: handle this case
# if args.ref_resacc_gen_data != "":
#     job.exported_user_variables["reference_acceptance_path"] = args.ref_resacc_gen_data

# joblist.append(job)

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
job_manager.append(job)
