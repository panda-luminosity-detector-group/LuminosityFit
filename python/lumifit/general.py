import glob
import json
import os
import re
from argparse import ArgumentDefaultsHelpFormatter, RawTextHelpFormatter
from enum import Enum


def getGoodFiles(
    directory, glob_pattern, min_filesize_in_byte=2000, is_bunches=False
):
    found_files = glob.glob(directory + "/" + glob_pattern)
    good_files = []
    bad_files = []
    for file in found_files:
        if os.stat(file).st_size > min_filesize_in_byte:
            good_files.append(file)
        else:
            bad_files.append(file)

    if is_bunches:
        m = re.search("\/bunches_(\d+)", directory)
        num_sim_files = int(m.group(1))
    else:
        m = re.search("\/(\d+)-(\d+)_.+?cut", directory)
        num_sim_files = int(m.group(2)) - int(m.group(1)) + 1

    files_percentage = len(good_files) / num_sim_files

    return [good_files, files_percentage]


class SmartFormatter(ArgumentDefaultsHelpFormatter):
    def _split_lines(self, text, width):
        return RawTextHelpFormatter._split_lines(self, text, width)


def addDebugArgumentsToParser(parser):
    parser.add_argument(
        "--force_level",
        metavar="force_level",
        type=int,
        default=0,
        help="force level 0: if directories exist with data "
        "files no new simulation is started\n"
        "force level 1: will do full reconstruction even if "
        "this data already exists, but not geant simulation\n"
        "force level 2: resimulation of everything!",
    )

    parser.add_argument(
        "--use_devel_queue",
        action="store_true",
        help="If flag is set, the devel queue is used",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="If flag is set, the simulation runs locally for "
        "debug purposes",
    )

    return parser


class _EnumEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, Enum):
            return obj.value
        return json.JSONEncoder.default(self, obj)


def write_params_to_file(params: dict, pathname: str, filename: str):
    file_path = pathname + "/" + filename
    if not os.path.exists(file_path):
        print("creating config file: " + file_path)
        with open(file_path, "w") as json_file:
            json.dump(
                params, json_file, sort_keys=True, indent=4, cls=_EnumEncoder
            )
    else:
        print(f"Config file {filename} already exists!")


def load_params_from_file(file_path: str) -> dict:
    if os.path.exists(file_path):
        with open(file_path, "r") as json_file:
            return json.load(json_file)

    print(f"file {file_path} does not exist!")
    return {}


class DirectorySearcher:
    def __init__(self, patterns_, not_contain_pattern_=""):
        self.patterns = patterns_
        self.not_contain_pattern = not_contain_pattern_
        self.dirs = []

    def getListOfDirectories(self):
        return self.dirs

    def searchListOfDirectories(self, path, glob_patterns):
        # print("looking for files with pattern: ", glob_patterns)
        # print("dirpath forbidden patterns:", self.not_contain_pattern)
        # print("dirpath patterns:", self.patterns)
        file_patterns = [glob_patterns]
        if isinstance(glob_patterns, list):
            file_patterns = glob_patterns

        for dirpath, dirs, files in os.walk(path):
            # print('currently looking at directory', dirpath)
            if dirpath == "mc_data" or dirpath == "Pairs":
                continue

            # first check if dirpath does not contain pattern
            if self.not_contain_pattern != "":
                m = re.search(self.not_contain_pattern, dirpath)
                if m:
                    continue

            is_good = True
            # print path
            for pattern in self.patterns:
                m = re.search(pattern, dirpath)
                if not m:
                    is_good = False
                    break
            if is_good:
                # check if there are useful files here
                found_files = False
                if len(file_patterns) == 1:
                    found_files = [x for x in files if glob_patterns in x]
                else:
                    for filename in files:
                        found_file = True
                        for pattern in file_patterns:
                            if pattern not in filename:
                                found_file = False
                                break
                        if found_file:
                            found_files = True
                            break

                if found_files:
                    self.dirs.append(dirpath)


class ConfigModifier:
    def __init__(self):
        pass

    def loadConfig(self, config_file_path):
        f = open(config_file_path, "r")
        return json.loads(f.read())

    def writeConfigToPath(self, config, config_file_path):
        f = open(config_file_path, "w")
        f.write(json.dumps(config, indent=2, separators=(",", ": ")))
