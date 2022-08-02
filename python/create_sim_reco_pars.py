#!/usr/bin/env python3

from dotenv import load_dotenv

load_dotenv(dotenv_path="../lmdEnvFile.env", verbose=True)

from pathlib import Path

import cattrs
from lumifit.alignment import AlignmentParameters
from lumifit.experiment import ClusterEnvironment, Experiment, ExperimentType
from lumifit.general import write_params_to_file
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import SimulationParameters

# write_params_to_file(asdict(simpars), ".", "simparams.config")
# write_params_to_file(asdict(recopars), ".", "recoparams.config")
# write_params_to_file(cattrs.unstructure(experiment), ".", "experiment.config")


def genExperimentConfig(momentum: float, experimentType: Experiment):
    simpars = SimulationParameters()
    recopars = ReconstructionParameters()
    alignpars = AlignmentParameters()

    simpars.lab_momentum = momentum
    simpars.num_samples = 100
    simpars.num_events_per_sample = 100000

    recopars.lab_momentum = momentum
    recopars.num_samples = 100
    recopars.num_events_per_sample = 100000

    experiment = Experiment(
        experimentType,
        ClusterEnvironment.HIMSTER,
        simpars,
        recopars,
        alignpars,
        "",
    )
    return experiment


confPath = Path("expConfigs/PANDA/")
confPath.mkdir(parents=True, exist_ok=True)


for mom in (1.5, 4.06, 8.1, 11.09, 15):

    experiment = genExperimentConfig(mom, ClusterEnvironment.HIMSTER)

    write_params_to_file(
        cattrs.unstructure(experiment), ".", f"{confPath}/{mom}.config"
    )
