"""
container class for a data set found by determineLuminosity.py.

only used by determineLuminosity.py internally, should never be written to
or read from file!
"""

from enum import Enum, IntEnum
from typing import List

from attrs import define, field


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

    TODO: also, e and r can be individual modes

    #! ATTENTION! Never change the literals for these enums (a, v, er, h)
    #! they are needed by the binaries that crate lmdData and merge them.
    """

    NONE = "none"  # none bro, trust me bro
    ANGULAR = "a"  # angular data is reconstructed MC data after the IP was reconstructed, that means with cuts
    VERTEX = "v"  # vertex data is simulated MC data before the IP was reconstructed, that means without cuts
    EFFICIENCY_RESOLUTION = "er"  # full set of simulation and reconstruction data WITH offset IP for efficiency and resolution
    HIST = "h"  # no idea, probably deprecated


@define
class SimulationTask:
    simDataType: SimulationDataType = SimulationDataType.NONE
    simState: SimulationState = SimulationState.INIT


@define
class SimRecipe:
    """
    This is a shopping-list-style object that holds at which step the simulation currently is.
    It is never to be written to disk, it only resides in memory for the determineLuminosity script.
    """

    SimulationTasks: List[SimulationTask] = field(factory=list)
