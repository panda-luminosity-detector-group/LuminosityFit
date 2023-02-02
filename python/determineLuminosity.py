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
from lumifit.scenario import LumiDeterminationState, Scenario, SimulationState
from lumifit.simulation import create_simulation_and_reconstruction_job

"""
This file needs one major rewrite, thats for sure.
- the logic is implemented as a GIANT state machine
- the states have no clear names
- the transitions between states are impossible to understand
- job_manager (and other objects) are defined globally and used in functions
- agent is not thead safe, but multiple job_managers are created
- job supervision is based on number of files;
- AND number of running jobs, even if these jobs belong to some other function
- but its not momitored if a given job has crashed
- and the entire thing is declarative, when object orientation would be better suited
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

    #! NEVER CREATE ANOTHER JOB HANDLER, THEY DEADLOCK THE SLURM AGENT
    # if experiment.cluster == ClusterEnvironment.VIRGO:
    #     job_handler = create_virgo_job_handler("long")
    # elif experiment.cluster == ClusterEnvironment.HIMSTER:
    #     if args.use_devel_queue:
    #         job_handler = create_himster_job_handler("devel")
    #     else:
    #         job_handler = create_himster_job_handler("himster2_exp")

    # TODO: I think there is a bug here, sometimes files are ready AFTER this check, this leads to a wrong lumi
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

    print(f"files percentage is smaller than requires. returning 0.")
    return return_value


# ----------------------------------------------------------------------------
# ok we do it in such a way we have a step state for each directory and stacks
# we try to process
#! nope, ONE scenario, ONE experiment
active_scenario_stack = []
#! oh shit, I think this is still needed because the function is executed multiple times,
#! with different internal states (which is just, just horrible)
waiting_scenario_stack = []


def simulateDataOnHimster(thisExperiment: Experiment, thisScenario: Scenario) -> Scenario:
    """Determines the luminosity of a set of Lumi_TrksQA_* files.

    This is pretty complicated and uses a lot of intermediate steps.
    Many of them have to be run on a cluster system, which is done
    with a slurm job handler.

    The states determine what step is currently being run:

    1:  Simulate Data (so box/dpm)
        Runs RunLmdSim and RunLmdReco to get MC Data and reconstructed
        tracks. This is the same as what the runSimulationReconstruction
        script does.

    2:  create Data (bunch data objects)
        first thing it should do is create bunches/binning dirs.
        Then call createMultipleLmdData which looks for these dirs.
        This also finds the reconstructed IP position from the TrksQA files.

    3:  Merge bunched Data


    Parameters:
    - dir_path: the path to the TrksQA files (e.g. 1-100_uncut/no_alignment_correction)
    - state: integer from 1 to 4


    """
    tasks_to_remove = []

    lab_momentum = thisScenario.momentum
    for simulation_task in thisScenario.simulation_info_lists:

        # what what what
        dir_path = simulation_task[0]
        sim_type = simulation_task[1]
        simState: SimulationState = simulation_task[2]
        last_state: SimulationState = simulation_task[3]

        print(f"running simulation of type {str(sim_type)} and path ({dir_path}, type {type(dir_path)}) at state={str(simState)}/{str(last_state)}")

        data_keywords = []
        data_pattern = ""

        # todo: don't generate here, there is a function for that in reconstruction.py.
        # cut_keyword = ""
        # if thisScenario.use_xy_cut:
        #     cut_keyword += "xy_"
        # if thisScenario.use_m_cut:
        #     cut_keyword += "m_"
        # if cut_keyword == "":
        #     cut_keyword += "un"
        # cut_keyword += "cut_real"
        cut_keyword = generateCutKeyword(thisExperiment.recoParams)

        # print(f" so wait. cutkeyword is {cut_keyword}")
        # if "uncut" in cut_keyword and "real" in cut_keyword:
        #     cut_keyword = "uncut"
        #     print("shit how can it be uncut but real")
        print(f"cut keyword is {cut_keyword}")

        merge_keywords = ["merge_data", "binning_300"]
        if "v" in sim_type:
            data_keywords = ["uncut", "bunches", "binning_300"]
            data_pattern = "lmd_vertex_data_"
        elif "a" in sim_type:
            data_keywords = [cut_keyword, "bunches", "binning_300"]
            data_pattern = "lmd_data_"
        else:
            data_keywords = [cut_keyword, "bunches", "binning_300"]
            data_pattern = "lmd_res_data_"

        # 1. simulate data
        # if state == 1:
        if simState == SimulationState.SIMULATION:
            os.chdir(lmd_fit_script_path)
            status_code = 1
            if "er" in sim_type:
                found_dirs = []
                # what the shit, this should never be empty in the first place
                if (dir_path != "") and (dir_path is not None):
                    print(f'\n\n\n GREP OUTPUT DIR: dir_path check !="" failed wirh dir_path: {dir_path}, type is {type(dir_path)}')
                    temp_dir_searcher = general.DirectorySearcher(
                        [
                            thisExperiment.recoParams.sim_type_for_resAcc.value,
                            data_keywords[0],
                        ]  # look for the folder name including sim_type_for_resAcc
                    )
                    temp_dir_searcher.searchListOfDirectories(dir_path, thisScenario.track_file_pattern)
                    found_dirs = temp_dir_searcher.getListOfDirectories()
                    print(f"found dirs now: {found_dirs}")
                else:
                    print(f"\n\n\n Well shit, dir_path is {dir_path} (or None? type is {type(dir_path)}), what now?!\n\n\n")

                if found_dirs:
                    status_code = wasSimulationSuccessful(
                        thisExperiment,
                        found_dirs[0],
                        thisScenario.track_file_pattern + "*.root",
                    )
                # elif last_state < 1:
                elif last_state < SimulationState.SIMULATION:
                    # then lets simulate!
                    # this command runs the full sim software with box gen data
                    # to generate the acceptance and resolution information
                    # for this sample
                    # note: beam tilt and divergence are not necessary here,
                    # because that is handled completely by the model

                    thisIPX = thisExperiment.recoParams.recoIPX
                    thisIPY = thisExperiment.recoParams.recoIPY
                    thisIPZ = thisExperiment.recoParams.recoIPZ

                    max_xy_shift = math.sqrt(thisIPX**2 + thisIPY**2)
                    max_xy_shift = float("{0:.2f}".format(round(float(max_xy_shift), 2)))

                    # TODO why is this read again?! Has it changed?
                    # TODO It was read right at the beginning to the experiment/thisExperiment object
                    # sim_par = SimulationParameters(
                    #     sim_type=SimulationType.BOX,
                    #     num_events_per_sample=box_num_events_per_sample,
                    #     num_samples=box_num_samples,
                    #     lab_momentum=lab_momentum,
                    # )

                    #! these mus be applied again, because they change at run time
                    # (the config on disk doesn't change and doesn't know this)
                    sim_par = thisExperiment.simParams
                    sim_par.sim_type = thisExperiment.recoParams.sim_type_for_resAcc
                    sim_par.num_events_per_sample = thisExperiment.recoParams.num_events_per_box_sample
                    sim_par.num_samples = thisExperiment.recoParams.num_box_samples
                    sim_par.theta_min_in_mrad -= max_xy_shift
                    sim_par.theta_max_in_mrad += max_xy_shift

                    # IP offset must also change for res/acc simulation!
                    sim_par.ip_offset_x = thisIPX
                    sim_par.ip_offset_y = thisIPY
                    sim_par.ip_offset_z = thisIPZ

                    # reconstruction parameters should be identical to dpm data
                    # EXCEPT for new IP if it comes from the gauss fit. but that was written
                    # to the reco params in memory as well

                    # TODO: alignment part
                    # if alignement matrices were specified, we used them as a mis-alignment
                    # and alignment for the box simulations
                    # align_par = AlignmentParameters()
                    # if (
                    #     thisScenario.alignment_parameters.alignment_matrices_path
                    # ):
                    #     align_par.misalignment_matrices_path = (
                    #         thisScenario.alignment_parameters.alignment_matrices_path
                    #     )
                    #     align_par.alignment_matrices_path = (
                    #         thisScenario.alignment_parameters.alignment_matrices_path
                    #     )

                    # TODO: make sure this object has the correct parameters!
                    # align_par = thisExperiment.alignParams

                    # update the sim and reco par dicts

                    (job, dir_path) = create_simulation_and_reconstruction_job(
                        sim_par,
                        thisExperiment.alignParams,
                        thisExperiment.recoParams,
                        application_command=thisScenario.Sim,
                        use_devel_queue=args.use_devel_queue,
                    )
                    job_manager.append(job)

                    simulation_task[0] = dir_path
                    thisScenario.acc_and_res_dir_path = dir_path
                    # last_state += 1
                    # last state was < 1, so 0. That means an increase is now 1
                    last_state = SimulationState.SIMULATION

            elif "a" in sim_type:
                """
                a is the angular case. this is the data set onto which the luminosiy fit is performed.
                it is therefore REAL digi data that must be reconstructed again with the updated reco
                parameters.
                """
                found_dirs = []
                status_code = 1
                # what the shit, this should never be empty in the first place
                if (dir_path != "") and (dir_path is not None):
                    print(f'\n\n\n GREP OUTPUT DIR: dir_path check !="" failed wirh dir_path: {dir_path}, type is {type(dir_path)}')
                    temp_dir_searcher = general.DirectorySearcher(["dpm_elastic", data_keywords[0]])
                    temp_dir_searcher.searchListOfDirectories(dir_path, thisScenario.track_file_pattern)
                    found_dirs = temp_dir_searcher.getListOfDirectories()

                else:
                    print(f"\n\n\n Well shit, dir_path is {dir_path} (or None? type is {type(dir_path)}), what now?!\n\n\n")

                if found_dirs:
                    status_code = wasSimulationSuccessful(
                        thisExperiment,
                        found_dirs[0],
                        thisScenario.track_file_pattern + "*.root",
                    )

                # oh boi that's bound to be trouble with IntEnums
                # in this branch state is 1
                elif last_state < simState:
                    # so last_state must have been 0

                    # then lets do reco
                    # this command runs the track reco software on the
                    # elastic scattering data with the estimated ip position
                    # the estimated ip position has been written into the recoParams already
                    # note: beam tilt and divergence are not used here because
                    # only the last reco steps are rerun of the track reco

                    # TODO: save digi files instead of mc files!!
                    # we are either in the base dir or an "aligned" subdirectory,
                    # apply dirty hack here:

                    print(f"\n\nDEBUG: this is this scenarios dir path:\n{thisScenario.trackDirectory}\n\n")

                    # TODO: Wait, why do we need sim params here at all? There won't be any sim params during the actual experiment
                    # simParamFile = thisScenario.trackDirectory + "/../sim_params.config"
                    # if not os.path.exists(simParamFile):
                    #     simParamFile = thisScenario.trackDirectory + "/../../sim_params.config"

                    # TODO why is this read again?! Has it changed?
                    # TODO It was read right at the beginning to the experiment/thisExperiment object
                    # sim_par: SimulationParameters = (
                    #     general.load_params_from_file(
                    #         simParamFile, SimulationParameters
                    #     )
                    # )
                    # sim_par = thisExperiment.simParams

                    # TODO why is this read again?! Has it changed?
                    # TODO It was read right at the beginning to the experiment/thisExperiment object
                    # rec_par: ReconstructionParameters = (
                    #     general.load_params_from_file(
                    #         thisScenario.dir_path + "/reco_params.config",
                    #         ReconstructionParameters,
                    #     )
                    # )

                    # why are these newly assigned? Have they changed?
                    # rec_par.use_xy_cut = thisScenario.use_xy_cut
                    # rec_par.use_m_cut = thisScenario.use_m_cut

                    # Z value is always taken fron config beceause it can't be determined from fit
                    #! this it taking  the value from the config and putting it into the config again?!
                    # rec_par.reco_ip_offset = [
                    #     ip_info_dict[
                    #         "ip_offset_x"
                    #     ],  #! wait this can also come from the config!
                    #     ip_info_dict[
                    #         "ip_offset_y"
                    #     ],  #! wait this can also come from the config!
                    #     experiment.simParams.ip_offset_z,
                    # ]

                    # * reco params must be adjusted if the res/acc sample had more jobs or samples that the real
                    # (or dpm) data
                    rec_par = thisExperiment.recoParams
                    if thisExperiment.recoParams.num_samples > 0 and rec_par.num_samples > thisExperiment.recoParams.num_samples:
                        rec_par.num_samples = thisExperiment.recoParams.num_samples
                        # sim_par.num_samples = thisExperiment.recoParams.num_samples

                    # TODO: have alignment parameters changed? take them from the experiment
                    align_par = thisExperiment.alignParams

                    # this directory is .../100000/1-100_uncut/alignStuff, but we need the 100000
                    # os.path.dirname() thinks this is a filename and gives 1-100_uncut back, which
                    # is too high
                    # dirname = os.path.dirname(scenario.dir_path) + "/../"
                    # dataBaseDirectory is supposed to be the directory to the .../100000 (nEvents)
                    # NOT any deeper!
                    # TODO: wait. that's in the config anyway?!
                    dataBaseDirectory = str(thisExperiment.baseDataOutputDir)

                    print(f"DEBUG:\ndataBaseDirectory is {dataBaseDirectory}")

                    (job, dir_path) = create_reconstruction_job(
                        rec_par,
                        align_par,
                        dataBaseDirectory,
                        application_command=thisScenario.Reco,
                        use_devel_queue=args.use_devel_queue,
                    )
                    job_manager.append(job)

                    simulation_task[0] = dir_path
                    thisScenario.filteredTrackDirectory = dir_path

                    # last_state += 1
                    last_state = SimulationState.SIMULATION  # last_state was 0, now it's 1
            else:
                # just skip simulation for vertex data... we always have that..
                status_code = 0

            if status_code == 0:
                print("found simulation files, skipping")
                simState = SimulationState.BUNCHES  # state = 2
                last_state = SimulationState.SIMULATION  # = 1
            elif status_code > 0:
                print("still waiting for himster simulation jobs for " + sim_type + " data to complete...")
            else:
                raise ValueError("status_code is negative, which means number of running jobs can't be determined. ")

        # 2. create data (that means bunch data, create data objects)
        # if state == 2:
        if simState == SimulationState.MERGE:
            # check if data objects already exists and skip!
            temp_dir_searcher = general.DirectorySearcher(data_keywords)
            temp_dir_searcher.searchListOfDirectories(dir_path, data_pattern)
            found_dirs = temp_dir_searcher.getListOfDirectories()
            status_code = 1
            if found_dirs:
                status_code = wasSimulationSuccessful(
                    thisExperiment,
                    found_dirs[0],
                    data_pattern + "*",
                    is_bunches=True,
                )
            elif last_state < simState:
                os.chdir(lmd_fit_script_path)
                # bunch data
                # TODO: pass experiment config, or better yet, make class instead of script
                bashcommand = (
                    "python makeMultipleFileListBunches.py "
                    + f" --filenamePrefix {thisScenario.track_file_pattern}"
                    + " --files_per_bunch 10 --maximum_number_of_files "
                    + str(thisExperiment.recoParams.num_samples)
                    + " "
                    + dir_path
                )
                print(f"Bash command for bunch creation:\n{bashcommand}\n")
                _ = subprocess.call(bashcommand.split())
                # TODO: pass experiment config, or better yet, make class instead of script
                # create data
                bashArgs = []
                if "a" in sim_type:
                    el_cs = thisScenario.elastic_pbarp_integrated_cross_secion_in_mb
                    bashArgs.append("python")
                    bashArgs.append("createMultipleLmdData.py")
                    bashArgs.append("--dir_pattern")
                    bashArgs.append(data_keywords[0])
                    bashArgs.append("--jobCommand")
                    bashArgs.append(thisScenario.LmdData)
                    bashArgs.append(f"{lab_momentum:.2f}")
                    bashArgs.append(sim_type)
                    bashArgs.append(dir_path)
                    bashArgs.append("../dataconfig_xy.json")
                    # bashcommand = (
                    #     "python createMultipleLmdData.py "
                    #     + " --dir_pattern "
                    #     + data_keywords[0]
                    #     + f" --jobCommand '{thisScenario.LmdData}'"
                    #     + " "
                    #     + f"{lab_momentum:.2f}"
                    #     + " "
                    #     + sim_type
                    #     + " "
                    #     + dir_path
                    #     + " ../dataconfig_xy.json"  # TODO: use absolute path here!
                    # )
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
                    bashArgs.append(f"{lab_momentum:.2f}")
                    bashArgs.append(sim_type)
                    bashArgs.append(dir_path)
                    bashArgs.append("../dataconfig_xy.json")
                    # bashcommand = (
                    #     "python createMultipleLmdData.py "
                    #     + " --dir_pattern "
                    #     + data_keywords[0]
                    #     + f" --jobCommand '{thisScenario.LmdData}'"
                    #     + " "
                    #     + f"{lab_momentum:.2f}"
                    #     + " "
                    #     + sim_type
                    #     + " "
                    #     + dir_path
                    #     + " ../dataconfig_xy.json"  # TODO: use absolute path here!
                    # )
                print(bashArgs)
                _ = subprocess.call(bashArgs)

                # last_state = last_state + 1
                # was apparently bunches
                last_state = SimulationState.MERGE

                bashArgs.clear()

            if status_code == 0:
                print("skipping bunching and data object creation...")
                # state = 3
                simState = SimulationState.MERGE
                last_state = SimulationState.BUNCHES
            elif status_code > 0:
                print(f"statuscode {status_code}: still waiting for himster simulation jobs for " + sim_type + " data to complete...")
            else:
                # ok something went wrong there, exit this scenario and
                # push on bad scenario stack
                simState = SimulationState.FAILED
                raise ValueError(
                    "ERROR: Something went wrong with the cluster jobs! " "This scenario will be pushed onto the dead stack, " "and is no longer processed."
                )

        # 3. merge data
        # if state == 3:
        if simState == SimulationState.MERGE:
            # check first if merged data already exists and skip it!
            temp_dir_searcher = general.DirectorySearcher(merge_keywords)
            temp_dir_searcher.searchListOfDirectories(dir_path, data_pattern)
            found_dirs = temp_dir_searcher.getListOfDirectories()
            if not found_dirs:
                os.chdir(lmd_fit_script_path)
                # merge data
                if "a" in sim_type:
                    bashcommand = (
                        "python mergeMultipleLmdData.py"
                        + " --dir_pattern "
                        + data_keywords[0]
                        + " --num_samples "
                        + str(bootstrapped_num_samples)
                        + " "
                        + sim_type
                        + " "
                        + dir_path
                    )
                else:
                    bashcommand = "python mergeMultipleLmdData.py" + " --dir_pattern " + data_keywords[0] + " " + sim_type + " " + dir_path
                _ = subprocess.call(bashcommand.split())
            simState = SimulationState.DONE

        simulation_task[2] = simState
        simulation_task[3] = last_state

        if simulation_task[3] == SimulationState.FAILED:
            thisScenario.is_broken = True
            break

        # what the actual fuck
        if simulation_task[2] == 4:
            tasks_to_remove.append(simulation_task)
            print("Task is finished and will be removed from list!")

    for x in tasks_to_remove:
        del thisScenario.simulation_info_lists[thisScenario.simulation_info_lists.index(x)]
    return thisScenario


def lumiDetermination(thisExperiment: Experiment, thisScenario: Scenario) -> None:
    lumiTrksQAPath = thisScenario.trackDirectory

    lumiDetState: LumiDeterminationState = thisScenario.state
    last_state = thisScenario.last_state

    # open cross section file (this was generated by apps/generatePbarPElasticScattering
    if os.path.exists(lumiTrksQAPath + "/../../elastic_cross_section.txt"):
        print("Found an elastic cross section file!")
        with open(lumiTrksQAPath + "/../../elastic_cross_section.txt") as f:
            content = f.readlines()
            thisScenario.elastic_pbarp_integrated_cross_secion_in_mb = float(content[0])
            f.close()
    else:
        raise FileNotFoundError(f"ERROR! Can not find elastic cross section file! The determined Luminosity will be wrong!\n")

    print("processing scenario " + lumiTrksQAPath + " at step " + str(lumiDetState))

    thisScenario.alignment_parameters = thisExperiment.alignParams

    finished = False

    # if lumiDetState == 1:
    if lumiDetState == LumiDeterminationState.SIMULATE_VERTEX_DATA:
        """
        State 1 simulates vertex data from which the IP can be determined.

        TODO: I'm not entirely sure, but I think if we use the IP from the config, we don't need this at all?
        """
        if len(thisScenario.simulation_info_lists) == 0:
            thisScenario.simulation_info_lists.append([lumiTrksQAPath, "v", 1, 0])

        thisScenario = simulateDataOnHimster(thisExperiment, thisScenario)
        if thisScenario.is_broken:
            raise SystemError(f"ERROR! Scenario is broken! debug scenario info:\n{thisScenario}")
        if len(thisScenario.simulation_info_lists) == 0:
            # lumiDetState += 1
            # last_state += 1
            # state was 1, last state was 0
            lumiDetState = LumiDeterminationState.DETERMINE_IP
            last_state = LumiDeterminationState.SIMULATE_VERTEX_DATA

    # if lumiDetState == 2:
    if lumiDetState == LumiDeterminationState.DETERMINE_IP:
        """
        state 2 only handles the reconstructed IP. If use_ip_determination is set,
        the IP is reconstructed from the uncut data set and written to a reco_ip.json
        file. The IP position will only be used from this file from now on.

        If use_ip_determination is false, the reconstruction will use the IP as
        specified from the reco params of the experiment config.

        Therefore, this is the ONLY place where thisScenario.ip[X,Y,Z] must be set.
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
            # nothing must be reassigned here because the values are already in the config

        # state was 2, last state was 1
        # lumiDetState += 1
        # last_state += 1
        lumiDetState = LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP
        last_state = LumiDeterminationState.DETERMINE_IP

    # if lumiDetState == 3:
    if lumiDetState == LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP:

        # state 3a:
        # the IP position is now reconstructed. filter the DPM data again,
        # this time using the newly determined IP position as a cut criterion.
        # (that again means bunch -> create -> merge)
        # 3b. generate acceptance and resolution with these reconstructed ip
        # values
        # (that means simulation + bunching + creating data objects + merging)
        # because this data is now with a cut applied, the new directory is called
        # something 1-100_xy_m_cut_real
        if len(thisScenario.simulation_info_lists) == 0:
            thisScenario.simulation_info_lists.append(["", "a", 1, 0])
            thisScenario.simulation_info_lists.append(["", "er", 1, 0])

        # all info needed for the COMPLETE reconstruction chain is here
        thisScenario = simulateDataOnHimster(thisExperiment, thisScenario)
        if thisScenario.is_broken:
            print(f"ERROR! Scenario is broken! debug scenario info:\n{thisScenario}")
            # dead_scenario_stack.append(thisScenario)
            return

        if len(thisScenario.simulation_info_lists) == 0:
            # lumiDetState += 1
            # last_state += 1
            # lumiDetState was 3
            lumiDetState = LumiDeterminationState.RUN_LUMI_FIT
            last_state = LumiDeterminationState.RECONSTRUCT_WITH_NEW_IP

    # if lumiDetState == 4:
    if lumiDetState == LumiDeterminationState.RUN_LUMI_FIT:
        """
        4. runLmdFit!

        - look where the merged data is, find the lmd_fitted_data root files.
        -

        """
        temp_dir_searcher = general.DirectorySearcher(["merge_data", "binning_300"])
        temp_dir_searcher.searchListOfDirectories(thisScenario.filteredTrackDirectory, "lmd_fitted_data")
        found_dirs = temp_dir_searcher.getListOfDirectories()

        #  TODO: so we just assume no dirs are found and specify the path manually anyway? then remove the stuff above
        if not found_dirs:

            os.chdir(lmd_fit_script_path)
            print("running lmdfit!")

            # TODO: nope, don't generate this here. the cut keyword are also generated in the reconstruction.py,
            # so take it from there too!
            # cut_keyword = ""
            # if thisScenario.use_xy_cut:
            #     cut_keyword += "xy_"
            # if thisScenario.use_m_cut:
            #     cut_keyword += "m_"
            # if cut_keyword == "":
            #     cut_keyword += "un"
            # cut_keyword += "cut_real "
            cut_keyword = generateCutKeyword(thisExperiment.recoParams)

            bashcommand = f"python doMultipleLuminosityFits.py --forced_resAcc_gen_data {thisScenario.acc_and_res_dir_path} -e {args.ExperimentConfigFile} {thisScenario.filteredTrackDirectory} {cut_keyword} {lmd_fit_path}/{thisExperiment.fitConfigPath}"
            print(f"Bash command is:\n{bashcommand}")
            _ = subprocess.call(bashcommand.split())

        print("this scenario is fully processed!!!")
        finished = True

    # if we are in an intermediate step then push on the waiting stack and
    # increase step state
    #! I don't think we should ever reach this point?
    if not finished:
        thisScenario.state = lumiDetState
        thisScenario.last_state = last_state
        print(f"WARNING! Scenario is not finished, but apparently this loop must be done multiple times?")
        waiting_scenario_stack.append(thisScenario)

    def determineLumiNew() -> None:
        pass


