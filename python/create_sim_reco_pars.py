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
    recopars.num_box_samples = 500
    recopars.num_events_per_box_sample = 100000

    experiment = Experiment(
        experimentType,
        ClusterEnvironment.HIMSTER,
        simpars,
        recopars,
        alignpars,
        Path("/lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_4.1GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000"),
    )
    return experiment


confPath = Path("expConfigs/PANDA/")
confPath.mkdir(parents=True, exist_ok=True)


for mom in (1.5, 4.06, 8.1, 11.09, 15):

    experiment = genExperimentConfig(mom, ExperimentType.LUMI)

    write_params_to_file(
        cattrs.unstructure(experiment), ".", f"{confPath}/{mom}.config"
    )
