import json
from pathlib import Path
from typing import Type, TypeVar, Union

import cattrs
from lumifit.types import (
    AlignmentParameters,
    ExperimentParameters,
    ReconstructionParameters,
    SimulationParameters,
)

cattrs.register_structure_hook(Path, lambda d, t: Path(d))
cattrs.register_unstructure_hook(Path, lambda d: str(d))


Params = Union[ExperimentParameters, AlignmentParameters, SimulationParameters, ReconstructionParameters]
T = TypeVar("T", bound=Params)


def write_params_to_file(params: Params, pathname: Path, filename: str, overwrite: bool = False) -> None:
    Path(pathname).mkdir(exist_ok=True, parents=True)
    file_path = pathname / filename
    if overwrite or not file_path.exists():
        print(f"creating config file: {file_path}")
        with open(file_path, "w") as json_file:
            json.dump(cattrs.unstructure(params), json_file, sort_keys=True, indent=4)  # , cls=_EnumEncoder)
    else:
        print(f"Config file {filename} already exists!")


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
