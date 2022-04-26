#!/usr/bin/env python3
import os
import re
import glob
import lumifit.general as general
import lumifit.himster as himster
import argparse

dirs: list = []
box_dirs: list = []

box_res_glob_pattern = ["lmd_res_data_", "of", ".root"]
box_acc_glob_pattern = ["lmd_acc_data_", "of", ".root"]

top_level_box_directory = ""

LMDscriptpath = os.environ["LMDFIT_SCRIPTPATH"]


def getListOfBoxDirectories(path):
    if os.path.isdir(path):
        print("currently looking at directory " + path)

        if os.path.split(path)[1] == "mc_data":
            return

        for dir in os.listdir(path):
            bunch_dirs = glob.glob(path + "/bunches_*/" + tail_dir_pattern)
            if bunch_dirs:
                for bunch_dir in bunch_dirs:
                    filelists = glob.glob(
                        bunch_dir + "/" + box_acc_glob_pattern
                    )
                    if filelists:
                        filelists = glob.glob(
                            bunch_dir + "/" + box_res_glob_pattern
                        )
                        if filelists:
                            box_dirs.append(bunch_dir)
                return
            else:
                if glob.glob(path + "Track" + "*.root"):
                    return
            dirpath = path + "/" + dir
            if os.path.isdir(dirpath):
                getListOfBoxDirectories(dirpath)


def getTopBoxDirectory(path):
    if os.path.isdir(path):
        found = False
        for dir in next(os.walk(path))[1]:
            if re.search("box", dir):
                global top_level_box_directory
                top_level_box_directory = path + "/" + dir
                found = True
                break
        if not found:
            getTopBoxDirectory(os.path.dirname(path))


def findMatchingDirs(box_data_path):
    matching_dir_pairs = []
    if box_data_path == "":
        for dpm_dir in dirs:
            print(dpm_dir)
            match = re.search(
                r"^(.*/)dpm_.*?/(ip_offset_XYZDXDYDZ_.*)/.*/\d*/\d*-\d*_(.*cut)/.*/(binning_\d*)/merge_data$",
                dpm_dir,
            )
            pattern = (
                "^"
                + match.group(1)
                + "box_.*?"
                + match.group(2)
                + ".*"
                + match.group(3)
                + "/.*"
                + match.group(4)
                + "/merge_data$"
            )
            # print pattern
            for box_dir in box_dirs:
                # print box_dir
                box_match = re.search(pattern, box_dir)
                if box_match:
                    matching_dir_pairs.append([dpm_dir, box_dir])
                    break
    else:
        for dpm_dir in dirs:
            # attempt to find directory with same binning
            print("checking for matching directory for " + dpm_dir)
            match = re.search(r"^.*(binning_\d*)/.*$", dpm_dir)
            if match:
                dir_searcher = general.DirectorySearcher([match.group(1)])
                dir_searcher.searchListOfDirectories(
                    box_data_path, box_acc_glob_pattern
                )
                correct_dirs = dir_searcher.getListOfDirectories()

                if correct_dirs:
                    matching_dir_pairs.append([dpm_dir, correct_dirs[0]])
    return matching_dir_pairs


parser = argparse.ArgumentParser(
    description="Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser.add_argument(
    "dirname",
    metavar="dirname_to_scan",
    type=str,
    nargs=1,
    help="Name of directory to scan recursively for lmd data files and call merge!",
)

parser.add_argument(
    "dirname_pattern",
    metavar="directory pattern",
    type=str,
    nargs=1,
    help="Only found directories with this pattern are used!",
)

parser.add_argument(
    "config_url",
    metavar="Path to fit config file (json)",
    type=str,
    nargs=1,
    help="Specify the full path of the config file that will be used for the fit.",
)

parser.add_argument(
    "--tail_dir_pattern",
    metavar="tail directory pattern",
    type=str,
    default="merge_data",
    help="Only found directories with this pattern are used!",
)

parser.add_argument(
    "--ref_box_gen_data",
    metavar="ref box gen data",
    type=str,
    default="",
    help="If specified then this path will be used for all fits as the reference box gen data.",
)

parser.add_argument(
    "--forced_box_gen_data",
    metavar="forced box gen data",
    type=str,
    default="",
    help="If specified then this path will be used for all fits as the box gen data, ignoring other box gen data directories.",
)

parser.add_argument(
    "--number_of_threads",
    metavar="number of concurrent threads",
    type=int,
    default="16",
    help="Number of concurrent threads within the estimator that is used for fitting. Default: 16 threads.",
)

args = parser.parse_args()

number_of_threads = args.number_of_threads
if args.number_of_threads > 32:
    number_of_threads = 32

tail_dir_pattern = args.tail_dir_pattern

dpm_glob_pattern = ["lmd_data_", "of", ".root"]

patterns = [args.dirname_pattern[0], args.tail_dir_pattern]
dir_searcher = general.DirectorySearcher(patterns)

dir_searcher(args.dirname[0], dpm_glob_pattern)
dirs = dir_searcher.getListOfDirectories()

if args.forced_box_gen_data == "":
    getTopBoxDirectory(args.dirname[0])
    print("box top dir: " + top_level_box_directory)
    # getListOfBoxDirectories(top_level_box_directory)
    box_dir_searcher = general.DirectorySearcher(patterns)
    box_dir_searcher.searchListOfDirectories(
        top_level_box_directory, box_acc_glob_pattern
    )
    box_dirs = box_dir_searcher.getListOfDirectories()


matches = findMatchingDirs(args.forced_box_gen_data)

print(matches)
print(len(matches))

joblist = []

for match in matches:
    elastic_data_path = match[0]
    acc_res_data_path = match[1]

    resource_request = himster.JobResourceRequest(12 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = number_of_threads
    resource_request.memory_in_mb = 2000
    job = himster.Job(
        resource_request,
        f"{LMDscriptpath}/singularityJob.sh {LMDscriptpath}/runLmdFit.sh",
        "runLmdFit",
        elastic_data_path + "/runLmdFit.log",
    )
    job.set_job_array_indices([1])

    job.add_exported_user_variable("config_url", args.config_url[0])
    job.add_exported_user_variable("data_path", elastic_data_path)
    job.add_exported_user_variable(
        "acceptance_resolution_path", acc_res_data_path
    )
    job.add_exported_user_variable("number_of_threads", number_of_threads)
    if args.ref_box_gen_data != "":
        job.add_exported_user_variable(
            "reference_acceptance_path", args.ref_box_gen_data
        )

    joblist.append(job)

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = himster.HimsterJobManager(2500)

job_manager.submit_jobs_to_himster(joblist)
job_manager.manage_jobs()
