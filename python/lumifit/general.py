import json
import os
import re
from argparse import (
    ArgumentDefaultsHelpFormatter,
    ArgumentParser,
    RawTextHelpFormatter,
)
from enum import Enum
from pathlib import Path
from typing import Any, List, Optional, Type, TypeVar, Union

import cattrs
from alignment import AlignmentParameters
from experiment import Experiment
from reconstruction import ReconstructionParameters
from simulation import SimulationParameters

cattrs.register_structure_hook(Path, lambda d, t: Path(d))
cattrs.register_unstructure_hook(Path, lambda d: str(d))


def envPath(env_var: str) -> Path:
    """
    returns a Path object from an environment variable, ensures that the variable is set.
    raises a ValueError if the variable is not set.
    """
    env_var_value = os.environ.get(env_var)

    if env_var_value:
        return Path(env_var_value)

    raise ValueError(f'Environment variable "{env_var}" not set!')


def toCbool(input: bool) -> str:
    """
    returns a string ("true|false") for ROOT macros from a Python bool
    """
    if input:
        return "true"
    else:
        return "false"


def matrixMacroFileName(input: Optional[Path]) -> str:
    if isinstance(input, Path):
        return str(input)
    elif input is None:
        return ""
    return ""


# TODO: overhaul this function
def getGoodFiles(
    directory: Path,
    glob_pattern: str,
    min_filesize_in_bytes: int = 2000,
    is_bunches: bool = False,
) -> list:
    # TODO: this can be done with pathlibs glob
    found_files = list(directory.glob(glob_pattern))
    good_files = []
    bad_files = []
    for file in found_files:
        if os.stat(file).st_size > min_filesize_in_bytes:
            good_files.append(file)
        else:
            bad_files.append(file)

    if is_bunches:
        m = re.search(r"\/bunches_(\d+)", str(directory))
        if m:
            num_sim_files = int(m.group(1))
        else:
            return [0, 0]
    else:
        m = re.search(r"\/(\d+)-(\d+)_.+?cut", str(directory))
        if m:
            num_sim_files = int(m.group(2)) - int(m.group(1)) + 1
        return [0, 0]

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
        help="If flag is set, the simulation runs locally for " "debug purposes",
    )

    return parser


class _EnumEncoder(json.JSONEncoder):
    def default(self, obj: Any) -> json.JSONEncoder:
        if isinstance(obj, Enum):
            return obj.value
        return json.JSONEncoder.default(self, obj)


def write_params_to_file(params: dict, pathname: Path, filename: str, overwrite: bool = False) -> None:
    Path(pathname).mkdir(exist_ok=True, parents=True)
    file_path = pathname / filename
    if overwrite or (not os.path.exists(file_path)):
        print(f"creating config file: {file_path}")
        with open(file_path, "w") as json_file:
            json.dump(params, json_file, sort_keys=True, indent=4, cls=_EnumEncoder)
    else:
        print(f"Config file {filename} already exists!")


Params = Union[Experiment, AlignmentParameters, SimulationParameters, ReconstructionParameters]
T = TypeVar("T", bound=Params)


def load_params_from_file(file_path: Path, asType: Type[T]) -> T:
    """
    Uses cattrs to deserialize a json file to a python object. Requires target type
    (i.e. AlignmentParams, SimulationParams or ReconstructionParams ) to be specified.
    """
    if asType is None:
        raise NotImplementedError("Please specify the type to deserialize as.")

    if file_path.exists():
        with open(file_path, "r") as json_file:
            return cattrs.structure(json.load(json_file), asType)
    else:
        raise FileNotFoundError(f"file {file_path} does not exist!")


class DirectorySearcher:
    """
    Class to search directories that include files according to a list of patterns.   A pattern is for example a file name. A pattern is for example a file name. A directory that matches any of the patterns is included.


    You can specify a list of patterns that are to be excluded from search. If a directory includes any of the exclude patterns, it is ignored.
    """

    def __init__(self, patterns_: List[str], excludePatterns: str = "") -> None:
        self.patterns = patterns_
        self.not_contain_pattern = excludePatterns
        self.dirs: List[Path] = []

    def getListOfDirectories(self) -> List[Path]:
        return self.dirs

    def searchListOfDirectories(self, path: Path, glob_patterns: Any) -> None:
        """
        Searches a directory for the file patterns given in the constructor.
        Returns all subdirectories that match the given patterns.
        """
        print(f"looking in path: {path}")
        print("looking for files with pattern: ", glob_patterns)
        print("dirpath forbidden patterns:", self.not_contain_pattern)
        print("dirpath patterns:", self.patterns)

        if isinstance(glob_patterns, list):
            file_patterns = glob_patterns
        else:
            file_patterns = [glob_patterns]

        for dirpath in path.glob("**/*"):
            if not dirpath.is_dir():
                continue

            if dirpath.name == "mc_data" or dirpath.name == "Pairs":
                continue

            if self.not_contain_pattern != "" and self.not_contain_pattern in str(dirpath):
                continue

            is_good = True
            for pattern in self.patterns:
                if pattern not in str(dirpath):
                    is_good = False
                    break
            if is_good:
                found_files = False
                for filename in dirpath.glob("*"):
                    if not filename.is_file():
                        continue

                    for pattern in file_patterns:
                        if pattern in filename.name:
                            found_files = True
                            break

                    if found_files:
                        self.dirs.append(dirpath)
                        break


class ConfigModifier:
    """
    Todo: actually just remove that when you're sure you've removed all references to it.
    The config should be read, not modified.
    """

    def __init__(self) -> None:
        pass

    def loadConfig(self, config_file_path: str) -> Any:
        f = open(config_file_path, "r")
        return json.loads(f.read())

    def writeConfigToPath(self, config: Any, config_file_path: str) -> None:
        f = open(config_file_path, "w")
        f.write(json.dumps(config, indent=2, separators=(",", ": ")))
