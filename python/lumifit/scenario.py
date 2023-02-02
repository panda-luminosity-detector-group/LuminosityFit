"""
container class for a data set found by determineLuminosity.py.

only used by determineLuminosity.py internally, should never be written to
or read from file!
"""

from .alignment import AlignmentParameters
from .experiment import ExperimentType

# this should be set during construction
# lmdScriptPath = os.environ["LMDFIT_SCRIPTPATH"]


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
        # self.rec_ip_info = {}
        self.ipX = 0.0
        self.ipY = 0.0
        self.ipZ = 0.0
        self.elastic_pbarp_integrated_cross_secion_in_mb = None
        self.use_m_cut = True
        self.use_xy_cut = True
        self.use_ip_determination = True
        self.Lumi = True
        if experiment_type == ExperimentType.LUMI:

            # * these should be in scenrio at all, they are config parameters!
            # self.phi_min_in_rad = 0.0
            # self.phi_max_in_rad = 2 * math.pi

            self.Sim = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdSimReco.py"
            self.Reco = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdReco.py"
            self.LmdData = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/createLumiFitData.sh"
            self.track_file_pattern = "Lumi_TrksQA_"
        elif experiment_type == ExperimentType.KOALA:

            # * these should be in scenrio at all, they are config parameters!
            # self.phi_min_in_rad = 0.9 * math.pi
            # self.phi_max_in_rad = 1.3 * math.pi

            self.Sim = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaSimReco.py"
            self.Reco = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaReco.py"
            self.LmdData = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/createKoaFitData.sh"
            self.track_file_pattern = "Koala_IP_"
        else:
            raise ValueError("Experiment Type not defined!")

        # don't define default args here, better let fail with None
        self.alignment_parameters: AlignmentParameters = None

        self.state = 1
        self.last_state = 0

        # what the hell is this?
        self.simulation_info_lists = []

        self.is_broken = False
