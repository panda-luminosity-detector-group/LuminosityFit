#!/usr/bin/env python3

import argparse
import concurrent.futures
import copy
import json
import math
import os
import subprocess
from enum import Enum
from pathlib import Path
from time import sleep
from typing import List, Optional

from lumifit.cluster import ClusterJobManager
from lumifit.config import load_params_from_file, write_params_to_file
from lumifit.general import getGoodFiles
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler
from lumifit.paths import (
    generateAbsoluteMergeDataPath,
    generateAbsoluteROOTDataPath,
    generateRelativeBinningDir,
    generateRelativeBunchesDir,
    generateRelativeMergeDir,
)
from lumifit.recipe import (
    SimRecipe,
    SimulationDataType,
    SimulationState,
    SimulationTask,
)
from lumifit.types import ClusterEnvironment, DataMode, ExperimentParameters
from wrappers.createRecoJob import create_reconstruction_job
from wrappers.createSimRecoJob import create_simulation_and_reconstruction_job
from wrappers.lmdData import createLmdDataJob
from wrappers.lumiFit import createLumiFitJob

"""

Determines the luminosity of a set of Lumi_TrksQA_* files.

    This is pretty complicated and uses a lot of intermediate steps.
    Many of them have to be run on a cluster system, which is done
    with a slurm job handler.

This file needs one major rewrite, thats for sure.
- the logic is implemented as a GIANT state machine
"""


class StatusCode(Enum):
    ENOUGH_FILES = 0
    NO_FILES = 2


# TODO: the percentage check is shite because we don't know how many files there are in total
# all we know is how MANY are good
def enoughFilesPresent(
    directory: Path,
    glob_pattern: str,
    min_filesize_in_bytes: int = 10000,
) -> StatusCode:
    """
    Returns StatusCode.ENOUGH_FILES if at least 80% of the files in the directory
    match the glob pattern and are larger than min_filesize_in_bytes.

    Returns StatusCode.NO_FILES otherwise
    """
    print(f"checking if job was successful. checking in path {directory}, glob pattern {glob_pattern}")
    required_files_percentage = 0.8

    _, files_percentage = getGoodFiles(
        directory,
        glob_pattern,
        min_filesize_in_bytes=min_filesize_in_bytes,
    )

    print(f"files percentage (depends on getGoodFiles) is {files_percentage}")

    if files_percentage >= required_files_percentage:
        return StatusCode.ENOUGH_FILES

    return StatusCode.NO_FILES


