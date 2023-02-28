#!/usr/bin/env python3

from dotenv import load_dotenv

load_dotenv(dotenv_path="../lmdEnvFile.env", verbose=True)

import argparse
import json
import math
import os
import subprocess
import sys
import time
from pathlib import Path
from typing import List

import lumifit.general as general
from lumifit.alignment import AlignmentParameters
from lumifit.cluster import ClusterJobManager
from lumifit.experiment import ClusterEnvironment, Experiment
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.himster import create_himster_job_handler
from lumifit.reconstruction import (
    ReconstructionParameters,
    create_reconstruction_job,
    generateCutKeyword,
)
from lumifit.scenario import (
    LumiDeterminationState,
    Scenario,
    SimulationDataType,
    SimulationState,
    SimulationTask,
)
from lumifit.simulation import create_simulation_and_reconstruction_job

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


def wasSimulationSuccessful(
    thisExperiment: Experiment,
    directory: str,
    glob_pattern: str,
    # job_handler: JobHandler,  # there is a global job_handler, but this needs fixing anyway
    min_filesize_in_bytes: int = 10000,
    is_bunches: bool = False,
) -> int:
    print(f"checking if job was successful. checking in path {directory}, glob pattern {glob_pattern}")
    # return values:
    # 0: everything is fine
    # >0: its not finished processing, just keep waiting
    # <0: something went wrong...
    required_files_percentage = 0.8
    return_value = 0

    files_percentage = general.getGoodFiles(
        directory,
        glob_pattern,
        min_filesize_in_bytes=min_filesize_in_bytes,
        is_bunches=is_bunches,
    )[1]

    print(f"files percentage (depends on getGoodFiles) is {files_percentage}")

    if files_percentage < required_files_percentage:
        if job_handler.get_active_number_of_jobs() > 0:
            print(f"there are still {job_handler.get_active_number_of_jobs()} jobs running")
            return_value = 1
        else:
            print(
                "WARNING: "
                + str((1 - files_percentage) * 100)
                + "% of input files ("
                + str(glob_pattern)
                + ") missing and no jobs on himster remaining..."
                + " Something went wrong here..."
            )
            return_value = -1

    return return_value


# ----------------------------------------------------------------------------
# ok we do it in such a way we have a step state for each directory and stacks
# we try to process
active_scenario_stack: List[Scenario] = []
waiting_scenario_stack: List[Scenario] = []


