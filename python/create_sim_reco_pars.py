#!/usr/bin/env python3

import argparse
import random
import string
from pathlib import Path

from attrs import evolve
from lumifit.config import write_params_to_file
from lumifit.general import envPath

# from lumifit.simulation import generateDirectory
from lumifit.types import (
    AlignmentParameters,
    ClusterEnvironment,
    ConfigPackage,
    ExperimentParameters,
    ExperimentType,
    ReconstructionParameters,
    SimulationGeneratorType,
    SimulationParameters,
    SoftwarePaths,
)


def genExperimentConfig(
    momentum: float,
    theta_min: float,
    theta_max: float,
    phi_min: float,
    phi_max: float,
    experimentType: ExperimentType,
    simGenTypeForResAcc: SimulationGeneratorType,
) -> ExperimentParameters:
    """
    Generates a default experiment config without misalignment or alignment.

    # TODO: add misalignment and alignment parameters
    """

    lmdScriptPath = envPath("LMDFIT_SCRIPTPATH")
    lmdfit_data_dir = envPath("LMDFIT_DATA_DIR")
    lmd_build_path = envPath("LMDFIT_BUILD_PATH")
    PNDmacropath = envPath("LMDFIT_MACROPATH")
    LMDscriptpath = envPath("LMDFIT_SCRIPTPATH")

    if experimentType == ExperimentType.LUMI:
        simulationCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdSimReco.py"
        reconstructionCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdReco.py"
        LMDDataCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/createLumiFitData.sh"
        trackFilePattern = "Lumi_TrksQA_"

    elif experimentType == ExperimentType.KOALA:
        simulationCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaSimReco.py"
        reconstructionCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runKoaReco.py"
        LMDDataCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/createKoaFitData.sh"
        trackFilePattern = "Koala_IP_"
    else:
        raise ValueError("Experiment Type not defined!")

    experimentDir = lmdfit_data_dir / Path("LMD-" + "".join(random.choices(string.ascii_letters, k=8)))

    simParamsData = SimulationParameters(
        simulationCommand=simulationCommand,
        lab_momentum=momentum,
        num_samples=100,
        num_events_per_sample=100000,
        theta_min_in_mrad=theta_min,
        theta_max_in_mrad=theta_max,
        phi_min_in_rad=phi_min,
        phi_max_in_rad=phi_max,
        random_seed=random.randint(11, 9999),
    )
    recoParamsData = ReconstructionParameters(
        reconstructionCommand=reconstructionCommand,
        lab_momentum=momentum,
        num_samples=100,
        num_events_per_sample=100000,
    )

    dataPackage = ConfigPackage(
        simParams=simParamsData,
        recoParams=recoParamsData,
        alignParams=AlignmentParameters(),
        baseDataDir=lmdfit_data_dir / experimentDir / "data",
        MCDataDir=lmdfit_data_dir / experimentDir / "data/mc_data",
    )

    simParamsResAcc = SimulationParameters(
        simulationCommand=simulationCommand,
        lab_momentum=momentum,
        num_samples=500,
        num_events_per_sample=100000,
        theta_min_in_mrad=theta_min,
        theta_max_in_mrad=theta_max,
        phi_min_in_rad=phi_min,
        phi_max_in_rad=phi_max,
        random_seed=random.randint(11, 9999),
        simGeneratorType=simGenTypeForResAcc,
    )

    recoParamsResAcc = ReconstructionParameters(
        reconstructionCommand=reconstructionCommand,
        lab_momentum=momentum,
        num_samples=500,
        num_events_per_sample=100000,
    )

    resAccPackage = ConfigPackage(
        simParams=simParamsResAcc,
        recoParams=recoParamsResAcc,
        alignParams=AlignmentParameters(),
        baseDataDir=lmdfit_data_dir / experimentDir / "resacc",
        MCDataDir=lmdfit_data_dir / experimentDir / "resacc/mc_data",
    )

    # = Path("LMD-" + "".join(random.choices(string.ascii_letters, k=10)))  # type: ignore

    softwarePaths = SoftwarePaths(
        LmdFitBinaries=lmd_build_path,
        PandaRootMacroPath=PNDmacropath,
        LmdFitScripts=LMDscriptpath,
    )

    experiment = ExperimentParameters(
        experimentType=experimentType,
        cluster=ClusterEnvironment.HIMSTER,
        softwarePaths=softwarePaths,
        LMDDataCommand=LMDDataCommand,
        trackFilePattern=trackFilePattern,
        # LmdFitDataDir=lmdfit_data_dir,
        experimentDir=experimentDir,
        dataPackage=dataPackage,
        resAccPackage=resAccPackage,
        fitConfigPath=(LMDscriptpath / Path("../fitconfig-fast.json")).resolve(),
    )

    return experiment


