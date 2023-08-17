"""
container class for a data set found by determineLuminosity.py.

only used by determineLuminosity.py internally, should never be written to
or read from file!
"""

from enum import Enum, IntEnum
from typing import List

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
    """
    Wait, this holds largely the same info as types.DataMode, do we really need both?
    """

    NONE = "none"
    ANGULAR = "a"
    VERTEX = "v"
    EFFICIENCY_RESOLUTION = "er"
    HIST = "h"


@define
class SimulationTask:
    simDataType: SimulationDataType = SimulationDataType.NONE
    simState: SimulationState = SimulationState.INIT
    lastState: SimulationState = SimulationState.INIT


@define
class SimRecipe:
    """
    Scenarios are always simulated on a cluster, but all runtime parameters are set during construction!
    # also the name scenario is wrong, this is more of a todo-list of running jobs
    # maybe a better name would be "simulationRecipe"?
    """

    elastic_pbarp_integrated_cross_secion_in_mb: float = 0.0

    SimulationTasks: List[SimulationTask] = []
    lumiDetState: LumiDeterminationState = LumiDeterminationState.SIMULATE_VERTEX_DATA
    lastLumiDetState: LumiDeterminationState = LumiDeterminationState.INIT

    is_broken: bool = False