def simulateDataOnHimster(thisExperiment: Experiment, thisScenario: Scenario) -> Scenario:
    """
    Start a SLURM job for every SimulationTask assigned to a scenario.
    A scenario may hold multiple SimulationTasks at the same time that
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
    even though jobs are still running. The Scenario objects therefore hold this state
    info so that the waiting method knows where it left off.

    Parameters:
    - dir_path: the path to the TrksQA files (e.g. 1-100_uncut/no_alignment_correction)
    """

    for task in thisScenario.SimulationTasks:

        print(f"running simulation of type {str(task.simDataType)} and path ({task.dirPath} at states:")
        print(f"current state: {str(task.simState)}")
        print(f"last state:    {str(task.lastState)}")

        data_keywords = []
        data_pattern = ""

        cut_keyword = generateCutKeyword(thisExperiment.recoParams)

        print(f"cut keyword is {cut_keyword}")

        merge_keywords = ["merge_data", "binning_300"]
        # if "v" in task.simType:
        if task.simDataType == SimulationDataType.VERTEX:
            data_keywords = ["uncut", "bunches", "binning_300"]
            data_pattern = "lmd_vertex_data_"
        # elif "a" in task.simType:
        elif task.simDataType == SimulationDataType.ANGULAR:
            data_keywords = [cut_keyword, "bunches", "binning_300"]
            data_pattern = "lmd_data_"
        elif task.simDataType == SimulationDataType.EFFICIENCY_RESOLUTION:
            data_keywords = [cut_keyword, "bunches", "binning_300"]
            data_pattern = "lmd_res_data_"
        else:
            raise NotImplementedError(f"Simulation type {task.simDataType} is not implemented!")

        # 1. simulate data
        if task.simState == SimulationState.START_SIM:
            os.chdir(lmd_fit_script_path)
            status_code = 1
            # if "er" in task.simType:
            if task.simDataType == SimulationDataType.EFFICIENCY_RESOLUTION:
                """
                efficiency / resolution calculation.

                Takes an offset of the IP into account.

                TODO: This needs to know the misalignment of the detector.
                """
                found_dirs = []
                # what the shit, this should never be empty in the first place
                if (task.dirPath != "") and (task.dirPath is not None):
                    temp_dir_searcher = general.DirectorySearcher(
                        [
                            thisExperiment.recoParams.simGenTypeForResAcc.value,
                            data_keywords[0],
                        ]  # look for the folder name including sim_type_for_resAcc
                    )
                    temp_dir_searcher.searchListOfDirectories(task.dirPath, thisScenario.track_file_pattern)
                    found_dirs = temp_dir_searcher.getListOfDirectories()
                    print(f"found dirs now: {found_dirs}")
                else:
                    # path may be empty, then the directory searcher tries to find it
                    pass

                if found_dirs:
                    status_code = wasSimulationSuccessful(
                        thisExperiment,
                        found_dirs[0],
                        thisScenario.track_file_pattern + "*.root",
                    )
                elif task.lastState < SimulationState.START_SIM:
                    # then lets simulate!
                    # this command runs the full sim software with box gen data
                    # to generate the acceptance and resolution information
                    # for this sample
                    # note: beam tilt and divergence are not necessary here,
                    # because that is handled completely by the model

                    # because we don't want to change the experiment config or
                    # anything in the simParams, recoParam, alignParams,
                    # we'll create temp objects here.

                    tempSimParams = thisExperiment.simParams
                    tempRecoParams = thisExperiment.recoParams
                    tempAlignParams = thisExperiment.alignParams

                    thisIPX = tempRecoParams.recoIPX
                    thisIPY = tempRecoParams.recoIPY
                    thisIPZ = tempRecoParams.recoIPZ

                    max_xy_shift = math.sqrt(thisIPX**2 + thisIPY**2)
                    max_xy_shift = float("{0:.2f}".format(round(float(max_xy_shift), 2)))

                    # since this is the res/acc case, these parameters must be changed
                    tempSimParams.simGeneratorType = tempRecoParams.simGenTypeForResAcc
                    tempSimParams.num_events_per_sample = tempRecoParams.num_events_per_resAcc_sample
                    tempSimParams.num_samples = tempRecoParams.num_resAcc_samples
                    tempSimParams.theta_min_in_mrad -= max_xy_shift
                    tempSimParams.theta_max_in_mrad += max_xy_shift
                    tempSimParams.ip_offset_x = thisIPX
                    tempSimParams.ip_offset_y = thisIPY
                    tempSimParams.ip_offset_z = thisIPZ

                    # since this is the res/acc case, these parameters must be updated
                    tempRecoParams.num_samples = tempRecoParams.num_resAcc_samples
                    tempRecoParams.num_events_per_sample = tempRecoParams.num_events_per_resAcc_sample

                    # TODO: alignment part
                    # if alignement matrices were specified, we used them as a mis-alignment
                    # and alignment for the box simulations

                    (job, returnPath) = create_simulation_and_reconstruction_job(
                        tempSimParams,
                        tempAlignParams,
                        tempRecoParams,
                        application_command=thisScenario.Sim,
                        use_devel_queue=args.use_devel_queue,
                    )
                    job_manager.append(job)

                    task.dirPath = returnPath
                    thisScenario.acc_and_res_dir_path = returnPath
                    # last_state += 1
                    # last state was < 1, so 0. That means an increase is now 1
                    task.lastState = SimulationState.START_SIM

            # elif "a" in task.simType:
            elif task.simDataType == SimulationDataType.ANGULAR:
                """
                a is the angular case. this is the data set onto which the luminosiy fit is performed.
                it is therefore REAL digi data (or DPM data of course) that must be reconstructed again
                with the updated reco parameter (like the IP position, cuts applied and alignment).
                note: beam tilt and divergence are not used here because
                only the last reco steps are rerun of the track reco
                """
                found_dirs = []
                status_code = 1
                # what the shit, this should never be empty in the first place
                if (task.dirPath != "") and (task.dirPath is not None):
                    temp_dir_searcher = general.DirectorySearcher(["dpm_elastic", data_keywords[0]])
                    temp_dir_searcher.searchListOfDirectories(task.dirPath, thisScenario.track_file_pattern)
                    found_dirs = temp_dir_searcher.getListOfDirectories()

                else:
                    # path may be empty, then the directory searcher tries to find it
                    pass

                if found_dirs:
                    status_code = wasSimulationSuccessful(
                        thisExperiment,
                        found_dirs[0],
                        thisScenario.track_file_pattern + "*.root",
                    )

                # oh boi that's bound to be trouble with IntEnums
                elif task.lastState < task.simState:

                    # * reco params must be adjusted if the res/acc sample had more jobs or samples that the real (or dpm) data
                    rec_par = thisExperiment.recoParams
                    if thisExperiment.recoParams.num_samples > 0 and rec_par.num_samples > thisExperiment.recoParams.num_samples:
                        rec_par.num_samples = thisExperiment.recoParams.num_samples

                    # TODO: have alignment parameters changed? take them from the experiment
                    align_par = thisExperiment.alignParams

                    (job, returnPath) = create_reconstruction_job(
                        rec_par,
                        align_par,
                        str(thisExperiment.baseDataOutputDir),
                        application_command=thisScenario.Reco,
                        use_devel_queue=args.use_devel_queue,
                    )
                    job_manager.append(job)

                    task.dirPath = returnPath
                    thisScenario.filteredTrackDirectory = returnPath

                    # Simulation is done, so update the last_state
                    task.lastState = SimulationState.START_SIM

            # elif "v" in task.simType:
            elif task.simDataType == SimulationDataType.VERTEX:

                # TODO: check if the sim data is already there, if yes return 0, else start sim
                status_code = 0

                # # vertex Data must always be created without any cuts first
                # tempRecoPars = thisExperiment.recoParams
                # tempRecoPars.use_xy_cut = False
                # tempRecoPars.use_m_cut = False

                # # TODO: misalignment is important here. the vertex data can have misalignment (because it's real data)
                # # but it has no alignment yet. that is only for the second reconstruction
                # tempAlignPars = thisExperiment.alignParams
                # tempAlignPars.alignment_matrices_path = None

                # job, _ = create_simulation_and_reconstruction_job(
                #     thisExperiment.simParams,
                #     tempAlignPars,
                #     tempRecoPars,
                #     use_devel_queue=args.use_devel_queue,
                #     application_command=thisScenario.Sim,
                # )
                # job_manager.append(job)

            else:
                raise ValueError(f"This tasks simType is {task.simDataType}, which is invalid!")

            if status_code == 0:
                print("found simulation files, skipping")
                task.simState = SimulationState.MAKE_BUNCHES
                task.lastState = SimulationState.START_SIM
            elif status_code > 0:
                print(f"still waiting for himster simulation jobs for {task.simDataType} data to complete...")
            else:
                raise ValueError("status_code is negative, which means number of running jobs can't be determined. ")

        # 2. create data (that means bunch data, create data objects)
        if task.simState == SimulationState.MAKE_BUNCHES:
            # check if data objects already exists and skip!
            temp_dir_searcher = general.DirectorySearcher(data_keywords)
            temp_dir_searcher.searchListOfDirectories(task.dirPath, data_pattern)
            found_dirs = temp_dir_searcher.getListOfDirectories()
            status_code = 1
            if found_dirs:
                status_code = wasSimulationSuccessful(
                    thisExperiment,
                    found_dirs[0],
                    data_pattern + "*",
                    is_bunches=True,
                )

            elif task.lastState < task.simState:
                os.chdir(lmd_fit_script_path)
                # bunch data
                # TODO: pass experiment config, or better yet, make class instead of script
                bashcommand = (
                    "python makeMultipleFileListBunches.py "
                    + f" --filenamePrefix {thisScenario.track_file_pattern}"
                    + " --files_per_bunch 10 --maximum_number_of_files "
                    + str(thisExperiment.recoParams.num_samples)
                    + " "
                    + task.dirPath
                )
                print(f"Bash command for bunch creation:\n{bashcommand}\n")
                _ = subprocess.call(bashcommand.split())
                # TODO: pass experiment config, or better yet, make class instead of script
                # create data
                bashArgs = []
                # if "a" in task.simType:
                if task.simDataType == SimulationDataType.ANGULAR:
                    el_cs = thisScenario.elastic_pbarp_integrated_cross_secion_in_mb
                    bashArgs.append("python")
                    bashArgs.append("createMultipleLmdData.py")
                    bashArgs.append("--dir_pattern")
                    bashArgs.append(data_keywords[0])
                    bashArgs.append("--jobCommand")
                    bashArgs.append(thisScenario.LmdData)
                    bashArgs.append(f"{thisScenario.momentum:.2f}")
                    bashArgs.append(str(task.simDataType.value))  # we have to give the value because the script expects a/er/v !
                    bashArgs.append(task.dirPath)
                    bashArgs.append("../dataconfig_xy.json")

                    if el_cs:
                        bashArgs.append("--elastic_cross_section")
                        bashArgs.append(str(el_cs))
                        # bashcommand += " --elastic_cross_section " + str(el_cs)
                else:
                    bashArgs.append("python")
                    bashArgs.append("createMultipleLmdData.py")
                    bashArgs.append("--dir_pattern")
                    bashArgs.append(data_keywords[0])
                    bashArgs.append("--jobCommand")
                    bashArgs.append(thisScenario.LmdData)
                    bashArgs.append(f"{thisScenario.momentum:.2f}")
                    bashArgs.append(str(task.simDataType.value))  # we have to give the value because the script expects a/er/v !
                    bashArgs.append(task.dirPath)
                    bashArgs.append("../dataconfig_xy.json")

                print(bashArgs)
                _ = subprocess.call(bashArgs)

                # last_state = last_state + 1
                # was apparently bunches
                task.lastState = SimulationState.MERGE

                bashArgs.clear()

            # else:
            #     raise RuntimeError("No data could be found, but no commands are to be executed. This can't be!")

            if status_code == 0:
                print("skipping bunching and data object creation...")
                # state = 3
                task.simState = SimulationState.MERGE
                task.lastState = SimulationState.MAKE_BUNCHES
            elif status_code > 0:
                print(f"status_code {status_code}: still waiting for himster simulation jobs for {task.simDataType} data to complete...")
            else:
                # ok something went wrong there, exit this scenario and
                # push on bad scenario stack
                task.simState = SimulationState.FAILED
                raise ValueError("Something went wrong with the cluster jobs! This scenario will no longer be processed.")

        # 3. merge data
        if task.simState == SimulationState.MERGE:
            # check first if merged data already exists and skip it!
            temp_dir_searcher = general.DirectorySearcher(merge_keywords)
            temp_dir_searcher.searchListOfDirectories(task.dirPath, data_pattern)
            found_dirs = temp_dir_searcher.getListOfDirectories()
            if not found_dirs:
                os.chdir(lmd_fit_script_path)
                # merge data
                # if "a" in task.simType:
                bashArgs = []
                if task.simDataType == SimulationDataType.ANGULAR:
                    bashArgs.append("python")
                    bashArgs.append("mergeMultipleLmdData.py")
                    bashArgs.append("--dir_pattern")
                    bashArgs.append(data_keywords[0])
                    bashArgs.append("--num_samples")
                    bashArgs.append(str(bootstrapped_num_samples))
                    bashArgs.append(str(task.simDataType.value))  # we have to give the value because the script expects a/er/v !
                    bashArgs.append(task.dirPath)

                else:
                    bashArgs.append("python")
                    bashArgs.append("mergeMultipleLmdData.py")
                    bashArgs.append("--dir_pattern")
                    bashArgs.append(data_keywords[0])
                    bashArgs.append(str(task.simDataType.value))  # we have to give the value because the script expects a/er/v !
                    bashArgs.append(task.dirPath)

                print("working directory:")
                print(f"{os.getcwd()}")
                print(f"running command:\n{bashArgs}")
                _ = subprocess.call(bashArgs)

            task.simState = SimulationState.DONE

        if task.lastState == SimulationState.FAILED:
            thisScenario.is_broken = True
            break

    # remove done tasks
    thisScenario.SimulationTasks = [simTask for simTask in thisScenario.SimulationTasks if simTask.simState != SimulationState.DONE]

    return thisScenario


