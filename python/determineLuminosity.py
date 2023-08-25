#!/usr/bin/env python3

import argparse
import copy
import json
import math
import os
import subprocess
import sys
import time
from enum import Enum
from pathlib import Path
from typing import List

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
    LumiDeterminationState,
    SimRecipe,
    SimulationDataType,
    SimulationState,
    SimulationTask,
)
from lumifit.types import ClusterEnvironment, DataMode, ExperimentParameters
from wrappers.createRecoJob import create_reconstruction_job
from wrappers.createSimRecoJob import create_simulation_and_reconstruction_job

"""

Determines the luminosity of a set of Lumi_TrksQA_* files.

    This is pretty complicated and uses a lot of intermediate steps.
    Many of them have to be run on a cluster system, which is done
    with a slurm job handler.

This file needs one major rewrite, thats for sure.
- the logic is implemented as a GIANT state machine
- because job submission doesn't block
- job supervision is based on number of files;
- AND number of running jobs, even if these jobs belong to some other function
- but its not monitored if a given job has crashed
- and the entire thing is declarative, when object orientation would be better suited

For this rewrite, a better job supervision is needed. It would suffice already to
accept the jobID (or job Array ID) and simply poll once a minute if the jobs for that ID 
are still running.

If not, proceed.
If the needed files aren't there (but the jobs don't run anymore either), there is a runtime error.

"""


class StatusCode(Enum):
    ENOUGH_FILES = 0
    STILL_RUNNING = 1
    NO_FILES = 2
    FAILED = 3


# TODO: add jobID or jobArrayID check here
# TODO: the percentage check is also shite because files can be done after this check is true.
def wasSimulationSuccessful(
    directory: Path,
    glob_pattern: str,
    min_filesize_in_bytes: int = 10000,
) -> StatusCode:
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

    if files_percentage < required_files_percentage:
        if job_handler.get_active_number_of_jobs() > 0:
            print(f"there are still {job_handler.get_active_number_of_jobs()} jobs running")
            return StatusCode.STILL_RUNNING
        else:
            print(
                "WARNING: "
                + str((1 - files_percentage) * 100)
                + "% of input files ("
                + str(glob_pattern)
                + ") missing and no jobs on himster remaining..."
                + " Something went wrong here..."
            )
            return StatusCode.NO_FILES

    return StatusCode.FAILED


# ----------------------------------------------------------------------------
# ok we do it in such a way we have a step state for each directory and stacks
# we try to process
active_recipe_stack: List[SimRecipe] = []
waiting_recipe_stack: List[SimRecipe] = []