def executeTask(experiment: ExperimentParameters, task: SimulationTask) -> Optional[int]:
    """
    A task is always three steps:

    - simulate and/or reconstruct data
    - make file list bunches (that means make some lists of files that are merged later)
    - merge data (that means merge the bunches into one file)

    If some of this data already exists, the intermediate step is skipped.

    That's it already, HOWEVER:
    Since there are three different types of data (vertex, angular, res/acc)
    and each of these can be in a different state, there are a lot of if branches
    here.

    Worse, a task will have a state its in. When it finishes a step, this internal
    state is incremented. That means a task must be handled multiple times UNTIL
    it's in the done-state.

    Returns: the job ID if a job was submitted or
    None if an intermediate step was executed locally (i.e. not on a cluster).
    """
    print(f"executeTask: {experiment.experimentDir}")
    print(f"running simulation of type {str(task.simDataType)} at states:")
    print(f"current state: {str(task.simState)}")

    # data pattern is used to check if previous simulation steps were successfully run
    # the config patten switch is essential to generate the correct paths
    data_pattern = ""
    if task.simDataType == SimulationDataType.VERTEX:
        data_pattern = "lmd_vertex_data_"
        configPackage = experiment.dataPackage
    elif task.simDataType == SimulationDataType.ANGULAR:
        configPackage = experiment.dataPackage
        data_pattern = "lmd_data_"
    elif task.simDataType == SimulationDataType.EFFICIENCY_RESOLUTION:
        configPackage = experiment.resAccPackage
        data_pattern = "lmd_res_data_"
    else:
        raise NotImplementedError(f"Simulation type {task.simDataType} is not implemented!")

    # 1. simulate data
    if task.simState == SimulationState.START_SIM:
        if task.simDataType == SimulationDataType.EFFICIENCY_RESOLUTION:
            """
            efficiency / resolution calculation.

            Takes the offset of the IP into account.

            TODO: This needs to know the misalignment of the detector for the sim steps
            and the alignment for the reconstruction.
            Since real data has unknown misalignment, we must use the inverse alignment matrices
            to model the acceptance of the real detector faithfully.
            """

            assert experiment.resAccPackage.simParams is not None

            # TODO: later, WITH alignment since that affects the acceptance!
            resAccDataDir = generateAbsoluteROOTDataPath(configPackage=configPackage)
            status_code = enoughFilesPresent(directory=resAccDataDir, glob_pattern=experiment.trackFilePattern + "*.root")

            if status_code == StatusCode.ENOUGH_FILES:
                task.simState = SimulationState.MAKE_BUNCHES
                return None

            elif status_code == StatusCode.NO_FILES:
                # then lets simulate!
                # this command runs the full sim software with box gen data
                # (or dpm gen data for KOALA)
                # to generate the acceptance and resolution information
                # for this sample
                # note: beam tilt and divergence are not necessary here,
                # because that is handled completely by the model

                # TODO: alignment part
                # if alignement matrices were specified, we used them as a mis-alignment
                # and alignment for the box simulations

                job = create_simulation_and_reconstruction_job(
                    experiment,
                    thisMode=DataMode.RESACC,
                    use_devel_queue=args.use_devel_queue,
                )
                returnJobID = job_manager.enqueue(job)
                return int(returnJobID)
            else:
                raise RuntimeError(f"Status code {status_code} is unexpected!")

        elif task.simDataType == SimulationDataType.ANGULAR:
            """
            a is the angular case. this is the data set onto which the luminosity fit is performed.
            it is therefore REAL digi data (or DPM data of course) that must be reconstructed again
            with the updated reco parameter (like the IP position, cuts applied and alignment).
            note: beam tilt and divergence are not used here because
            only the last reco steps are rerun of the track reco
            """

            angularDataDir = generateAbsoluteROOTDataPath(experiment.dataPackage)
            status_code = enoughFilesPresent(
                directory=angularDataDir,
                glob_pattern=experiment.trackFilePattern + "*.root",
            )
            if status_code == StatusCode.ENOUGH_FILES:
                task.simState = SimulationState.MAKE_BUNCHES
                return None

            elif status_code == StatusCode.NO_FILES:
                # TODO: alignment part

                job = create_reconstruction_job(
                    experiment,
                    thisMode=DataMode.DATA,
                    use_devel_queue=args.use_devel_queue,
                )
                returnJobID = job_manager.enqueue(job)
                return int(returnJobID)
            else:
                raise RuntimeError(f"Status code {status_code} is unexpected!")

        elif task.simDataType == SimulationDataType.VERTEX:
            assert experiment.dataPackage.MCDataDir is not None
            mcDataDir = experiment.dataPackage.MCDataDir

            status_code = enoughFilesPresent(directory=mcDataDir, glob_pattern="Lumi_MC_*.root")

            if status_code == StatusCode.ENOUGH_FILES:
                task.simState = SimulationState.MAKE_BUNCHES
                return None

            elif status_code == StatusCode.NO_FILES:
                # vertex data must always be created without any cuts first
                copyExperiment = copy.deepcopy(experiment)
                copyExperiment.dataPackage.recoParams.disableCuts()

                # TODO: misalignment is important here. the vertex data can have misalignment (because it's real data)
                # but it has no alignment yet. that is only for the second reconstruction

                job = create_simulation_and_reconstruction_job(
                    copyExperiment,
                    thisMode=DataMode.VERTEXDATA,
                    use_devel_queue=args.use_devel_queue,
                )
                del copyExperiment
                returnJobID = job_manager.enqueue(job)
                return int(returnJobID)
            else:
                raise RuntimeError(f"Status code {status_code} is unexpected!")

        else:
            raise ValueError(f"This tasks simType is {task.simDataType}, which is invalid!")

    # 2. create data (that means bunch data, create data objects)
    if task.simState == SimulationState.MAKE_BUNCHES:
        """
        Okay, quick recap what's happening here:

        makeMultipleFileListBunches is ALWAYS called, but the path depends on if
        we're in angular, vertex or resAcc mode.

        createMultipleLmdData is ALSO ALWAYS called, for the path same as above,
        AND elastic_cross_section is passed IF we're in ANGULAR mode

        lastly the status_code check and the branch exits.
        Fuck I want to rewrite this logic shit so badly, but the path shit is more
        important shit right now, so I'll just leave this shit comment to do this
        shit later.
        """

        # okay, we are either in DATA mode or RESACC mode, so choose the
        # correct configPackage. also, in VERTEX mode, the reco params don't
        # have cuts!
        # yes, I hate this too, this will all be scrapped
        # once these scripts are proper modules
        # TODO: kick this case switch!
        if task.simDataType == SimulationDataType.VERTEX:
            pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage, dataMode=DataMode.VERTEXDATA)
        else:
            pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage)

        binningPath = pathToRootFiles / generateRelativeBunchesDir() / generateRelativeBinningDir()

        # TODO: this check shouldn't be here but in the makeMultipleFileListBunches or createMultipleLmdData modules. This script should only execute the steps in the right order. If  anything is already there, the submodules can just exit early.
        status_code = enoughFilesPresent(
            directory=binningPath,
            glob_pattern=data_pattern + "*",
        )

        if status_code == StatusCode.ENOUGH_FILES:
            print("skipping bunching and data object creation...")
            task.simState = SimulationState.MERGE
            return None

        elif status_code == StatusCode.NO_FILES:
            # create bunch files
            os.chdir(experiment.softwarePaths.LmdFitScripts)
            multiFileListCommand: List[str] = []
            multiFileListCommand.append("python")
            multiFileListCommand.append("makeMultipleFileListBunches.py")
            multiFileListCommand.append("--filenamePrefix")
            multiFileListCommand.append(f"{experiment.trackFilePattern}")
            multiFileListCommand.append("--files_per_bunch")
            multiFileListCommand.append("10")
            multiFileListCommand.append("--maximum_number_of_files")

            multiFileListCommand.append(str(configPackage.recoParams.num_samples))
            multiFileListCommand.append(str(pathToRootFiles))

            print(f"Bash command for file list bunch creation:\n{' '.join(multiFileListCommand)}\n")
            _ = subprocess.call(multiFileListCommand)

            multiFileListCommand.clear()

            # we need the elastic cross section for the angular data
            csFile = experiment.experimentDir / "elastic_cross_section.txt"
            if not csFile.exists():
                raise FileNotFoundError("ERROR! Can not find elastic cross section file! The determined Luminosity will be wrong!\n")
            with open(csFile, "r") as f:
                content = f.readlines()
                el_cs = float(content[0])

            # create LMD data objects
            LMDdataJob = createLmdDataJob(experiment, task, el_cs)

            # was apparently bunches
            returnJobID = job_manager.enqueue(LMDdataJob)
            return int(returnJobID)
        else:
            raise RuntimeError(f"Status code {status_code} is unexpected!")

    # 3. merge data
    if task.simState == SimulationState.MERGE:
        # TODO: kick this case switch!
        if task.simDataType == SimulationDataType.VERTEX:
            pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage, dataMode=DataMode.VERTEXDATA)
        else:
            pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage)

        mergePath = pathToRootFiles / generateRelativeBunchesDir() / generateRelativeBinningDir() / generateRelativeMergeDir()

        status_code = enoughFilesPresent(
            directory=mergePath,
            glob_pattern=data_pattern + "*",
        )

        if status_code == StatusCode.ENOUGH_FILES:
            print("skipping data merging...")
            task.simState = SimulationState.DONE
            return None

        elif status_code == StatusCode.NO_FILES:
            os.chdir(experiment.softwarePaths.LmdFitScripts)
            mergeCommand: List[str] = []
            mergeCommand.append("python")
            mergeCommand.append("mergeMultipleLmdData.py")
            mergeCommand.append(str(task.simDataType.value))  # we have to give the value because the script expects a/er/v !
            mergeCommand.append(str(pathToRootFiles))

            # TODO: do we still need this?
            # if task.simDataType == SimulationDataType.ANGULAR:
            #     mergeCommand.append("--num_samples")
            #     mergeCommand.append(str(1))

            print(f"running command:\n{mergeCommand}")
            _ = subprocess.call(mergeCommand)

            task.simState = SimulationState.DONE
            return None
        else:
            raise RuntimeError(f"Status code {status_code} is unexpected!")

    else:
        raise NotImplementedError(f"Simulation state {task.simState} is not implemented!")


