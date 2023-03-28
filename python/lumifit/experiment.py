#!/usr/bin/env python3

from enum import Enum
from pathlib import Path

from attrs import define, field

from .alignment import AlignmentParameters
from .reconstruction import ReconstructionParameters
from .simulation import SimulationParameters, generateDirectory


class ExperimentType(Enum):
    LUMI = "LUMI"
    KOALA = "KOALA"


class ClusterEnvironment(Enum):
    HIMSTER = "HimsterII"
    VIRGO = "Virgo"


@define
class Experiment:
    """
    Dataclass for simulation, reconstruction and alignment.
    """

    experimentType: ExperimentType
    cluster: ClusterEnvironment
    simParams: SimulationParameters
    recoParams: ReconstructionParameters
    alignParams: AlignmentParameters
    baseDataOutputDir: Path
    LMDdirectory: Path
    fitConfigPath: Path = field(default=Path("fitconfig-fast.json"))

    def updateBaseDataDirectory(self) -> None:
        self.baseDataOutputDir = self.LMDdirectory / generateDirectory(self.simParams, self.alignParams)
