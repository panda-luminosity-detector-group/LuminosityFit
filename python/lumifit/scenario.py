"""
holds info about which scenario (LMD or KOALA) is run.
"""

import math
from .alignment import AlignmentParameters
from enum import Enum


class ExperimentType(Enum):
    LUMI = "LUMI"
    KOALA = "KOALA"


class Scenario:
    def __init__(self, dir_path_, experiment_type: ExperimentType):
        self.momentum = 0.0

        self.dir_path = dir_path_
        self.filtered_dir_path = ""
        self.acc_and_res_dir_path = ""
        self.rec_ip_info = {}
        self.elastic_pbarp_integrated_cross_secion_in_mb = None
        self.use_m_cut = True
        self.use_xy_cut = True
        self.use_ip_determination = True
        self.Lumi = True
        if experiment_type == ExperimentType.LUMI:
            self.phi_min_in_rad = 0
            self.phi_max_in_rad = 2 * math.pi
            self.Sim = "./runLmdSimReco.py"
            self.Reco = "./runLmdReco.py"
        else:
            self.phi_min_in_rad = 0.9 * math.pi
            self.phi_max_in_rad = 1.3 * math.pi
            self.Sim = "./runKoalaSimReco.py"
            self.Reco = "./runKoalaReco.py"

        self.alignment_parameters: AlignmentParameters = AlignmentParameters()

        self.state = 1
        self.last_state = 0

        self.simulation_info_lists = []

        self.is_broken = False