def singleTaskWorker(experiment: ExperimentParameters, task: SimulationTask) -> bool:
    """
    The recipe will have one to many tasks. "Handling" a task means either running
    some script to process data, or submit a job to the cluster to generate or process
    data.
    Either way, the handleTask() function either returns None, which means the task is
    immediately ready for the next step (that means call handleTask() again)
    or it returns a jobID, which means the task is submitted to the cluster and we have to wait for it.

    A task has a state, which is changed by handleTask(). When it task is finished,
    its state will be DONE.

    This function will be called in a loop until all tasks are done. It must be wrapped
    in a thread if multiple tasks are to be handled at the same time.

    Returns True once the task is finished successfully (so we can catch exceptions).
    """
    iteration = 0
    while True:
        iteration += 1
        print(f"Task: {task}, iteration {iteration}")
        if task.simState == SimulationState.DONE:
            print(f"task {task} is done, returning...")
            return True
        jobID = executeTask(experiment=experiment, task=task)

        # no job was submitted, we can process the task again
        if jobID is None:
            continue

        # a job was submitted, we have to wait for it
        else:
            print(f"Waiting for job ID {jobID}, task {task}...")
            while True:
                runningJobs = job_manager.get_active_number_of_jobs(jobID)
                if runningJobs == 0:
                    break
                else:
                    sleep(60)


