"""
This module contains dataclasses for simulation, reconstruction and alignment.

In the future, this can maybe be split again into smaller modules in a separate folder,
but for now it is easier to put it here while I resolve circular dependencies.
"""

import math
from enum import Enum
from pathlib import Path
from typing import Optional

from attrs import define


class TrackSearchAlgorithm(Enum):
    CA = "CA"
    FOLLOW = "Follow"


class ExperimentType(Enum):
    LUMI = "LUMI"
    KOALA = "KOALA"


class ClusterEnvironment(Enum):
    HIMSTER = "HimsterII"
    VIRGO = "Virgo"


class SimulationGeneratorType(Enum):
    BOX = "box"
    PBARP_ELASTIC = "dpm_elastic"
    PBARP = "dpm_elastic_inelastic"
    NOISE = "noise"


class DataMode(Enum):
    DATA = "data"  # reconstructed data (doesn't matter if MC or real data)
    VERTEXDATA = "vertexData"  # vertex data must always be reconstructed without cuts
    RESACC = "resAcc"  # data for resolution/Acceptance maps


@define(frozen=True)
class AlignmentParameters:
    use_point_transform_misalignment: bool = False
    alignment_matrices_path: Optional[Path] = None
    misalignment_matrices_path: Optional[Path] = None


@define(frozen=True)
class SimulationParameters:
    # Using default_factory for random value defaults
    simulationCommand: str

    random_seed: int

    simGeneratorType: SimulationGeneratorType = SimulationGeneratorType.PBARP_ELASTIC
    num_events_per_sample: int = 100000
    num_samples: int = 100
    lab_momentum: float = 1.5
    low_index: int = 1
    lmd_geometry_filename: str = "Luminosity-Detector.root"
    theta_min_in_mrad: float = 2.7
    theta_max_in_mrad: float = 13.0
    phi_min_in_rad: float = 0.0
    phi_max_in_rad: float = 2 * math.pi
    neglect_recoil_momentum: bool = False

    useRestGas: bool = False
    ip_offset_x: float = 0.0  # in cm
    ip_offset_y: float = 0.0  # in cm
    ip_offset_z: float = 0.0  # in cm
    ip_spread_x: float = 0.0  # in cm
    ip_spread_y: float = 0.0  # in cm
    ip_spread_z: float = 0.0  # in cm
    beam_tilt_x: float = 0.0  # in rad
    beam_tilt_y: float = 0.0  # in rad
    beam_divergence_x: float = 0.0  # in rad
    beam_divergence_y: float = 0.0  # in rad

    def setNewIPPosition(self, x: float, y: float, z: float) -> None:
        """
        Yes, I hate it too, the data class is supposed to be immutable.
        But better to do it here (and document it) than to do it in the
        determineLuminosity.py or elsewhere and forget about it.
        """
        print("Attention! Setting new IP position")
        object.__setattr__(self, "ip_offset_x", x)
        object.__setattr__(self, "ip_offset_y", y)
        object.__setattr__(self, "ip_offset_z", z)

    def setNewThetaAngles(self, thetaMin: float, thetaMax: float) -> None:
        """
        Yes, I hate it too, the data class is supposed to be immutable.
        But better to do it here (and document it) than to do it in the
        determineLuminosity.py or elsewhere and forget about it.
        """
        print("Attention! Setting new theta angles")
        object.__setattr__(self, "theta_min_in_mrad", thetaMin)
        object.__setattr__(self, "theta_max_in_mrad", thetaMax)


@define(frozen=True)
class ReconstructionParameters:
    reconstructionCommand: str
    num_events_per_sample: int = 100000
    num_samples: int = 100
    lab_momentum: float = 1.5
    low_index: int = 1
    lmd_geometry_filename: str = "Luminosity-Detector.root"
    use_ip_determination: bool = True
    use_xy_cut: bool = True
    use_m_cut: bool = True
    track_search_algo: str = "CA"
    recoIPX: float = 0.0
    recoIPY: float = 0.0
    recoIPZ: float = 0.0
    force_cut_disable: bool = False

    def setNewIPPosition(self, x: float, y: float, z: float) -> None:
        """
        Yes, I hate it too, the data class is supposed to be immutable.
        But better to do it here (and document it) than to do it in the
        determineLuminosity.py or elsewhere and forget about it.
        """
        print("Attention! Setting new IP position")
        object.__setattr__(self, "recoIPX", x)
        object.__setattr__(self, "recoIPY", y)
        object.__setattr__(self, "recoIPZ", z)

    def disableCuts(self) -> None:
        """
        This is only needed for VertexData, which must always be reconstructed without cuts.
        """
        print("----------------------------------------------------------------------------------------")
        print("Attention! This experiment configs specifies to use XY and m cuts during reconstruction.")
        print("That's reasonable for the luminosity determination, but the initial data sample must    ")
        print("still be generated without cuts first.                                                  ")
        print("                                                                                        ")
        print("                            Disabling all cuts for this run!                            ")
        print("----------------------------------------------------------------------------------------")
        object.__setattr__(self, "use_xy_cut", False)
        object.__setattr__(self, "use_m_cut", False)


@define(frozen=True)
class SoftwarePaths:
    """
    hold paths where the LumiFit binaries and scripts are located, and where PandaRoot macros are.
    """

    LmdFitBuildDir: Path  # where the LmdFit build is (i.e. LuminosityFit/build/) but NOT ./bin/
    LmdFitScripts: Path  # where the python scripts for LmdFit are
    PandaRootMacroPath: Path  # where the macros for PandaRoot are


@define(frozen=True)
class ConfigPackage:
    """
    This package holds all config info that is relevant to data or resAcc modes.

    data: either simulation data or real experimental data:
            - (simParams)
            - (MCDataDir)
            - baseDataDir
            - recoParams
            - alignParams
    resAcc: resolution acceptance data generated specifically for the data
            - simParams
            - MCDataDir
            - baseDataDir
            - recoParams
            - alignParams
    """

    recoParams: ReconstructionParameters
    alignParams: AlignmentParameters
    baseDataDir: Path

    # these options are really only applicable if we use simulated data,
    # for real data, these are none
    MCDataDir: Optional[Path] = None
    simParams: Optional[SimulationParameters] = None


@define(frozen=True)
class ExperimentParameters:
    """
    Dataclass for simulation, reconstruction and alignment.
    """

    experimentType: ExperimentType  # PANDA or KOALA
    cluster: ClusterEnvironment  # HimsterII or Virgo

    # LmdFitDataDir: Path  # the lustreFS dir where all simulation data is stored

    # this depends on PANDA or KOALA case
    trackFilePattern: str
    LMDDataCommand: str

    # where is the LMDFit Software and PandaROOT?
    softwarePaths: SoftwarePaths

    # sim,reco,align params for both modes
    dataPackage: ConfigPackage
    resAccPackage: ConfigPackage

    # absolute path for this specific experiment
    experimentDir: Path

    # absolute path to the fit config
    fitConfigPath: Path

    # absolute path to the data config (for bunches/binning)
    dataConfigPath: Path

    # path to the vertex config (needed to reconstruct the IP)
    vertexConfigPath: Path

    # this must only be there if the reconstructed IP is used
    recoIPpath: Optional[Path] = None

    lumiFileName: Optional[Path] = None

    def isConsistent(self) -> bool:
        if self.dataPackage.recoParams.use_ip_determination != self.resAccPackage.recoParams.use_ip_determination:
            return False
        
        if self.dataPackage.simParams.lab_momentum != self.resAccPackage.simParams.lab_momentum:
            return False

        return True