def simulateDataOnHimster(thisExperiment: ExperimentParameters, recipe: SimRecipe) -> SimRecipe:
    """
    Start a SLURM job for every SimulationTask assigned to a recipe.
    A recipe may hold multiple SimulationTasks at the same time that
    can be run concurrently (like reconstruction of DPM data with cut
    while at the same time res/acc data is generated)

    The simStates determine what step is currently being run:

    1:  Simulate Data (so box/dpm)
        Runs RunLmdSim and RunLmdReco to get MC Data and reconstructed
        tracks. This is the same as what the runSimulationReconstruction
        script does.

    2:  create bunch data objects
        first thing it should do is create bunches/binning dirs.
        Then call createMultipleLmdData which looks for these dirs.
        This also finds the reconstructed IP position from the TrksQA files.

    3:  Merge bunched Data

    Because the job submission via Slurm doesn't block, the function is exited
    even though jobs are still running. The recipe objects therefore hold this state
    info so that the waiting method knows where it left off.

    TODO: this function is a bloody fucking mess. It gets passed a recipe object
    and depending on the internal state of this object, over nine fucking thousand
    different if branches are executed. Of course that means this shit is impossible
    to understand or debug.

    So this must change.


    TODO: AAAAAH SHIT there are multiple tasks here, we cannot exit early!
    remove the "return recipe" statements in the if clauses!

    also, since we're looping over multiple tasks, we cannot simply block after a enqueue(job) call...

    okay we can solve this by calling
    for taks in tasks:
        handleTask(task)

    and the handleTask function returns a job array ID (since a task is always at most one submit call)

    then this function can store the ids in a list and periodically check if the jobs are done, and
    ONLY THEN return
    """

    for task in recipe.SimulationTasks:
        print(f"simulateDataOnHimster: {thisExperiment.experimentDir}")
        print(f"running simulation of type {str(task.simDataType)} at states:")
        print(f"current state: {str(task.simState)}")
        print(f"last state:    {str(task.lastState)}")

        # data pattern is used to check if previous simulation steps were successfully run
        # the config patten switch is essential to generate the correct paths
        data_pattern = ""
        if task.simDataType == SimulationDataType.VERTEX:
            data_pattern = "lmd_vertex_data_"
            configPackage = thisExperiment.dataPackage
        elif task.simDataType == SimulationDataType.ANGULAR:
            configPackage = thisExperiment.dataPackage
            data_pattern = "lmd_data_"
        elif task.simDataType == SimulationDataType.EFFICIENCY_RESOLUTION:
            configPackage = thisExperiment.resAccPackage
            data_pattern = "lmd_res_data_"
        else:
            raise NotImplementedError(f"Simulation type {task.simDataType} is not implemented!")

        # 1. simulate data
        if task.simState == SimulationState.START_SIM:
            os.chdir(lmd_fit_script_path)
            if task.simDataType == SimulationDataType.EFFICIENCY_RESOLUTION:
                """
                efficiency / resolution calculation.

                Takes the offset of the IP into account.

                TODO: This needs to know the misalignment of the detector for the sim steps
                and the alignment for the reconstruction.
                Since real data has unknown misalignment, we must use the inverse alignment matrices
                to model the acceptance of the real detector faithfully.
                """

                assert thisExperiment.resAccPackage.simParams is not None

                # TODO: later, WITH alignment since that affects the acceptance!
                resAccDataDir = generateAbsoluteROOTDataPath(configPackage=configPackage)
                status_code = wasSimulationSuccessful(directory=resAccDataDir, glob_pattern=thisExperiment.trackFilePattern + "*.root")

                if status_code == StatusCode.ENOUGH_FILES:
                    task.lastState = SimulationState.START_SIM
                    task.simState = SimulationState.MAKE_BUNCHES

                elif status_code == StatusCode.STILL_RUNNING:
                    return recipe

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
                        thisExperiment,
                        thisMode=DataMode.RESACC,
                        use_devel_queue=args.use_devel_queue,
                    )
                    job_manager.enqueue(job)

                    return recipe

            elif task.simDataType == SimulationDataType.ANGULAR:
                """
                a is the angular case. this is the data set onto which the luminosity fit is performed.
                it is therefore REAL digi data (or DPM data of course) that must be reconstructed again
                with the updated reco parameter (like the IP position, cuts applied and alignment).
                note: beam tilt and divergence are not used here because
                only the last reco steps are rerun of the track reco
                """

                angularDataDir = generateAbsoluteROOTDataPath(thisExperiment.dataPackage)
                status_code = wasSimulationSuccessful(
                    directory=angularDataDir,
                    glob_pattern=thisExperiment.trackFilePattern + "*.root",
                )
                if status_code == StatusCode.ENOUGH_FILES:
                    task.lastState = SimulationState.START_SIM
                    task.simState = SimulationState.MAKE_BUNCHES

                elif status_code == StatusCode.STILL_RUNNING:
                    return recipe

                elif status_code == StatusCode.NO_FILES:
                    # TODO: alignment part

                    job = create_reconstruction_job(
                        thisExperiment,
                        thisMode=DataMode.DATA,
                        use_devel_queue=args.use_devel_queue,
                    )
                    job_manager.enqueue(job)
                    return recipe

            elif task.simDataType == SimulationDataType.VERTEX:
                assert thisExperiment.dataPackage.MCDataDir is not None
                mcDataDir = thisExperiment.dataPackage.MCDataDir

                status_code = wasSimulationSuccessful(directory=mcDataDir, glob_pattern="Lumi_MC_*.root")

                if status_code == StatusCode.ENOUGH_FILES:
                    task.lastState = SimulationState.START_SIM
                    task.simState = SimulationState.MAKE_BUNCHES

                elif status_code == StatusCode.STILL_RUNNING:
                    return recipe

                if status_code == StatusCode.NO_FILES:
                    # vertex data must always be created without any cuts first
                    copyExperiment = copy.deepcopy(thisExperiment)
                    copyExperiment.dataPackage.recoParams.disableCuts()

                    # TODO: misalignment is important here. the vertex data can have misalignment (because it's real data)
                    # but it has no alignment yet. that is only for the second reconstruction

                    job = create_simulation_and_reconstruction_job(
                        copyExperiment,
                        thisMode=DataMode.VERTEXDATA,
                        use_devel_queue=args.use_devel_queue,
                    )
                    job_manager.enqueue(job)

                    del copyExperiment
                    return recipe

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
            status_code = wasSimulationSuccessful(
                directory=binningPath,
                glob_pattern=data_pattern + "*",
            )

            if status_code == StatusCode.ENOUGH_FILES:
                print("skipping bunching and data object creation...")
                task.lastState = SimulationState.MAKE_BUNCHES
                task.simState = SimulationState.MERGE

            elif status_code == StatusCode.STILL_RUNNING:
                return recipe

            elif status_code == StatusCode.NO_FILES:
                os.chdir(lmd_fit_script_path)
                multiFileListCommand: List[str] = []
                multiFileListCommand.append("python")
                multiFileListCommand.append("makeMultipleFileListBunches.py")
                multiFileListCommand.append("--filenamePrefix")
                multiFileListCommand.append(f"{thisExperiment.trackFilePattern}")
                multiFileListCommand.append("--files_per_bunch")
                multiFileListCommand.append("10")
                multiFileListCommand.append("--maximum_number_of_files")

                multiFileListCommand.append(str(configPackage.recoParams.num_samples))
                multiFileListCommand.append(str(pathToRootFiles))

                print(f"Bash command for file list bunch creation:\n{' '.join(multiFileListCommand)}\n")
                _ = subprocess.call(multiFileListCommand)

                multiFileListCommand.clear()

                lmdDataCommand: List[str] = []
                lmdDataCommand.append("python")
                lmdDataCommand.append("createMultipleLmdData.py")
                lmdDataCommand.append("--jobCommand")
                lmdDataCommand.append(thisExperiment.LMDDataCommand)
                lmdDataCommand.append(f"{thisExperiment.dataPackage.recoParams.lab_momentum:.2f}")
                lmdDataCommand.append(str(task.simDataType.value))  # we have to give the value because the script expects a/er/v !
                lmdDataCommand.append(str(pathToRootFiles))
                lmdDataCommand.append(str(thisExperiment.dataConfigPath))

                if task.simDataType == SimulationDataType.ANGULAR:
                    # TODO: can we write this to the experiment config? I mean it only depends on the momentum anyway,
                    # so it could be written at generation time
                    el_cs = recipe.elastic_pbarp_integrated_cross_secion_in_mb
                    lmdDataCommand.append("--elastic_cross_section")
                    lmdDataCommand.append(str(el_cs))

                print("bash command for LmdData creation")
                print(lmdDataCommand)
                _ = subprocess.call(lmdDataCommand)

                # was apparently bunches
                task.lastState = SimulationState.MERGE

                lmdDataCommand.clear()
                return recipe

        # 3. merge data
        if task.simState == SimulationState.MERGE:
            # TODO: kick this case switch!
            if task.simDataType == SimulationDataType.VERTEX:
                pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage, dataMode=DataMode.VERTEXDATA)
            else:
                pathToRootFiles = generateAbsoluteROOTDataPath(configPackage=configPackage)

            mergePath = pathToRootFiles / generateRelativeBunchesDir() / generateRelativeBinningDir() / generateRelativeMergeDir()

            status_code = wasSimulationSuccessful(
                directory=mergePath,
                glob_pattern=data_pattern + "*",
            )

            if status_code == StatusCode.ENOUGH_FILES:
                print("skipping data merging...")
                task.lastState = SimulationState.MERGE
                task.simState = SimulationState.DONE

            # this is just to be sure, but since this is NOT
            # done on a separate compute node, this should never
            # happen
            elif status_code == StatusCode.STILL_RUNNING:
                return recipe

            elif status_code == StatusCode.NO_FILES:
                os.chdir(lmd_fit_script_path)
                mergeCommand: List[str] = []
                mergeCommand.append("python")
                mergeCommand.append("mergeMultipleLmdData.py")
                mergeCommand.append(str(task.simDataType.value))  # we have to give the value because the script expects a/er/v !
                mergeCommand.append(str(pathToRootFiles))

                if task.simDataType == SimulationDataType.ANGULAR:
                    mergeCommand.append("--num_samples")
                    mergeCommand.append(str(bootstrapped_num_samples))

                print(f"running command:\n{mergeCommand}")
                _ = subprocess.call(mergeCommand)

                task.lastState = SimulationState.MERGE
                task.simState = SimulationState.DONE

    # remove done tasks
    recipe.SimulationTasks = [simTask for simTask in recipe.SimulationTasks if simTask.simState != SimulationState.DONE]

    return recipe


