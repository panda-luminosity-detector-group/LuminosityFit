"""
container class for a data set found b determineLuminosity.py.
"""

import math
import os

from .alignment import AlignmentParameters
from .experiment import ExperimentType

lmdScriptPath = os.environ["LMDFIT_SCRIPTPATH"]


class Scenario:
    """
    Scenarios are always simualted on a cluster, so for now, the singularityJob.sh
    wrappre can be here. this means we need env variables, which means we need os...
    """

    def __init__(self, dir_path_: str, experiment_type: ExperimentType):
        self.momentum = 0.0

        self.dir_path = dir_path_
        self.filtered_dir_path = ""
        self.acc_and_res_dir_path = ""
        self.rec_ip_info: dict = {}
        self.elastic_pbarp_integrated_cross_secion_in_mb = None
        self.use_m_cut = True
        self.use_xy_cut = True
        self.use_ip_determination = True
        self.Lumi = True
        if experiment_type == ExperimentType.LUMI:
            self.phi_min_in_rad = 0.0
            self.phi_max_in_rad = 2 * math.pi
            self.Sim = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdSimReco.py"
            self.Reco = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdReco.py"
            self.track_file_pattern = "Lumi_TrksQA_"
            self.filenamePrefix = "Lumi_TrksQA_"
        elif experiment_type == ExperimentType.KOALA:
            self.phi_min_in_rad = 0.9 * math.pi
            self.phi_max_in_rad = 1.3 * math.pi
            self.Sim = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaSimReco.py"
            self.Reco = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaReco.py"
            self.track_file_pattern = "Koala_Track_"
            self.filename_prefix = "Koala_comp_"
        else:
            raise ValueError("Experiment Type not defined!")

        self.alignment_parameters: AlignmentParameters = AlignmentParameters()

        self.state = 1
        self.last_state = 0

        # what the hell is this?
        self.simulation_info_lists: list = []

        self.is_broken = False