#! --------------------------------------------------
#!             script part starts here
#! --------------------------------------------------

parser = argparse.ArgumentParser(description="Lmd One Button Script", formatter_class=general.SmartFormatter)

parser.add_argument(
    "--bootstrapped_num_samples",
    type=int,
    default=1,
    help="number of elastic data samples to create via" " bootstrapping (for statistical analysis)",
)

# nope, no runtime args, only config

# parser.add_argument(
#     "--disable_xy_cut",
#     action="store_true",
#     help="Disable the x-theta & y-phi filter after the "
#     "tracking stage to remove background.",
# )
# parser.add_argument(
#     "--disable_m_cut",
#     action="store_true",
#     help="Disable the tmva based momentum cut filter after the"
#     " backtracking stage to remove background.",
# )
# parser.add_argument(
#     "--disable_ip_determination",
#     action="store_true",
#     help="Disable the determination of the IP. Instead (0,0,0) is assumed",
# )
# parser.add_argument(
#     "--use_devel_queue",
#     action="store_true",
#     help="If flag is set, the devel queue is used",
# )

parser.add_argument(
    "-e",
    "--experiment_config",
    dest="ExperimentConfigFile",
    type=Path,
    help="The Experiment.config file that holds all info.",
    required=True,
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
# the config only holds the base directory (i. where the first sim_params is)
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

# nope, no runtime args, only config

# if args.disable_xy_cut:
#     print("Disabling xy cut!")
#     thisScenario.use_xy_cut = False  # for testing purposes
# if args.disable_m_cut:
#     print("Disabling m cut!")
#     thisScenario.use_m_cut = False  # for testing purposes
# if args.disable_ip_determination:
#     print("Disabling IP determination!")
#     thisScenario.use_ip_determination = False

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
# it will be run again because it is imolemned as a state machine (for now...)
lumiDetermination(loadedExperimentFromConfig, thisScenario)

# TODO: okay this is tricky, sometimes scenarios are pushed to the waiting stack,
# and then they are run again? let's see if we can do this some other way.
# while len(waiting_scenario_stack) > 0:
while len(active_scenario_stack) > 0 or len(waiting_scenario_stack) > 0:
    for scen in active_scenario_stack:
        lumiDetermination(loadedExperimentFromConfig, scen)

    # clear active stack, if the sceenario needs to be processed again,
    # it will be placed in the waiting stack
    active_scenario_stack = []
    # if all scenarios are currently processed just wait a bit and check again
    # TODO: I think it would be better to wait for a real signal and not just "when enough files are there"
    if len(waiting_scenario_stack) > 0:
        print("currently waiting for 15 min to process scenarios again")
        print("press ctrl C (ONCE) to skip this waiting round.")
        # wait, thats not really robust. shouldn't we actually monitor the jobs?

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

# ------------------------------------------------------------------------------------------
