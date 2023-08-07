"""
container class for a data set found by determineLuminosity.py.

only used by determineLuminosity.py internally, should never be written to
or read from file!

TODO: maybe integrate this into lumifit.types?
"""

from enum import Enum, IntEnum
from pathlib import Path
from typing import List

import attr
from lumifit.types import AlignmentParameters, ExperimentType


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


@attr.s
class SimulationTask:
    dirPath: Path = attr.ib(default=Path())
    simDataType: SimulationDataType = attr.ib(default=SimulationDataType.NONE)
    simState: SimulationState = attr.ib(default=SimulationState.INIT)
    lastState: SimulationState = attr.ib(default=SimulationState.INIT)


# TODO: use the @define decorator here, makes this easier to read
class Scenario:
    """
    Scenarios are always simulated on a cluster, but all runtime parameters are set during construction!
    """

    def __init__(self, trackDirectory_: Path, experiment_type: ExperimentType, lmdScriptPath: Path):
        self.momentum = 0.0

        self.trackDirectory = trackDirectory_
        self.filteredTrackDirectory: Path = Path()
        self.acc_and_res_dir_path: Path = Path()
        self.elastic_pbarp_integrated_cross_secion_in_mb: float = 0.0

        if experiment_type == ExperimentType.LUMI:
            self.Sim = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdSimReco.py"
            self.Reco = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdReco.py"
            self.LmdData = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/createLumiFitData.sh"
            self.track_file_pattern = "Lumi_TrksQA_"

        elif experiment_type == ExperimentType.KOALA:
            self.Sim = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaSimReco.py"
            self.Reco = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaReco.py"
            self.LmdData = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/createKoaFitData.sh"
            self.track_file_pattern = "Koala_IP_"
        else:
            raise ValueError("Experiment Type not defined!")

        # don't define default args here, better let fail with None
        self.alignment_parameters: AlignmentParameters = AlignmentParameters()

        self.lumiDetState = LumiDeterminationState.SIMULATE_VERTEX_DATA
        self.lastLumiDetState = LumiDeterminationState.INIT

        # what the hell is this?
        # self.simulation_info_lists = []
        self.SimulationTasks: List[SimulationTask] = []

        self.is_broken = False
