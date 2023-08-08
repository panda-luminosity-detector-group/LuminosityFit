"""
container class for a data set found by determineLuminosity.py.

only used by determineLuminosity.py internally, should never be written to
or read from file!

TODO: maybe integrate this into lumifit.types?
"""

from enum import Enum, IntEnum
from pathlib import Path
from typing import List

# import attr
from attrs import define


class LumiDeterminationState(IntEnum):
    INIT = 0
    SIMULATE_VERTEX_DATA = 1
    DETERMINE_IP = 2
    RECONSTRUCT_WITH_NEW_IP = 3
    RUN_LUMINOSITY_FIT = 4


class SimulationState(IntEnum):
    FAILED = -1
    INIT = 0
    START_SIM = 1
    MAKE_BUNCHES = 2
    MERGE = 3
    DONE = 4


class SimulationDataType(Enum):
    NONE = "none"
    ANGULAR = "a"
    VERTEX = "v"
    EFFICIENCY_RESOLUTION = "er"


@define
class SimulationTask:
    dirPath: Path = Path()
    simDataType: SimulationDataType = SimulationDataType.NONE
    simState: SimulationState = SimulationState.INIT
    lastState: SimulationState = SimulationState.INIT


@define
class Scenario:
    """
    Scenarios are always simulated on a cluster, but all runtime parameters are set during construction!
    """

    momentum = 0.0
    elastic_pbarp_integrated_cross_secion_in_mb: float = 0.0

    trackDirectory: Path = Path()
    filteredTrackDirectory: Path = Path()
    acc_and_res_dir_path: Path = Path()

    SimulationTasks: List[SimulationTask] = []
    lumiDetState = LumiDeterminationState.SIMULATE_VERTEX_DATA
    lastLumiDetState = LumiDeterminationState.INIT

    is_broken: bool = False