def lumiDetermination(thisExperiment: Experiment, thisScenario: Scenario) -> None:
    lumiTrksQAPath = thisScenario.trackDirectory

    # open cross section file (this was generated by apps/generatePbarPElasticScattering
    if os.path.exists(lumiTrksQAPath + "/../../elastic_cross_section.txt"):
        print("Found an elastic cross section file!")
        with open(lumiTrksQAPath + "/../../elastic_cross_section.txt") as f:
            content = f.readlines()
            thisScenario.elastic_pbarp_integrated_cross_secion_in_mb = float(content[0])
            f.close()
    else:
        raise FileNotFoundError(f"ERROR! Can not find elastic cross section file! The determined Luminosity will be wrong!\n")

    print("processing scenario " + lumiTrksQAPath + " at step " + str(thisScenario.lumiDetState))

    thisScenario.alignment_parameters = thisExperiment.alignParams

    finished = False

    # if lumiDetState == 1:
    if thisScenario.lumiDetState == LumiDeterminationState.SIMULATE_VERTEX_DATA:
        """
        State 1 simulates vertex data from which the IP can be determined.
        """
        if len(thisScenario.SimulationTasks) == 0:
            thisScenario.SimulationTasks.append(
                SimulationTask(dirPath=lumiTrksQAPath, simDataType=SimulationDataType.VERTEX, simState=SimulationState.START_SIM)
            )

        thisScenario = simulateDataOnHimster(thisExperiment, thisScenario)
        if thisScenario.is_broken:
            raise SystemError(f"ERROR! Scenario is broken! debug scenario info:\n{thisScenario}")

        # when all sim Tasks are done, the IP can be determined
        if len(thisScenario.SimulationTasks) == 0:
            thisScenario.lumiDetState = LumiDeterminationState.DETERMINE_IP
            thisScenario.lastLumiDetState = LumiDeterminationState.SIMULATE_VERTEX_DATA

    # if lumiDetState == 2:
    if thisScenario.lumiDetState == LumiDeterminationState.DETERMINE_IP:
        """
        state 2 only handles the reconstructed IP. If use_ip_determination is set,
        the IP is reconstructed from the uncut data set and written to a reco_ip.json
        file. The IP position will only be used from this file from now on.

        If use_ip_determination is false, the reconstruction will use the IP as
        specified from the reco params of the experiment config.

        Therefore, this is the ONLY place where thisScenario.ip[X,Y,Z] may be set.
        """
        if thisExperiment.recoParams.use_ip_determination:
            temp_dir_searcher = general.DirectorySearcher(["merge_data", "binning_300"])
            temp_dir_searcher.searchListOfDirectories(lumiTrksQAPath, "reco_ip.json")
            found_dirs = temp_dir_searcher.getListOfDirectories()
            if not found_dirs:
                # 2. determine offset on the vertex data sample
                os.chdir(lmd_fit_bin_path)
                temp_dir_searcher = general.DirectorySearcher(["merge_data", "binning_300"])
                temp_dir_searcher.searchListOfDirectories(lumiTrksQAPath, ["lmd_vertex_data_", "of1.root"])
                found_dirs = temp_dir_searcher.getListOfDirectories()
                bashcommand = "./determineBeamOffset -p " + found_dirs[0] + " -c " + "../../vertex_fitconfig.json"
                _ = subprocess.call(bashcommand.split())
                ip_rec_file = found_dirs[0] + "/reco_ip.json"
            else:
                ip_rec_file = found_dirs[0] + "/reco_ip.json"

            file_content = open(ip_rec_file)
            ip_rec_data = json.load(file_content)

            thisExperiment.recoParams.recoIPX = float("{0:.3f}".format(round(float(ip_rec_data["ip_x"]), 3)))  # in cm
            thisExperiment.recoParams.recoIPY = float("{0:.3f}".format(round(float(ip_rec_data["ip_y"]), 3)))
            thisExperiment.recoParams.recoIPZ = float("{0:.3f}".format(round(float(ip_rec_data["ip_z"]), 3)))

            print("Finished IP determination for this scenario!")
        else:
            print("Skipped IP determination for this scenario, using values from config.")

        thisScenario.lumiDetState = LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP
        thisScenario.lastLumiDetState = LumiDeterminationState.DETERMINE_IP

    # if lumiDetState == 3:
    if thisScenario.lumiDetState == LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP:

        # state 3a:
        # the IP position is now reconstructed. filter the DPM data again,
        # this time using the newly determined IP position as a cut criterion.
        # (that again means bunch -> create -> merge)
        # 3b. generate acceptance and resolution with these reconstructed ip values
        # (that means simulation + bunching + creating data objects + merging)
        # because this data is now with a cut applied, the new directory is called
        # something 1-100_xy_m_cut_real
        if len(thisScenario.SimulationTasks) == 0:
            thisScenario.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.ANGULAR, simState=SimulationState.START_SIM))
            thisScenario.SimulationTasks.append(SimulationTask(simDataType=SimulationDataType.EFFICIENCY_RESOLUTION, simState=SimulationState.START_SIM))

        thisScenario = simulateDataOnHimster(thisExperiment, thisScenario)

        if thisScenario.is_broken:
            raise ValueError(f"ERROR! Scenario is broken! debug scenario info:\n{thisScenario}")

        # when all simulation Tasks are done, the Lumi Fit may start
        if len(thisScenario.SimulationTasks) == 0:
            thisScenario.lumiDetState = LumiDeterminationState.RUN_LUMINOSITY_FIT
            thisScenario.lastLumiDetState.RECONSTRUCT_WITH_NEW_IP

    # if lumiDetState == 4:
    if thisScenario.lumiDetState == LumiDeterminationState.RUN_LUMINOSITY_FIT:
        """
        4. runLmdFit!

        - look where the merged data is, find the lmd_fitted_data root files.
        """

        os.chdir(lmd_fit_script_path)
        print("running lmdfit!")

        # TODO: nope, don't generate this here. the cut keyword are also generated in the reconstruction.py,
        cut_keyword = generateCutKeyword(thisExperiment.recoParams)

        bashcommand = f"python doMultipleLuminosityFits.py --forced_resAcc_gen_data {thisScenario.acc_and_res_dir_path} -e {args.ExperimentConfigFile} {thisScenario.filteredTrackDirectory} {cut_keyword} {lmd_fit_path}/{thisExperiment.fitConfigPath}"
        print(f"Bash command is:\n{bashcommand}")
        _ = subprocess.call(bashcommand.split())

        print("this scenario is fully processed!!!")
        finished = True

    # if we are in an intermediate step then push on the waiting stack and
    # increase step state
    if not finished:
        waiting_scenario_stack.append(thisScenario)