def processSimulationTasks(experiment: ExperimentParameters, recipe: SimRecipe) -> None:
    """
    The recipe will contain one to many SimulationTasks.
    Use a thread pool to process all tasks at the same time,
    use the singleTaskWorker() method for that. It will return True
    once the task is finished.

    If the debug flag is set, all tasks are processed sequentially
    """

    if args.debug:
        print("DEBUG: running task sequentially without thread pool.")
        for task in recipe.SimulationTasks:
            singleTaskWorker(experiment, task)
    else:
        print(f"Submitting {len(recipe.SimulationTasks)} tasks to thread pool...")

        with concurrent.futures.ThreadPoolExecutor() as taskExecutor:
            futures = [taskExecutor.submit(singleTaskWorker, experiment, task) for task in recipe.SimulationTasks]

            # wait for all tasks to finish
            print("submitted, waiting...")
            taskExecutor.shutdown(wait=True)

        print("All threads done!")

        # check the results of the futures to see if a task raised an exception
        # it will be raised here
        for future in futures:
            if future.result() is False:
                raise RuntimeError("ERROR! A task crashed!")

    # check if all tasks are finished. ALL should be finished here!
    recipe.SimulationTasks = [simTask for simTask in recipe.SimulationTasks if simTask.simState != SimulationState.DONE]

    if len(recipe.SimulationTasks) > 0:
        raise RuntimeError(
            f"ERROR! There are still {len(recipe.SimulationTasks)} tasks left to process, but all should be done or still processing. That means one of the threads crashed unexpectedly."
        )


