#!/usr/bin/env python3

from attrs import asdict
from lumifit.general import write_params_to_file, load_params_from_file
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import SimulationParameters
from lumifit.experiment import Experiment, ExperimentType, ClusterEnvironment
from lumifit.alignment import AlignmentParameters

import cattrs

simpars = SimulationParameters()
recopars = ReconstructionParameters()
alignpars = AlignmentParameters()
experiment = Experiment(
    ExperimentType.KOALA,
    ClusterEnvironment.HIMSTER,
    simpars,
    recopars,
    alignpars,
)


write_params_to_file(asdict(simpars), ".", "simparams.config")
write_params_to_file(asdict(recopars), ".", "recoparams.config")
write_params_to_file(cattrs.unstructure(experiment), ".", "experiment.config")

verifyExperiment = load_params_from_file("experiment.config", Experiment)

assert verifyExperiment == experiment
print("assert okay!")
