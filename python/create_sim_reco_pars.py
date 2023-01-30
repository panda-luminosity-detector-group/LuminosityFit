#!/usr/bin/env python3

from dotenv import load_dotenv

load_dotenv(dotenv_path="../lmdEnvFile.env", verbose=True)

import argparse
import os
from pathlib import Path
from typing import Union

import cattrs
from lumifit.alignment import AlignmentParameters
from lumifit.experiment import ClusterEnvironment, Experiment, ExperimentType
from lumifit.general import write_params_to_file
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import SimulationParameters
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
    """
    Generates a default experiment config without misalignment or alignment.
    Is mis/alignment is wanted, simply change the alignpars attribute and call
    experiment.updateBaseDataDirectory()
    """
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

    lmdfit_data_dir: Union[None, Path, str] = os.getenv("LMDFIT_DATA_DIR")

    if lmdfit_data_dir is not None:
        lmdfit_data_dir = Path(lmdfit_data_dir)
    else:
        raise ValueError("Please set $LMDFIT_DATA_DIR!")

    experiment = Experiment(
        experimentType,
        ClusterEnvironment.HIMSTER,
        simpars,
        recopars,
        alignpars,
        LMDdirectory=lmdfit_data_dir,
    )

    return experiment


def restrictPhiConfigs():
    for mom in momenta:

        for i in range(len(upperPhiAngles)):

            # PANDA configs
            experiment = genExperimentConfig(
                mom,
                theta_min[0],
                theta_max[0],
                phi_min[0],
                upperPhiAngles[i],
                ExperimentType.LUMI,
                SimulationType.RESACCBOX,
            )

            # change simpars if misalignment matrices are given
            if args.misalignMatrixFileP is not None:
                experiment.alignParams.misalignment_matrices_path = (
                    f'/home/roklasen/LMD-Alignment/output/misMat-nothing-{phiMatNames[i]}.json'
                )
            # change alignpars is alignment matrices are given

            # update internal paths
            experiment.updateBaseDataDirectory()

            write_params_to_file(
                cattrs.unstructure(experiment),
                ".",
                f"{confPathPanda}/{mom}.config",
                overwrite=True,
            )

def genConfigs():
    
    for mom in momenta:

        # PANDA configs

        experiment = genExperimentConfig(
            mom,
            theta_min[0],
            theta_max[0],
            phi_min[0],
            phi_max[0],
            ExperimentType.LUMI,
            SimulationType.RESACCBOX,
        )

        # change simpars if misalignment matrices are given
        if args.misalignMatrixFileP is not None:
            experiment.alignParams.misalignment_matrices_path = (
                args.misalignMatrixFileP
            )
        # change alignpars is alignment matrices are given

        # update internal paths
        experiment.updateBaseDataDirectory()

        write_params_to_file(
            cattrs.unstructure(experiment),
            ".",
            f"{confPathPanda}/{mom}.config",
            overwrite=True,
        )

        # KOALA configs

        experiment = genExperimentConfig(
            mom,
            theta_min[1],
            theta_max[1],
            phi_min[1],
            phi_max[1],
            ExperimentType.KOALA,
            SimulationType.RESACCPBARP_ELASTIC,
        )

        # change simpars if misalignment matrices are given
        if args.misalignMatrixFileK is not None:
            experiment.alignParams.misalignment_matrices_path = (
                args.misalignMatrixFileK
            )

        # change alignpars is alignment matrices are given

        # update internal paths
        experiment.updateBaseDataDirectory()

        write_params_to_file(
            cattrs.unstructure(experiment), ".", f"{confPathKoala}/{mom}.config"
        )



parser = argparse.ArgumentParser()
parser.add_argument(
    "-b", dest="inBetweenMomenta", action=argparse._StoreTrueAction
)
parser.add_argument(
    "-mp",
    dest="misalignMatrixFileP",
    help="Misalignment Maitrx absolute filename. Use with PANDA experiment.",
    type=Path,
)
parser.add_argument(
    "-mk",
    dest="misalignMatrixFileK",
    help="Misalignment Maitrx absolute filename. Use with KOALA experiment.",
    type=Path,
)

parser.add_argument('-rphi', dest="restrictPhi", help="generate multiple configs with restricted phi angles, no misalignment.", action=argparse._StoreTrueAction)

args = parser.parse_args()

# destination path
confPathPanda = Path("expConfigs/PANDA/")
confPathKoala = Path("expConfigs/KOALA/")

if args.misalignMatrixFileP is not None:
    confPathPanda = confPathPanda / args.misalignMatrixFileP.stem

if args.misalignMatrixFileK is not None:
    confPathKoala = confPathKoala / args.misalignMatrixFileK.stem

confPathPanda.mkdir(parents=True, exist_ok=True)
confPathKoala.mkdir(parents=True, exist_ok=True)

# angles
theta_min = (2.7, 4.8)
theta_max = (13.0, 20.0)
phi_min = (0.0, 0.0)
phi_max = (6.28318531718, 6.28318531718)

if args.inBetweenMomenta:
    momenta = [1.75, 3.5, 9.5, 13.0]
else:
    momenta = [1.5, 4.06, 8.9, 11.91, 15.0]


upperPhiAngles = (6.28318531718, 6.28318531718/2, 6.28318531718/4, 6.28318531718/8, 6.28318531718/10, 6.28318531718/12)
phiMatNames = ('2pi','2piO2','2piO4','2piO8','2piO10','2piO12')

#* special case: resticted phi angles. we'll have to cheat a little here with a misalignment matrix
# that has no misalignmen, so that we get new paths for each new angle

if args.restrictPhi:
    restrictPhiConfigs()

genConfigs()