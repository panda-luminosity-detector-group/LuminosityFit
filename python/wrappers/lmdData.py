#!/usr/bin/env python3
"""
Module to create LMD data objects via the createLumiFitData or createKoaFitData apps.
"""

from lumifit.cluster import Job, JobResourceRequest
from lumifit.general import ConfigReaderAndWriter, getGoodFiles
from lumifit.paths import (
    generateAbsoluteROOTDataPath,
    generateRelativeBinningDir,
    generateRelativeBunchesDir,
)
from lumifit.recipe import SimulationDataType, SimulationTask
from lumifit.types import DataMode, ExperimentParameters


def createLmdDataJob(experiment: ExperimentParameters, task: SimulationTask, elasticCrossSection: float) -> Job:
    jobCommand = experiment.LMDDataCommand
    inputConfigPath = experiment.dataConfigPath

    # select config package
    if task.simDataType == SimulationDataType.VERTEX:
        configPackage = experiment.dataPackage
    elif task.simDataType == SimulationDataType.ANGULAR:
        configPackage = experiment.dataPackage
    elif task.simDataType == SimulationDataType.EFFICIENCY_RESOLUTION:
        configPackage = experiment.resAccPackage
    else:
        raise NotImplementedError(f"Simulation type {task.simDataType} is not implemented!")

    # make root data path
    if task.simDataType == SimulationDataType.VERTEX:
        pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage, dataMode=DataMode.VERTEXDATA)
    else:
        pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage)

    # I think the binary has a special case for angular data
    if task.simDataType != SimulationDataType.ANGULAR:
        elasticCrossSection = 1.0

    fileListPath = pathToRootFiles / generateRelativeBunchesDir()
    binningPath = pathToRootFiles / generateRelativeBunchesDir() / generateRelativeBinningDir()

    # Why OOP?
    configIO = ConfigReaderAndWriter()
    config = configIO.loadConfig(inputConfigPath)

    general_dimension_bins_low = 300
    general_dimension_bins_high = 300
    general_dimension_bins_step = 100

    # apparently this means all?
    num_events = 0
    lab_momentum = configPackage.recoParams.lab_momentum

    dataType = task.simDataType.value

    for bins in range(
        general_dimension_bins_low,
        general_dimension_bins_high + 1,
        general_dimension_bins_step,
    ):
        if "general_data" in config:
            subconfig = config["general_data"]
            if "primary_dimension" in subconfig:
                subsubconfig = subconfig["primary_dimension"]
                if "bins" in subsubconfig:
                    subsubconfig["bins"] = bins
            if "secondary_dimension" in subconfig:
                subsubconfig = subconfig["secondary_dimension"]
                if "bins" in subsubconfig:
                    subsubconfig["bins"] = bins

        if "efficiency" in config:
            subconfig = config["efficiency"]
            if "primary_dimension" in subconfig:
                subsubconfig = subconfig["primary_dimension"]
                if "bins" in subsubconfig:
                    subsubconfig["bins"] = bins
            if "secondary_dimension" in subconfig:
                subsubconfig = subconfig["secondary_dimension"]
                if "bins" in subsubconfig:
                    subsubconfig["bins"] = bins

        # TODO: why tho? what even are the changes above for?
        print(f"saving config to {binningPath}")
        configIO.writeConfigToPath(config, binningPath / "dataconfig.json")

    fileList, _ = getGoodFiles(fileListPath, "filelist_*.txt", min_filesize_in_bytes=100)
    numFileList = len(fileList)

    if numFileList < 1:
        raise RuntimeError("No filelists found!")

    resource_request = JobResourceRequest(3 * 60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 2500

    job = Job(
        resource_request,
        jobCommand,
        "createFitData",
        str(binningPath) + "/createFitData-%a.log",
        array_indices=list(range(1, numFileList + 1)),
    )
    job.exported_user_variables["numEv"] = str(num_events)
    job.exported_user_variables["pbeam"] = f"{lab_momentum:.2f}"
    job.exported_user_variables["input_path"] = pathToRootFiles  # bunches
    job.exported_user_variables["filelist_path"] = fileListPath  # bunches
    job.exported_user_variables["output_path"] = str(binningPath)  # bunches/binning
    job.exported_user_variables["config_path"] = str(binningPath) + "/dataconfig.json"
    job.exported_user_variables["type"] = dataType
    job.exported_user_variables["elastic_cross_section"] = elasticCrossSection

    return job


if __name__ == "__main__":
    print("cannot be run as main module")
