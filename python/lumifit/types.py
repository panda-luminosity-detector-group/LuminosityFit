#!/usr/bin/env python3

"""
This module contains dataclasses for simulation, reconstruction and alignment.

In the future, this can maybe be split again into smaller modules in a separate folder,
but for now it is easier to put it here while I resolve circular dependencies.
"""

import math
import random
import string
from enum import Enum
from pathlib import Path
from typing import Optional

from attrs import define, field


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
    RESACCBOX = "resAcc-box"
    RESACCPBARP_ELASTIC = "resAcc-dpm_elastic"


@define
class AlignmentParameters:
    use_point_transform_misalignment: bool = False
    alignment_matrices_path: Optional[Path] = None
    misalignment_matrices_path: Optional[Path] = None


@define
class SimulationParameters:
    # Using default_factory for random value defaults
    random_seed: int = field(factory=lambda: random.randint(10, 9999))  # type: ignore

    simGeneratorType: SimulationGeneratorType = SimulationGeneratorType.PBARP_ELASTIC
    num_events_per_sample: int = 1000
    num_samples: int = 1
    lab_momentum: float = 1.5
    low_index: int = 1
    output_dir: Optional[Path] = None
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


@define
class ReconstructionParameters:
    simGenTypeForResAcc: SimulationGeneratorType = SimulationGeneratorType.BOX
    num_events_per_sample: int = 1000
    num_samples: int = 1
    lab_momentum: float = 1.5
    low_index: int = 1
    output_dir: Optional[Path] = None
    lmd_geometry_filename: str = "Luminosity-Detector.root"
    use_ip_determination: bool = True
    use_xy_cut: bool = True
    use_m_cut: bool = True
    track_search_algo: str = "CA"
    recoIPX: float = 0.0
    recoIPY: float = 0.0
    recoIPZ: float = 0.0
    force_cut_disable: bool = False

    num_resAcc_samples: int = 500
    num_events_per_resAcc_sample: int = 10000


@define
class ExperimentParameters:
    """
    Dataclass for simulation, reconstruction and alignment.
    """

    experimentType: ExperimentType
    cluster: ClusterEnvironment
    simParams: SimulationParameters
    recoParams: ReconstructionParameters
    alignParams: AlignmentParameters
    LMDdirectory: Path
    fitConfigPath: Path = Path("fitconfig-fast.json")

    baseDataOutputDir: Path = field(init=False)

    # generate a random directory for the simulation output
    def __attrs_post_init__(self) -> None:
        self.baseDataOutputDir = self.LMDdirectory / Path("LMD-" + "".join(random.choices(string.ascii_letters, k=10)))

    # def updateBaseDataDirectory(self) -> None:
    #     self.baseDataOutputDir = self.LMDdirectory / generateDirectory(self.simParams, self.alignParams)


# def generateDirectory(
#     sim_params: SimulationParameters,
#     align_params: AlignmentParameters,
#     output_dir: Optional[Path] = None,
# ) -> Path:
#     if output_dir is None:
#         if sim_params.useRestGas:
#             dirname = Path(f"plab_{sim_params.lab_momentum:.2f}GeV_RestGas")
#         else:
#             dirname = Path(f"plab_{sim_params.lab_momentum:.2f}GeV")

#         gen_part = f"{sim_params.simGeneratorType.value}_theta_" + f"{sim_params.theta_min_in_mrad}-" + f"{sim_params.theta_max_in_mrad}mrad"
#         if not sim_params.neglect_recoil_momentum:
#             gen_part += "_recoil_corrected"

#         dirname /= Path(gen_part)

#         dirname /= Path(
#             "/ip_offset_XYZDXDYDZ_"
#             + f"{sim_params.ip_offset_x}_{sim_params.ip_offset_y}_"
#             + f"{sim_params.ip_offset_z}_{sim_params.ip_spread_x}_"
#             + f"{sim_params.ip_spread_y}_{sim_params.ip_spread_z}"
#         )

#         dirname /= Path(
#             "beam_grad_XYDXDY_" + f"{sim_params.beam_tilt_x}_{sim_params.beam_tilt_y}_" + f"{sim_params.beam_divergence_x}_" + f"{sim_params.beam_divergence_y}"
#         )

#         if align_params.use_point_transform_misalignment or align_params.misalignment_matrices_path is None:
#             dirname /= "/no_geo_misalignment"
#         else:
#             dirname = dirname / "geo_misalignment-" / align_params.misalignment_matrices_path.stem

#         dirname /= str(sim_params.num_events_per_sample)

#         return dirname

#     return output_dir
