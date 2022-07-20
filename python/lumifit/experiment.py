#!/usr/bin/env python3

from enum import Enum

from attrs import define

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
