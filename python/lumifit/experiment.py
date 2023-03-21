#!/usr/bin/env python3

from enum import Enum
from pathlib import Path
from typing import Union

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
    fitConfigPath: Path = field(default=Path("fitconfig-fast.json"))
    baseDataOutputDir: Path = field(default=Path())
    LMDdirectory: Path = field(default=Path())

    def __attrs_post_init__(self) -> None:
        self.updateBaseDataDirectory()

    def updateBaseDataDirectory(self) -> None:
        if self.LMDdirectory is not None:
            self.baseDataOutputDir = self.LMDdirectory / generateDirectory(self.simParams, self.alignParams)
        else:
            raise AttributeError("Cannot update internal path! Please set LMD software directory first.")