def lumiDetermination(thisExperiment: ExperimentParameters, recipe: SimRecipe) -> None:
    # open cross section file (this was generated by apps/generatePbarPElasticScattering
    elastic_cross_section_file_path = thisExperiment.experimentDir / "elastic_cross_section.txt"
    if elastic_cross_section_file_path.exists():
        print("Found an elastic cross section file!")
        with open(elastic_cross_section_file_path, "r") as f:
            content = f.readlines()
            recipe.elastic_pbarp_integrated_cross_secion_in_mb = float(content[0])
    elif recipe.lumiDetState == LumiDeterminationState.SIMULATE_VERTEX_DATA:
        print("No elastic cross section file found. This is ok if you are first simulating vertex data!")
    else:
        raise FileNotFoundError("ERROR! Can not find elastic cross section file! The determined Luminosity will be wrong!\n")

    print(f"processing recipe {thisExperiment.experimentDir} at step {recipe.lumiDetState}")

    finished = False

    # by default, we always start with SIMULATE_VERTEX_DATA, even though
    # runSimulationReconstruction has probably already been run.
    if recipe.lumiDetState == LumiDeterminationState.SIMULATE_VERTEX_DATA:
        """
        State 1 simulates vertex data from which the IP can be determined.
        """
        if len(recipe.SimulationTasks) == 0:
            recipe.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.VERTEX, simState=SimulationState.START_SIM))

        recipe = simulateDataOnHimster(thisExperiment, recipe)

        # TODO: add blocking here, the later branches should only be executed if the simulation is done

        # when all sim Tasks are done, the IP can be determined
        if len(recipe.SimulationTasks) == 0:
            recipe.lastLumiDetState = LumiDeterminationState.SIMULATE_VERTEX_DATA
            recipe.lumiDetState = LumiDeterminationState.DETERMINE_IP

    if recipe.lumiDetState == LumiDeterminationState.DETERMINE_IP:
        """
        state 2 only handles the reconstructed IP. If use_ip_determination is set,
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
                bashCommand.append(str(lmd_fit_bin_path / "determineBeamOffset"))
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
            print("============================================================")
            print("")
            print("Finished IP determination for this recipe!")

        else:
            print("Skipped IP determination for this recipe, using values from config.")

        recipe.lastLumiDetState = LumiDeterminationState.DETERMINE_IP
        recipe.lumiDetState = LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP

    if recipe.lumiDetState == LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP:
        # state 3a:
        # the IP position is now reconstructed. filter the DPM data again,
        # this time using the newly determined IP position as a cut criterion.
        # (that also means create fileLists -> bunches/binning -> merge)
        # 3b. generate acceptance and resolution with these reconstructed ip values
        # (that means simulation + bunching + creating data objects + merging)

        # TODO: there is an error here, only need to make sure the IP was reconstruced, that has nothing to do with the number of simTasks
        if len(recipe.SimulationTasks) == 0:
            recipe.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.ANGULAR, simState=SimulationState.START_SIM))
            recipe.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.EFFICIENCY_RESOLUTION, simState=SimulationState.START_SIM))

        # remember, the experiment config may now contain the updated IP
        recipe = simulateDataOnHimster(thisExperiment, recipe)

        # TODO: add blocking here, the later branches should only be executed if the simulation is done

        # when all simulation Tasks are done, the Lumi Fit may start
        if len(recipe.SimulationTasks) == 0:
            recipe.lastLumiDetState = LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP
            recipe.lumiDetState = LumiDeterminationState.RUN_LUMINOSITY_FIT

    if recipe.lumiDetState == LumiDeterminationState.RUN_LUMINOSITY_FIT:
        """
        4. runLmdFit!
        """

        os.chdir(lmd_fit_script_path)
        print("running lmdfit!")

        lumiFitCommand: List[str] = []
        lumiFitCommand.append("python")
        lumiFitCommand.append("doMultipleLuminosityFits.py")
        lumiFitCommand.append("-e")
        lumiFitCommand.append(f"{args.ExperimentConfigFile}")

        print(f"Bash command for LumiFit is:\n{' '.join(lumiFitCommand)}")
        _ = subprocess.call(lumiFitCommand)

        print("this recipe is fully processed!!!")
        finished = True

    # if we are in an intermediate step then push on the waiting stack and
    # increase step state
    if not finished:
        waiting_recipe_stack.append(recipe)


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

parser.add_argument(
    "-e",
    "--experiment_config",
    dest="ExperimentConfigFile",
    type=Path,
    help="The Experiment.config file that holds all info.",
    required=True,
)

parser.add_argument(
    "--use_devel_queue",
    action="store_true",
    help="If flag is set, the devel queue is used",
)

args = parser.parse_args()


# load experiment config
experiment: ExperimentParameters = load_params_from_file(args.ExperimentConfigFile, ExperimentParameters)

# before anything else, check if there is a copy in the experiment dir. if not, make it.
if not (experiment.experimentDir / "experiment.config").exists():
    print("No copy of the experiment config found in the experiment directory. Copying it there now.")
    write_params_to_file(experiment, experiment.experimentDir, "experiment.config")


# read environ settings
lmd_fit_script_path = experiment.softwarePaths.LmdFitScripts
lmd_fit_bin_path = experiment.softwarePaths.LmdFitBinaries / "bin"

# * ----------------- create a recipe object. it resides only in memory and is never written to disk
# it is however liberally modified and passed around
recipe = SimRecipe()

# set via command line argument, usually 1
bootstrapped_num_samples = args.bootstrapped_num_samples

# * ----------------- check which cluster we're on and create job handler
if experiment.cluster == ClusterEnvironment.VIRGO:
    job_handler = create_virgo_job_handler("long")
elif experiment.cluster == ClusterEnvironment.HIMSTER:
    if args.use_devel_queue:
        job_handler = create_himster_job_handler("devel")
    else:
        job_handler = create_himster_job_handler("himster2_exp")
else:
    raise NotImplementedError("This cluster type is not implemented!")

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = ClusterJobManager(job_handler, 2000, 3600)

# * ----------------- start the lumi function for the first time.
# it will be run again because it is implemented as a loop over a state-machine-like thing (for now...)

"""
TODO for much later:
use a thread pool. for each given expConfig, a new thread is spawned.
each thread handles 1 (one, in words: O-N-E) experiment config.
then this entire state bullshit can be tossed.

