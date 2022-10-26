#!/usr/bin/env python3

from dotenv import load_dotenv

load_dotenv(dotenv_path="../lmdEnvFile.env", verbose=True)

import argparse
import os
from pathlib import Path

import cattrs
from lumifit.alignment import AlignmentParameters
from lumifit.experiment import ClusterEnvironment, Experiment, ExperimentType
from lumifit.general import write_params_to_file
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import SimulationParameters, generateDirectory
from lumifit.simulationTypes import SimulationType

# write_params_to_file(asdict(simpars), ".", "simparams.config")
# write_params_to_file(asdict(recopars), ".", "recoparams.config")
# write_params_to_file(cattrs.unstructure(experiment), ".", "experiment.config")


def genExperimentConfig(
    momentum: float,
    theta_min: float,
    theta_max: float,
    phi_min: float,
    phi_max: float,
    experimentType: ExperimentType,
    sim_type_for_resAcc: SimulationType,
) -> Experiment:
    simpars = SimulationParameters()
    recopars = ReconstructionParameters()
    alignpars = AlignmentParameters()

    simpars.lab_momentum = momentum
    simpars.num_samples = 100
    simpars.num_events_per_sample = 100000
    simpars.theta_min_in_mrad = theta_min
    simpars.theta_max_in_mrad = theta_max
    simpars.phi_min_in_rad = phi_min
    simpars.phi_max_in_rad = phi_max

    recopars.lab_momentum = momentum
    recopars.num_samples = 100
    recopars.num_events_per_sample = 100000
    recopars.num_box_samples = 500
    recopars.num_events_per_box_sample = 100000
    recopars.sim_type_for_resAcc = sim_type_for_resAcc

    lmdfit_data_dir = os.getenv("LMDFIT_DATA_DIR")

    if lmdfit_data_dir is None:
        raise ValueError('Please set $LMDFIT_DATA_DIR!')

    dirname = generateDirectory(simpars, alignpars)

    experiment = Experiment(
        experimentType,
        ClusterEnvironment.HIMSTER,
        simpars,
        recopars,
        alignpars,
        Path(lmdfit_data_dir + "/" + dirname),
    )

    return experiment


parser = argparse.ArgumentParser()
parser.add_argument(
    "-b", dest="inBetweenMomenta", action=argparse._StoreTrueAction
)


args = parser.parse_args()

confPathPanda = Path("expConfigs/PANDA/")
confPathPanda.mkdir(parents=True, exist_ok=True)
confPathKoala = Path("expConfigs/KOALA/")
confPathKoala.mkdir(parents=True, exist_ok=True)


theta_min = (2.7, 4.8)
theta_max = (13.0, 20.0)
phi_min = (0.0, 0.0)
phi_max = (6.28318531718, 6.28318531718)

if args.inBetweenMomenta:
    momenta = [1.75, 3.5, 9.5, 13.0]
else:
    momenta = [1.5, 4.06, 8.9, 11.91, 15.0]

for mom in momenta:

    experiment = genExperimentConfig(
        mom,
        theta_min[0],
        theta_max[0],
        phi_min[0],
        phi_max[0],
        ExperimentType.LUMI,
        SimulationType.RESACCBOX,
    )

    write_params_to_file(
        cattrs.unstructure(experiment), ".", f"{confPathPanda}/{mom}.config"
    )

    experiment = genExperimentConfig(
        mom,
        theta_min[1],
        theta_max[1],
        phi_min[1],
        phi_max[1],
        ExperimentType.KOALA,
        SimulationType.RESACCPBARP_ELASTIC,
    )

    write_params_to_file(
        cattrs.unstructure(experiment), ".", f"{confPathKoala}/{mom}.config"
    )
