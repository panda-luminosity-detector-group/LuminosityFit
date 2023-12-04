#!/usr/bin/env python3

import random
import string
from pathlib import Path

from lumifit.config import write_params_to_file
from lumifit.general import envPath
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


def genOffsetIPconfigs() -> None:
    destPath = Path("./expConfigs/PANDA/IPoffsetFromConfig")
    destPath.mkdir(parents=True, exist_ok=True)

    for momentum in [1.5, 15]:
        # offsets are in mm here, remember root units are cm!
        for offset in [-1.0, -0.5, -0.25, 0, 0.25, 0.5, 1.0]:
            offset = offset / 10.0

            experimentType = ExperimentType.LUMI

            lmdScriptPath = envPath("LMDFIT_SCRIPTPATH")
            lmdfit_data_dir = envPath("LMDFIT_DATA_DIR")
            lmd_build_path = envPath("LMDFIT_BUILD_PATH")
            PNDmacropath = envPath("LMDFIT_MACROPATH")
            LMDscriptpath = envPath("LMDFIT_SCRIPTPATH")

            simulationCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdSimReco.py"
            reconstructionCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/runLmdReco.py"
            LMDDataCommand = f"{lmdScriptPath}/singularityJob.sh {lmdScriptPath}/createLumiFitData.sh"
            trackFilePattern = "Lumi_TrksQA_"

            experimentDir = lmdfit_data_dir / Path(f"LMD-{momentum:.2f}-IPoffset:{offset}-" + "".join(random.choices(string.ascii_letters, k=4)))

            simParamsData = SimulationParameters(
                simulationCommand=simulationCommand,
                lab_momentum=momentum,
                random_seed=random.randint(11, 9999),
                ip_offset_x=offset,
                ip_offset_y=offset,
            )
            recoParamsData = ReconstructionParameters(
                reconstructionCommand=reconstructionCommand,
                lab_momentum=momentum,
                recoIPX=offset,
                recoIPY=offset,
                use_ip_determination=False
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
                random_seed=random.randint(11, 9999),
                simGeneratorType=SimulationGeneratorType.BOX,
            )

            recoParamsResAcc = ReconstructionParameters(
                reconstructionCommand=reconstructionCommand,
                lab_momentum=momentum,
                num_samples=500,
                num_events_per_sample=100000,
                recoIPX=offset,
                recoIPY=offset,
                use_ip_determination=False
            )

            resAccPackage = ConfigPackage(
                simParams=simParamsResAcc,
                recoParams=recoParamsResAcc,
                alignParams=AlignmentParameters(),
                baseDataDir=lmdfit_data_dir / experimentDir / "resacc",
                MCDataDir=lmdfit_data_dir / experimentDir / "resacc/mc_data",
            )

            softwarePaths = SoftwarePaths(
                LmdFitBuildDir=lmd_build_path,
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
                fitConfigPath=(LMDscriptpath / Path("../configs/fitconfig-fast.json")).resolve(),
                dataConfigPath=(LMDscriptpath / Path("../configs/dataconfig_xy.json")).resolve(),
                vertexConfigPath=(LMDscriptpath / Path("../configs/vertex_fitconfig.json")).resolve(),
                recoIPpath=(experimentDir / Path("recoIP.json")).resolve(),
                lumiFileName=(experimentDir / Path("lumi-values.json")).resolve(),
            )

            write_params_to_file(
                experiment,
                Path("."),
                f"{destPath}/{momentum}:{offset}.config",
                overwrite=True,
            )


if __name__ == "__main__":
    genOffsetIPconfigs()