#! --------------------------------------------------
#!             script part starts here
#! --------------------------------------------------

parser = argparse.ArgumentParser(description="Lmd One Button Script", formatter_class=general.SmartFormatter)

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
loadedExperimentFromConfig: Experiment = general.load_params_from_file(args.ExperimentConfigFile, Experiment)

# read environ settings
lmd_fit_script_path = os.environ["LMDFIT_SCRIPTPATH"]
lmd_fit_path = os.path.dirname(lmd_fit_script_path)
lmd_fit_bin_path = os.environ["LMDFIT_BUILD_PATH"] + "/bin"

# make scenario config
thisScenario = Scenario(
    str(loadedExperimentFromConfig.baseDataOutputDir),
    loadedExperimentFromConfig.experimentType,
    lmd_fit_script_path,
)
thisScenario.momentum = loadedExperimentFromConfig.recoParams.lab_momentum

# set via command line argument, usually 1
bootstrapped_num_samples = args.bootstrapped_num_samples

# * ----------------- find the path to the TracksQA files.
# the config only holds the base directory (i.e. where the first sim_params is)
dir_searcher = general.DirectorySearcher(["dpm_elastic", "uncut"])

dir_searcher.searchListOfDirectories(
    str(loadedExperimentFromConfig.baseDataOutputDir),
    thisScenario.track_file_pattern,
)
dirs = dir_searcher.getListOfDirectories()

