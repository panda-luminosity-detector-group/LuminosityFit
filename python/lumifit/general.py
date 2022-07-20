import glob
import json
import os
import re
from argparse import (
    ArgumentDefaultsHelpFormatter,
    ArgumentParser,
    RawTextHelpFormatter,
)
from enum import Enum
from typing import Any

import cattrs


def toCbool(input: bool) -> str:
    """
    returns a string ("true|false") for ROOT macros from a Python bool
    """
    if input:
        return "true"
    else:
        return "false"


def getGoodFiles(
    directory: str,
    glob_pattern: str,
    min_filesize_in_bytes: int = 2000,
    is_bunches: bool = False,
) -> list:
    found_files = glob.glob(directory + "/" + glob_pattern)
    good_files = []
    bad_files = []
    for file in found_files:
        if os.stat(file).st_size > min_filesize_in_bytes:
            good_files.append(file)
        else:
            bad_files.append(file)

    if is_bunches:
        m = re.search(r"\/bunches_(\d+)", directory)
        num_sim_files = int(m.group(1))  # type: ignore
    else:
        m = re.search(r"\/(\d+)-(\d+)_.+?cut", directory)
        num_sim_files = int(m.group(2)) - int(m.group(1)) + 1  # type: ignore

    files_percentage = len(good_files) / num_sim_files

    return [good_files, files_percentage]


def check_stage_success(file_url: str) -> bool:
    if os.path.exists(file_url):
        if os.stat(file_url).st_size > 3000:
            print(f"{file_url} exists and is larger than 3kb!")
            return True

    return False


class SmartFormatter(ArgumentDefaultsHelpFormatter):
    def _split_lines(self, text: str, width: int) -> list:
        return RawTextHelpFormatter._split_lines(self, text, width)


def addDebugArgumentsToParser(parser: ArgumentParser) -> ArgumentParser:
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
    def default(self, obj: Any) -> json.JSONEncoder:
        if isinstance(obj, Enum):
            return obj.value
        return json.JSONEncoder.default(self, obj)


def write_params_to_file(params: dict, pathname: str, filename: str) -> None:
    file_path = pathname + "/" + filename
    if not os.path.exists(file_path):
        print("creating config file: " + file_path)
        with open(file_path, "w") as json_file:
            json.dump(
                params, json_file, sort_keys=True, indent=4, cls=_EnumEncoder
            )
    else:
        print(f"Config file {filename} already exists!")


def load_params_from_file(file_path: str, asType: type):
    if asType is None:
        raise NotImplementedError("Please specify the type to deserialize as.")

    if os.path.exists(file_path):
        with open(file_path, "r") as json_file:
            return cattrs.structure(json.load(json_file), asType)

    print(f"file {file_path} does not exist!")
    return {}


class DirectorySearcher:
    def __init__(
        self, patterns_: list, not_contain_pattern_: str = ""
    ) -> None:
        self.patterns = patterns_
        self.not_contain_pattern = not_contain_pattern_
        self.dirs: list = []

    def getListOfDirectories(self) -> list:
        return self.dirs

    def searchListOfDirectories(self, path: str, glob_patterns: Any) -> None:
        # print("looking for files with pattern: ", glob_patterns)
        # print("dirpath forbidden patterns:", self.not_contain_pattern)
        # print("dirpath patterns:", self.patterns)
        if isinstance(glob_patterns, list):
            file_patterns = glob_patterns
        else:
            file_patterns = [glob_patterns]

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
                    # TODO: this line is highly dubious, but don't touch it for now.
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
    def __init__(self) -> None:
        pass

    def loadConfig(self, config_file_path: str) -> Any:
        f = open(config_file_path, "r")
        return json.loads(f.read())

    def writeConfigToPath(self, config: Any, config_file_path: str) -> None:
        f = open(config_file_path, "w")
        f.write(json.dumps(config, indent=2, separators=(",", ": ")))