def restrictPhiConfigs() -> None:
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
                SimulationGeneratorType.RESACCBOX,
            )

            # change simpars if misalignment matrices are given
            # we have to use evolve (deep copies) since all configs are frozen
            newMisalignPath = Path(f"/home/roklasen/LMD-Alignment/output/misMat-nothing-{phiMatNames[i]}.json")
            newAlignPars = evolve(experiment.dataPackage.alignParams, misalignment_matrices_path=newMisalignPath)
            newAlignPars = evolve(experiment.resAccPackage.alignParams, misalignment_matrices_path=newMisalignPath)
            newRecopars = evolve(experiment.dataPackage.recoParams, use_ip_determination=False)
            newRecopars = evolve(experiment.resAccPackage.recoParams, use_ip_determination=False)

            newExperiment = evolve(experiment, alignParams=newAlignPars, recoParams=newRecopars)

            write_params_to_file(
                newExperiment,
                Path(f"./{confPathPanda}/restrictPhi/"),
                f"{mom}-{phiMatNames[i]}.config",
                overwrite=True,
            )


def genConfigs() -> None:
    for mom in momenta:
        # PANDA configs

        experiment = genExperimentConfig(
            momentum=mom,
            theta_min=theta_min[0],
            theta_max=theta_max[0],
            phi_min=phi_min[0],
            phi_max=phi_max[0],
            experimentType=ExperimentType.LUMI,
            simGenTypeForResAcc=SimulationGeneratorType.RESACCBOX,
        )

        # change simpars if misalignment matrices are given
        if args.misalignMatrixFileP is not None:
            alignParams = AlignmentParameters(misalignment_matrices_path=args.misalignMatrixFileP)

            # we have to use evolve (deep copies) since all configs are frozen
            experiment = evolve(experiment, alignParams=alignParams)

        write_params_to_file(
            experiment,
            Path("."),
            f"{confPathPanda}/{mom}.config",
            overwrite=True,
        )

        # KOALA configs

        experiment = genExperimentConfig(
            momentum=mom,
            theta_min=theta_min[1],
            theta_max=theta_max[1],
            phi_min=phi_min[1],
            phi_max=phi_max[1],
            experimentType=ExperimentType.KOALA,
            simGenTypeForResAcc=SimulationGeneratorType.RESACCPBARP_ELASTIC,
        )

        # change simpars if misalignment matrices are given
        if args.misalignMatrixFileK is not None:
            alignParams = AlignmentParameters(misalignment_matrices_path=args.misalignMatrixFileK)
            experiment = evolve(experiment, alignParams=alignParams)

        write_params_to_file(experiment, Path("."), f"{confPathKoala}/{mom}.config", overwrite=True)


parser = argparse.ArgumentParser()
parser.add_argument("-b", dest="inBetweenMomenta", action=argparse._StoreTrueAction)
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

parser.add_argument(
    "-rphi", dest="restrictPhi", help="generate multiple configs with restricted phi angles, no misalignment.", action=argparse._StoreTrueAction
)

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


upperPhiAngles = (6.28318531718, 6.28318531718 / 2, 6.28318531718 / 4, 6.28318531718 / 8, 6.28318531718 / 10, 6.28318531718 / 12)
phiMatNames = ("2pi", "2piO2", "2piO4", "2piO8", "2piO10", "2piO12")

# * special case: resticted phi angles. we'll have to cheat a little here with a misalignment matrix
# that has no misalignmen, so that we get new paths for each new angle

if args.restrictPhi:
    restrictPhiConfigs()
else:
    genConfigs()