print(f"\n\nINFO: found these dirs:\n{dirs}\n\n")

if len(dirs) != 1:
    print(f"found {len(dirs)} directory candidates but it should be only one. something is wrong!")

if len(dirs) < 1:
    raise FileNotFoundError("ERROR! No dirs found!")

# path has changed now for the newly found dir
thisScenario.trackDirectory = dirs[0]


# * ----------------- create a scenario object. it resides only in memory and is never written to disk
print("creating scenario:", thisScenario.trackDirectory)

# * ----------------- check which cluster we're on and create job handler
if loadedExperimentFromConfig.cluster == ClusterEnvironment.VIRGO:
    job_handler = create_virgo_job_handler("long")
elif loadedExperimentFromConfig.cluster == ClusterEnvironment.HIMSTER:
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
# it will be run again because it is implemented as a state machine (for now...)
lumiDetermination(loadedExperimentFromConfig, thisScenario)

# TODO: okay this is tricky, sometimes scenarios are pushed to the waiting stack,
# and then they are run again? let's see if we can do this some other way.
# while len(waiting_scenario_stack) > 0:
while len(active_scenario_stack) > 0 or len(waiting_scenario_stack) > 0:
    for scen in active_scenario_stack:
        lumiDetermination(loadedExperimentFromConfig, scen)

    # clear active stack, if the scenario needs to be processed again,
    # it will be placed in the waiting stack
    active_scenario_stack = []
    # if all scenarios are currently processed just wait a bit and check again
    # TODO: I think it would be better to wait for a real signal and not just "when enough files are there"
    if len(waiting_scenario_stack) > 0:
        print("currently waiting for 15 min to process scenarios again")
        print("press ctrl C (ONCE) to skip this waiting round.")

        try:
            time.sleep(900)  # wait for 15min
        except KeyboardInterrupt:
            print("skipping wait.")

        print("press ctrl C again withing 3 seconds to kill this script")
        try:
            time.sleep(3)
        except KeyboardInterrupt:
            print("understood. killing program.")
            sys.exit(1)

        active_scenario_stack = waiting_scenario_stack
        waiting_scenario_stack = []
