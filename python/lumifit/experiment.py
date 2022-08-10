#!/usr/bin/env python3

from enum import Enum
from pathlib import Path

from attrs import define, field

from .alignment import AlignmentParameters
from .reconstruction import ReconstructionParameters
from .simulation import SimulationParameters


class ExperimentType(Enum):
    LUMI = "LUMI"
    KOALA = "KOALA"


class ClusterEnvironment(Enum):
    HIMSTER = "HimsterII"
    VIRGO = "Virgo"


@define
class Experiment:
    experimentType: ExperimentType
    cluster: ClusterEnvironment
    simParams: SimulationParameters
    recoParams: ReconstructionParameters
    alignParams: AlignmentParameters
    baseDataOutputDir: Path
    fitConfigPath: Path = field(default=Path("fitconfig-fast.json"))