so, for each thread:

the lumiDetermination() function executes all individual steps in sequence, nothing else.
no more fucking states.
but since each step may call simulateDataOnHimster(), each step must block.
no more state bullshit.

therefor the simulateDataOnHimster() function must only return once all tasks are done.
this is ensured in the handleTasks function (which doesn't exist yet). this loops 
over all tasks and checks if they are done, and only then returns.

Thread safety: since multiple threads can now call the clusterManager, that thing must be
thread-safe. 
use the "with clusterManager as manager:" directive, to ensure only one instance is there
(singleton, just like with the agent).

The manager blocks as soon as a job should be submitted and only returns once the submission
was successful. No other error handling here, either the job was submitted successfully, or 
the other threads have to wait anyway.  
"""
lumiDetermination(experiment, recipe)

# TODO: okay this is tricky, sometimes recipes are pushed to the waiting stack,
# and then they are run again? let's see if we can do this some other way.
# while len(waiting_recipe_stack) > 0:
while len(active_recipe_stack) > 0 or len(waiting_recipe_stack) > 0:
    for scen in active_recipe_stack:
        lumiDetermination(experiment, scen)

    # clear active stack, if the recipe needs to be processed again,
    # it will be placed in the waiting stack
    active_recipe_stack = []
    # if all recipes are currently processed just wait a bit and check again
    # TODO: I think it would be better to wait for a real signal and not just "when enough files are there"
    if len(waiting_recipe_stack) > 0:
        print("currently waiting for 15 min to process recipes again")
        print("press ctrl C (ONCE) to skip this waiting round.")

        try:
            time.sleep(5 * 60)  # wait for 5 min
        except KeyboardInterrupt:
            print("skipping wait.")

        print("press ctrl C again withing 3 seconds to kill this script")
        try:
            time.sleep(3)
        except KeyboardInterrupt:
            print("understood. killing program.")
            sys.exit(1)

        active_recipe_stack = waiting_recipe_stack
        waiting_recipe_stack = []
