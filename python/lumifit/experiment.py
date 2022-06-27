#!/usr/bin/env python3

from .simulation import SimulationParameters
from .reconstruction import ReconstructionParameters
from .alignment import AlignmentParameters

from enum import Enum
from attrs import define


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