def lumiDetermination(thisExperiment: ExperimentParameters, recipe: SimRecipe) -> bool:
    print(f"processing recipe {thisExperiment.experimentDir}")

    """
    *
    State 1 simulates vertex data from which the IP can be determined.
    
    by default, we always start with SIMULATE_VERTEX_DATA, even though
    runSimulationReconstruction has probably already been run.
    """
    recipe.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.VERTEX, simState=SimulationState.START_SIM))
    processSimulationTasks(thisExperiment, recipe)

    """
    *
    State 2 only handles the reconstructed IP. If use_ip_determination is set,
    the IP is reconstructed from the uncut data set and written to a reco_ip.json
    file. The IP position will only be used from this file from now on.

    If use_ip_determination is false, the reconstruction will use the IP as
    specified from the reco params of the experiment config.

    ! Therefore, this is the ONLY place where a new IP may be set.
    """

    if thisExperiment.dataPackage.recoParams.use_ip_determination:
        assert thisExperiment.resAccPackage.simParams is not None, "ERROR! simParams are not set in config!"
        assert thisExperiment.recoIPpath is not None, "ERROR! path to recoIP.json is not set in config!"

        vertexDataMergePath = generateAbsoluteMergeDataPath(
            thisExperiment.dataPackage,
            dataMode=DataMode.VERTEXDATA,
        )

        if not thisExperiment.recoIPpath.exists():
            # 2. determine offset on the vertex data sample
            bashCommand: List[str] = []
            bashCommand.append(str(thisExperiment.softwarePaths.LmdFitBuildDir / "bin/determineBeamOffset"))
            bashCommand.append("-p")
            bashCommand.append(str(vertexDataMergePath))
            bashCommand.append("-c")
            bashCommand.append(str(thisExperiment.vertexConfigPath))
            bashCommand.append("-o")
            bashCommand.append(str(thisExperiment.recoIPpath))

            print(f"beam offset determination command:\n{bashCommand}")
            _ = subprocess.call(bashCommand)

        else:
            print("IP determination file already exists, skipping IP determination!")

        with open(str(thisExperiment.recoIPpath), "r") as f:
            ip_rec_data = json.load(f)

        # check if ip_x and ip_y are in the reco_ip.json file
        # if not, the IP was not determined correctly
        if "ip_x" not in ip_rec_data or "ip_y" not in ip_rec_data:
            raise RuntimeError(f"ERROR! Reco IP file {thisExperiment.recoIPpath} does not contain ip_x or ip_y!")

        newRecoIPX = float("{0:.3f}".format(round(float(ip_rec_data["ip_x"]), 3)))  # in cm
        newRecoIPY = float("{0:.3f}".format(round(float(ip_rec_data["ip_y"]), 3)))
        newRecoIPZ = float("{0:.3f}".format(round(float(ip_rec_data["ip_z"]), 3)))

        # I don't like this. Actually I hate this.
        # The experiment config is frozen for a reason.
        # But unfortunately there doesn't seem to be a better way

        # so, three param sets must be updated with the new IP:
        # - the dataPackage recoParams
        # - the resAccPackage simParams (both IP and theta min/max)
        # - the resAccPackage recoParams
        #
        # at least we know it can happen ONLY here
        # remember, this config is never written to disk

        max_xy_shift = math.sqrt(newRecoIPX**2 + newRecoIPY**2)
        max_xy_shift = float("{0:.2f}".format(round(float(max_xy_shift), 2)))

        resAccThetaMin = thisExperiment.resAccPackage.simParams.theta_min_in_mrad - max_xy_shift
        resAccThetaMax = thisExperiment.resAccPackage.simParams.theta_max_in_mrad + max_xy_shift

        thisExperiment.dataPackage.recoParams.setNewIPPosition(newRecoIPX, newRecoIPY, newRecoIPZ)
        thisExperiment.resAccPackage.simParams.setNewIPPosition(newRecoIPX, newRecoIPY, newRecoIPZ)
        thisExperiment.resAccPackage.simParams.setNewThetaAngles(resAccThetaMin, resAccThetaMax)
        thisExperiment.resAccPackage.recoParams.setNewIPPosition(newRecoIPX, newRecoIPY, newRecoIPZ)

        print("============================================================")
        print("                       Attention!                           ")
        print("     Experiment Config has been changed in memory!          ")
        print("This MUST only happen if you are using the IP determination!")
        print("")
        print("New IP position:")
        print(f"  x: {thisExperiment.dataPackage.recoParams.recoIPX} cm")
        print(f"  y: {thisExperiment.dataPackage.recoParams.recoIPY} cm")
        print(f"  z: {thisExperiment.dataPackage.recoParams.recoIPZ} cm")
        print("")
        print("New theta angles:")
        print(f"  theta min: {thisExperiment.resAccPackage.simParams.theta_min_in_mrad} mrad")
        print(f"  theta max: {thisExperiment.resAccPackage.simParams.theta_max_in_mrad} mrad")
        print("")
        print("============================================================")
        print("Finished IP determination for this recipe!")

        # overwrite existing config file
        print(f"Overwriting experiment config file at {thisExperiment.experimentDir}/experiment.config with new IP position...")
        write_params_to_file(thisExperiment, thisExperiment.experimentDir, "experiment.config", overwrite=True)
    else:
        print("Skipped IP determination for this recipe, using values from config.")

    """
    *
    State 3
    the IP position is now reconstructed. filter the DPM data again,
    this time using the newly determined IP position as a cut criterion.
    (that also means create fileLists -> bunches/binning -> merge)
    at the same time:
    generate acceptance and resolution with these reconstructed ip values
    (that means simulation + bunching + creating data objects + merging)
    """

    recipe.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.EFFICIENCY_RESOLUTION, simState=SimulationState.START_SIM))
    recipe.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.ANGULAR, simState=SimulationState.START_SIM))

    # remember, the experiment config may now contain the updated IP
    # there are now two simulation tasks running, but this call only returns once BOTH are done
    processSimulationTasks(thisExperiment, recipe)

    """
    *
    State 4: Luminosity Fit
    """
    lumiFitJob = createLumiFitJob(thisExperiment)

    # TODO: ayo we get the job ID, let's wait for it to finish?
    job_manager.enqueue(job=lumiFitJob)

    print("this recipe is fully processed!!!")

    return True


