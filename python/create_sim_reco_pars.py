#!/usr/bin/env python3

from dotenv import load_dotenv

load_dotenv(dotenv_path="../lmdEnvFile.env", verbose=True)

import cattrs
from attrs import asdict
from lumifit.alignment import AlignmentParameters
from lumifit.experiment import ClusterEnvironment, Experiment, ExperimentType
from lumifit.general import load_params_from_file, write_params_to_file
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import SimulationParameters

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
