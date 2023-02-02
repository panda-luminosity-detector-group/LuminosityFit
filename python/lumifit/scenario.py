"""
container class for a data set found by determineLuminosity.py.

only used by determineLuminosity.py internally, should never be written to
or read from file!
"""

from enum import IntEnum

from .alignment import AlignmentParameters
from .experiment import ExperimentType


class LumiDeterminationState(IntEnum):
    INIT = 0
    SIMULATE_VERTEX_DATA = 1
    DETERMINE_IP = 2
    RECONSTRUCT_WITH_NEW_IP = 3
    RUN_LUMI_FIT = 4


class SimulationState(IntEnum):
    FAILED = -1
    SIMULATION = 1
    BUNCHES = 2
    MERGE = 3
    DONE = 4


class Scenario:
    """
    Scenarios are always simulated on a cluster, but all runtime parameters are set during construction!
    """

    def __init__(
        self,
        dir_path_: str,
        experiment_type: ExperimentType,
        lmdScriptPath: str,
    ):
        self.momentum = 0.0

        self.trackDirectory = dir_path_
        self.filteredTrackDirectory = ""
        self.acc_and_res_dir_path = ""
        self.elastic_pbarp_integrated_cross_secion_in_mb = None

        # TODO: why are these here at all, they're already in the reco params
        # self.ipX = 0.0
        # self.ipY = 0.0
        # self.ipZ = 0.0
        # self.use_m_cut = True
        # self.use_xy_cut = True
        # self.use_ip_determination = True

        # why is THIS here, this is always true?!
        self.Lumi = True

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
        self.alignment_parameters: AlignmentParameters = None

        self.state = LumiDeterminationState.SIMULATE_VERTEX_DATA
        self.last_state = LumiDeterminationState.INIT

        # what the hell is this?
        self.simulation_info_lists = []

        self.is_broken = False