def experimentWorker(experiment: ExperimentParameters) -> None:
    experiment.experimentDir.mkdir(parents=True, exist_ok=True)

    # before anything else, check if there is a copy in the experiment dir. if not, make it.
    if not (experiment.experimentDir / "experiment.config").exists():
        print("No copy of the experiment config found in the experiment directory. Copying it there now.")
        write_params_to_file(experiment, experiment.experimentDir, "experiment.config")

    # create a recipe object. it resides only in memory and is never written to disk
    # it is however liberally modified and passed around
    recipe = SimRecipe()

    lumiDetermination(experiment, recipe)


#! --------------------------------------------------
#!             script part starts here
#! --------------------------------------------------

parser = argparse.ArgumentParser(description="Lmd One Button Script")

parser.add_argument(
    "--bootstrapped_num_samples",
    type=int,
    default=1,
    help="number of elastic data samples to create via bootstrapping (for statistical analysis)",
)

configGroup = parser.add_mutually_exclusive_group(required=True)

configGroup.add_argument(
    "-e",
    "--experiment_config",
    dest="ExperimentConfigFile",
    type=Path,
    help="The Experiment.config file that holds all info.",
    default=None,
)

configGroup.add_argument(
    "-E",
    "--experiment_dir",
    dest="ExperimentDir",
    type=Path,
    help="Process all configs in this directory. If not set, at least one config must be specified with -e.",
    default=None,
)

parser.add_argument(
    "-d",
    "--debug",
    dest="debug",
    action="store_true",
    help="In debug mode, all concurrency is disabled an all task are executed sequentially.",
)

parser.add_argument(
    "--use_devel_queue",
    action="store_true",
    help="If flag is set, the devel queue is used",
)

args = parser.parse_args()

experiments: List[ExperimentParameters] = []

# either we have one experiment config file or a directory with many
if args.ExperimentDir is not None:
    if args.ExperimentDir.is_file():
        raise ValueError("ERROR! ExperimentDir must be a directory!")

    # process all experiment configs in the given dir
    for experimentConfig in args.ExperimentDir.glob("*.config"):
        experiment: ExperimentParameters = load_params_from_file(experimentConfig, ExperimentParameters)
        experiments.append(experiment)

        # this is a hack to make the code work with the old config files
elif args.ExperimentConfigFile is not None:
    if args.ExperimentConfigFile.is_dir():
        raise ValueError("ERROR! ExperimentConfigFile must be a file!")

    # load experiment config
    experiment = load_params_from_file(args.ExperimentConfigFile, ExperimentParameters)
    experiments.append(experiment)

# ask confirmation
print("The following experiments will be processed:")
for experiment in experiments:
    print(f"  {experiment.experimentDir} ({experiment.dataPackage.recoParams.lab_momentum} GeV)")
print("")
print("Continue? [y/n]")
user_input = input()
if user_input != "y":
    print("Aborting!")
    exit(0)

# check which cluster we're on and create job handler
# we know there is at least one config, and we just assume all use the same cluster
if experiments[0].cluster == ClusterEnvironment.VIRGO:
    job_handler = create_virgo_job_handler("long")
elif experiments[0].cluster == ClusterEnvironment.HIMSTER:
    if args.use_devel_queue:
        job_handler = create_himster_job_handler("devel")
    else:
        job_handler = create_himster_job_handler("himster2_exp")
else:
    raise NotImplementedError("This cluster type is not implemented!")

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = ClusterJobManager(job_handler, 2000, 3600)

if args.debug:
    print("DEBUG: running experiment sequentially without thread pool.")
    for experiment in experiments:
        experimentWorker(experiment)
else:
    with concurrent.futures.ThreadPoolExecutor() as recipeExecutor:
        futures = [recipeExecutor.submit(experimentWorker, experiment) for experiment in experiments]

        # wait for all tasks to finish
        print("Enqueued all experiments, waiting...")
        recipeExecutor.shutdown(wait=True)

        for future in futures:
            if future.result() is False:
                raise RuntimeError("ERROR! A recipe crashed!")

print("all done!")
