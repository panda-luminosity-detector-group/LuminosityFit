#!/usr/bin/env python3

# ok this whole procedure is extremely tedious and costly. unfortunately there is no other way...
# so dont be scared when you see all this...

import argparse
import glob
import json
import math
import os
import re
import subprocess
import sys
import time

import alignment
import general
import himster
import reconstruction
import simulation


class Scenario:
    def __init__(self, dir_path_):
        self.momentum = 0.0

        self.dir_path = dir_path_
        self.filtered_dir_path = ''
        self.acc_and_res_dir_path = ''
        self.rec_ip_info = {}
        self.elastic_pbarp_integrated_cross_secion_in_mb = None
        self.use_m_cut = True
        self.use_xy_cut = True
        self.use_ip_determination = True
        self.Lumi = True
        if Lumi:
            self.Track = 'Lumi_TrkQA_'
            self.MC = 'Lumi_MC_'
            self.Params = 'Lumi_Params_'
            self.Sim = './runLmdSimReco'
            self.Reco = './runLmdReco'
        else:
            self.Track = 'Koala_Track_'
            self.MC = 'Koala_MC_'
            self.Params = 'Koala_Params_'
            self.Sim = './runKoalaSimReco'
            self.Reco = './runKoalaReco'


        self.alignment_parameters = {}

        self.state = 1
        self.last_state = 0

        self.simulation_info_lists = []

        self.is_broken = False


def wasSimulationSuccessful(directory, glob_pattern, is_bunches=False):
    # return values:
    # 0: everything is fine
    # >0: its not finished processing, just keep waiting
    # <0: something went wrong...
    required_files_percentage = 0.8
    return_value = 0

    files_percentage = general.getGoodFiles(directory,
                                            glob_pattern,
                                            10000,
                                            is_bunches=is_bunches)[1]

    if files_percentage < required_files_percentage:
        if himster.get_num_jobs_on_himster() > 0:
            return_value = 1
        else:
            print('WARNING: ' + str((1-files_percentage)*100)
                  + '% of input files (' + str(glob_pattern)
                  + ') missing and no jobs on himster remaining...'
                  + ' Something went wrong here...')
            return_value = -1

    return return_value


# ----------------------------------------------------------------------------
# ok we do it in such a way we have a step state for each directory and stacks
# we try to process
active_scenario_stack = []
waiting_scenario_stack = []
dead_scenario_stack = []


def simulateDataOnHimster(scenario: Scenario):
    tasks_to_remove = []

    lab_momentum = scenario.momentum
    for simulation_task in scenario.simulation_info_lists:
        dir_path = simulation_task[0]
        sim_type = simulation_task[1]
        state = simulation_task[2]
        last_state = simulation_task[3]

        print('running simulation of type ' + str(sim_type) + ' and path (' +
              dir_path + ') at state=' + str(state) + '/' + str(last_state))

        data_keywords = []
        data_pattern = ''

        cut_keyword = ''
        if scenario.use_xy_cut:
            cut_keyword += 'xy_'
        if scenario.use_m_cut:
            cut_keyword += 'm_'
        if cut_keyword == '':
            cut_keyword += 'un'
        cut_keyword += 'cut_real'

        merge_keywords = ['merge_data', 'binning_300']
        if 'v' in sim_type:
            data_keywords = ['uncut', 'bunches', 'binning_300']
            data_pattern = 'lmd_vertex_data_'
        elif 'a' in sim_type:
            data_keywords = [cut_keyword, 'bunches', 'binning_300']
            data_pattern = 'lmd_data_'
        else:
            data_keywords = [cut_keyword, 'bunches', 'binning_300']
            data_pattern = 'lmd_res_data_'

        # 1. simulate data
        if state == 1:
            os.chdir(lmd_fit_script_path)
            status_code = 1
            if 'er' in sim_type:
                found_dirs = []
                if dir_path != '':
                    temp_dir_searcher = general.DirectorySearcher(
                        ['box', data_keywords[0]])
                    temp_dir_searcher.searchListOfDirectories(
                        dir_path, Track)
                    found_dirs = temp_dir_searcher.getListOfDirectories()

                if found_dirs:
                    status_code = wasSimulationSuccessful(
                        found_dirs[0], Track + '*.root')
                elif last_state < 1:
                    # then lets simulate!
                    # this command runs the full sim software with box gen data
                    # to generate the acceptance and resolution information
                    # for this sample
                    # note: beam tilt and divergence are not necessary here,
                    # because that is handled completely by the model
                    ip_info_dict = scenario.rec_ip_info
                    max_xy_shift = math.sqrt(ip_info_dict['ip_offset_x'] ** 2 +
                                             ip_info_dict['ip_offset_y'] ** 2)
                    max_xy_shift = float('{0:.2f}'.format(
                        round(float(max_xy_shift), 2)))

                    gen_par = general.createGeneralRunParameters(
                        box_num_events_per_sample,
                        box_num_samples, lab_momentum)
                    sim_par = simulation.createSimulationParameters('box')
                    sim_par['theta_min_in_mrad'] -= max_xy_shift
                    sim_par['theta_max_in_mrad'] += max_xy_shift
                    sim_par.update(gen_par)
                    rec_par = reconstruction.ReconstructionParmaters()
                    rec_par['use_xy_cut'] = scenario.use_xy_cut
                    rec_par['use_m_cut'] = scenario.use_m_cut
                    rec_par['reco_ip_offset'] = [ip_info_dict['ip_offset_x'],
                                                 ip_info_dict['ip_offset_y'],
                                                 ip_info_dict['ip_offset_z']]
                    rec_par.update(gen_par)

                    # alignment part
                    # if alignement matrices were specified, we used them as a mis-alignment
                    # and alignment for the box simulations
                    align_par = alignment.createAlignmentParameters()
                    if 'alignment_matrices_path' in scen.alignment_parameters:
                        align_par['misalignment_matrices_path'] = scen.alignment_parameters['alignment_matrices_path']
                        align_par['alignment_matrices_path'] = scen.alignment_parameters['alignment_matrices_path']
                    # update the sim and reco par dicts
                    sim_par.update(align_par)
                    rec_par.update(align_par)

                    (dir_path, is_finished) = simulation.startSimulationAndReconstruction(
                        sim_par, align_par, rec_par, use_devel_queue=args.use_devel_queue, scenario.Track, scenario.SimReco)
                    simulation_task[0] = dir_path
                    scenario.acc_and_res_dir_path = dir_path
                    if is_finished:
                        state += 1
                    last_state += 1

            elif 'a' in sim_type:
                found_dirs = []
                status_code = 1
                if dir_path != '':
                    temp_dir_searcher = general.DirectorySearcher(
                        ['dpm_elastic', data_keywords[0]])
                    temp_dir_searcher.searchListOfDirectories(
                        dir_path, Track)
                    found_dirs = temp_dir_searcher.getListOfDirectories()
                if found_dirs:
                    status_code = wasSimulationSuccessful(
                        found_dirs[0], Track + '*.root')

                elif last_state < state:
                    # then lets do reco
                    # this command runs the track reco software on the
                    # elastic scattering data with the estimated ip position
                    # note: beam tilt and divergence are not used here because
                    # only the last reco steps are rerun of the track reco
                    ip_info_dict = scenario.rec_ip_info

                    # TODO: save digi files instead of mc files!!
                    # we are either in the base dir or an "aligned" subdirectory,
                    # apply dirty hack here:
                    simParamFile = scenario.dir_path+'/../sim_params.config'
                    if not os.path.exists(simParamFile):
                        simParamFile = scenario.dir_path+'/../../sim_params.config'

                    with open(simParamFile, 'r') as json_file:
                        sim_par = json.load(json_file)
                    with open(scenario.dir_path + '/reco_params.config', 'r') as json_file:
                        rec_par = json.load(json_file)
                    rec_par['use_xy_cut'] = scenario.use_xy_cut
                    rec_par['use_m_cut'] = scenario.use_m_cut
                    rec_par['reco_ip_offset'] = [ip_info_dict['ip_offset_x'],
                                                 ip_info_dict['ip_offset_y'],
                                                 ip_info_dict['ip_offset_z']]
                    if (num_samples > 0 and
                            rec_par['num_samples'] > num_samples):
                        rec_par['num_samples'] = num_samples
                        sim_par['num_samples'] = num_samples
                    # dirname = os.path.dirname(scenario.dir_path)
                    (dir_path, is_finished) = simulation.startSimulationAndReconstruction(
                        sim_par, alignment.getAlignmentParameters(rec_par),
                        rec_par, use_devel_queue=args.use_devel_queue, scenario.Track , scenario.Sim)
                    # (dir_path, is_finished) = reconstruction.startReconstruction(
                    #    rec_par, alignment.getAlignmentParameters(rec_par),
                    #    dirname, use_devel_queue=args.use_devel_queue)
                    simulation_task[0] = dir_path
                    scen.filtered_dir_path = dir_path
                    if is_finished:
                        state += 1
                    last_state += 1
            else:
                # just skip simulation for vertex data... we always have that..
                print('skipping simulation step...')
                status_code = 0

            if status_code == 0:
                print('found simulation files, skipping')
                state = 2
                last_state = 1
            elif status_code > 0:
                print('still waiting for himster simulation jobs for ' +
                      sim_type + ' data to complete...')
            else:
                # ok something went wrong there, exit this scenario and
                # push on bad scenario stack
                last_state = -1

        # 2. create data (that means bunch data, create data objects)
        if state == 2:
            # check if data objects already exists and skip!
            temp_dir_searcher = general.DirectorySearcher(data_keywords)
            temp_dir_searcher.searchListOfDirectories(dir_path, data_pattern)
            found_dirs = temp_dir_searcher.getListOfDirectories()
            status_code = 1
            if found_dirs:
                status_code = wasSimulationSuccessful(
                    found_dirs[0], data_pattern + "*", True)
            elif last_state < state:
                os.chdir(lmd_fit_script_path)
                # bunch data
                bashcommand = 'python makeMultipleFileListBunches.py '\
                    '--files_per_bunch 10 --maximum_number_of_files ' + \
                    str(num_samples) + ' ' + dir_path
                returnvalue = subprocess.call(bashcommand.split())
                # create data
                if 'a' in sim_type:
                    el_cs = scenario.elastic_pbarp_integrated_cross_secion_in_mb
                    bashcommand = 'python createMultipleLmdData.py '\
                        + ' --dir_pattern ' + data_keywords[0] + ' ' + str(
                            lab_momentum) + ' ' + sim_type + ' ' + dir_path\
                        + ' ../dataconfig_xy.json'
                    if el_cs:
                        bashcommand += ' --elastic_cross_section ' + str(el_cs)
                else:
                    bashcommand = 'python createMultipleLmdData.py '\
                        + '--dir_pattern ' + data_keywords[0] + ' '\
                        + str(lab_momentum) + ' '\
                        + sim_type + ' ' + dir_path + ' ../dataconfig_xy.json'
                    print(bashcommand)
                returnvalue = subprocess.call(bashcommand.split())
                last_state = last_state + 1

            if status_code == 0:
                print('skipping bunching and data object creation...')
                state = 3
                last_state = 2
            elif status_code > 0:
                print('still waiting for himster simulation jobs for ' +
                      sim_type + ' data to complete...')
            else:
                # ok something went wrong there, exit this scenario and
                # push on bad scenario stack
                print("ERROR: Something went wrong with the cluster jobs! "
                      "This scenario will be pushed onto the dead stack, "
                      "and is no longer processed.")
                last_state = -1

        # 3. merge data
        if state == 3:
            # check first if merged data already exists and skip it!
            temp_dir_searcher = general.DirectorySearcher(merge_keywords)
            temp_dir_searcher.searchListOfDirectories(dir_path, data_pattern)
            found_dirs = temp_dir_searcher.getListOfDirectories()
            if not found_dirs:
                os.chdir(lmd_fit_script_path)
                # merge data
                if 'a' in sim_type:
                    bashcommand = 'python mergeMultipleLmdData.py'\
                        + ' --dir_pattern ' + data_keywords[0]\
                        + ' --num_samples ' + str(bootstrapped_num_samples)\
                        + ' ' + sim_type + ' ' + dir_path
                else:
                    bashcommand = 'python mergeMultipleLmdData.py'\
                        + ' --dir_pattern '+data_keywords[0]\
                        + ' ' + sim_type + ' ' + dir_path
                returnvalue = subprocess.call(bashcommand.split())
            state = 4

        simulation_task[2] = state
        simulation_task[3] = last_state

        if simulation_task[3] == -1:
            scenario.is_broken = True
            break
        if simulation_task[2] == 4:
            tasks_to_remove.append(simulation_task)
            print("Task is finished and will be removed from list!")

    for x in tasks_to_remove:
        del scenario.simulation_info_lists[
            scenario.simulation_info_lists.index(x)]
    return scenario


def lumiDetermination(scen):
    dir_path = scen.dir_path

    state = scen.state
    last_state = scen.last_state

    # open file
    if os.path.exists(dir_path + "/../../elastic_cross_section.txt"):
        print("Found an elastic cross section file!")
        with open(dir_path + "/../../elastic_cross_section.txt") as f:
            content = f.readlines()
            scen.elastic_pbarp_integrated_cross_secion_in_mb = float(
                content[0])
            f.close()
    else:
        print(f'ERROR! Can not find elastic cross section file! The determined Luminosity will be wrong!\n')
        sys.exit()

    print('processing scenario ' + dir_path + ' at step ' + str(state))

    # TODO: Not sure if later on the lab momentum has to be extracted from the data
    m = re.search('(\d*?\.\d*?)GeV', dir_path)
    momentum = float(m.group(1))
    scen.momentum = momentum

    with open(scen.dir_path + '/reco_params.config', 'r') as json_file:
        rec_par = json.load(json_file)
        scen.alignment_parameters = alignment.getAlignmentParameters(rec_par)

    finished = False
    # 1. create vertex data (that means bunch data, create data objects and merge)
    if state == 1:
        if len(scen.simulation_info_lists) == 0:
            scen.simulation_info_lists.append([dir_path, 'v', 1, 0])

        scen = simulateDataOnHimster(scen)
        if scen.is_broken:
            dead_scenario_stack.append(scen)
            return
        if len(scen.simulation_info_lists) == 0:
            state += 1
            last_state += 1

    if state == 2:
        # check if ip was already determined
        if scen.use_ip_determination:
            temp_dir_searcher = general.DirectorySearcher(
                ['merge_data', 'binning_300'])
            temp_dir_searcher.searchListOfDirectories(dir_path, 'reco_ip.json')
            found_dirs = temp_dir_searcher.getListOfDirectories()
            if not found_dirs:
                # 2. determine offset on the vertex data sample
                os.chdir(lmd_fit_bin_path)
                temp_dir_searcher = general.DirectorySearcher(
                    ['merge_data', 'binning_300'])
                temp_dir_searcher.searchListOfDirectories(
                    dir_path, ['lmd_vertex_data_', 'of1.root'])
                found_dirs = temp_dir_searcher.getListOfDirectories()
                bashcommand = './determineBeamOffset -p ' + \
                    found_dirs[0] + ' -c ' + '../../vertex_fitconfig.json'
                returnvalue = subprocess.call(bashcommand.split())
                ip_rec_file = found_dirs[0] + '/reco_ip.json'
            else:
                ip_rec_file = found_dirs[0] + '/reco_ip.json'

            file_content = open(ip_rec_file)
            ip_rec_data = json.load(file_content)

            scen.rec_ip_info['ip_offset_x'] = float('{0:.3f}'.format(
                round(float(ip_rec_data["ip_x"]), 3)))  # in cm
            scen.rec_ip_info['ip_offset_y'] = float(
                '{0:.3f}'.format(round(float(ip_rec_data["ip_y"]), 3)))
            scen.rec_ip_info['ip_offset_z'] = float(
                '{0:.3f}'.format(round(float(ip_rec_data["ip_z"]), 3)))

            print("Finished IP determination for this scenario!")
        else:
            scen.rec_ip_info['ip_offset_x'] = 0.0
            scen.rec_ip_info['ip_offset_y'] = 0.0
            scen.rec_ip_info['ip_offset_z'] = 0.0
            print("Skipped IP determination for this scenario!")

        state += 1
        last_state += 1

    if state == 3:
        # 3a. track filter the dpm data using the ip values and create ang
        # dist objects
        # (that again means bunch -> create -> merge)
        # 3b. generate acceptance and resolution with these reconstructed ip
        # values
        # (that means simulation + bunching + creating data objects + merging)
        if len(scen.simulation_info_lists) == 0:
            scen.simulation_info_lists.append(['', 'a', 1, 0])
            scen.simulation_info_lists.append(['', 'er', 1, 0])

        scen = simulateDataOnHimster(scen)
        if scen.is_broken:
            dead_scenario_stack.append(scen)
            return

        if len(scen.simulation_info_lists) == 0:
            state += 1
            last_state += 1

    if state == 4:
        # 4. runLmdFit!
        temp_dir_searcher = general.DirectorySearcher(
            ['merge_data', 'binning_300'])
        temp_dir_searcher.searchListOfDirectories(
            scen.filtered_dir_path, 'lmd_fitted_data')
        found_dirs = temp_dir_searcher.getListOfDirectories()
        if not found_dirs:
            os.chdir(lmd_fit_script_path)
            print('running lmdfit!')
            cut_keyword = ''
            if scen.use_xy_cut:
                cut_keyword += 'xy_'
            if scen.use_m_cut:
                cut_keyword += 'm_'
            if cut_keyword == '':
                cut_keyword += 'un'
            cut_keyword += 'cut_real '
            bashcommand = 'python doMultipleLuminosityFits.py '\
                '--forced_box_gen_data ' + scen.acc_and_res_dir_path + \
                ' ' + scen.filtered_dir_path + ' ' + cut_keyword + \
                lmd_fit_path + '/' + args.fit_config + ' ' + Track
            returnvalue = subprocess.call(bashcommand.split())

        print('this scenario is fully processed!!!')
        finished = True

    # if we are in an intermediate step then push on the waiting stack and
    # increase step state
    if not finished:
        scen.state = state
        scen.last_state = last_state
        waiting_scenario_stack.append(scen)


parser = argparse.ArgumentParser(description='Lmd One Button Script',
                                 formatter_class=general.SmartFormatter)

parser.add_argument('--base_output_data_dir', metavar='base_output_data_dir',
                    type=str, default=os.getenv('LMDFIT_DATA_DIR'),
                    help='Base directory for output files created by this'
                    ' script.\n')
parser.add_argument('--fit_config', metavar='fit_config',
                    type=str, default='fitconfig-fast.json')
parser.add_argument('--box_num_events_per_sample',
                    metavar='box_num_events_per_sample',
                    type=int, default=100000,
                    help='number of events per sample to simulate')
parser.add_argument('--box_num_samples', metavar='box_num_samples',
                    type=int, default=500,
                    help='number of samples to simulate')
parser.add_argument('--num_samples', metavar='num_samples',
                    type=int, default=100,
                    help='number of elastic data files to reconstruct'
                    ' (-1 means all)')
parser.add_argument('--bootstrapped_num_samples', type=int, default=1,
                    help='number of elastic data samples to create via'
                    ' bootstrapping (for statistical analysis)')
parser.add_argument('--disable_xy_cut', action='store_true',
                    help='Disable the x-theta & y-phi filter after the '
                    'tracking stage to remove background.')
parser.add_argument('--disable_m_cut', action='store_true',
                    help='Disable the tmva based momentum cut filter after the'
                    ' backtracking stage to remove background.')
parser.add_argument('--disable_ip_determination', action='store_true',
                    help='Disable the determination of the IP. Instead (0,0,0) is assumed')
parser.add_argument('--use_devel_queue', action='store_true',
                    help='If flag is set, the devel queue is used')

args = parser.parse_args()

lmd_fit_script_path = os.path.dirname(os.path.realpath(__file__))
lmd_fit_path = os.path.dirname(lmd_fit_script_path)
lmd_fit_bin_path = os.getenv('LMDFIT_BUILD_PATH') + '/bin'

num_samples = args.num_samples
bootstrapped_num_samples = args.bootstrapped_num_samples
box_num_samples = args.box_num_samples
box_num_events_per_sample = args.box_num_events_per_sample

# first lets try to find all directories and their status/step
dir_searcher = general.DirectorySearcher(['dpm_elastic', 'uncut'])

dir_searcher.searchListOfDirectories(
    args.base_output_data_dir, Track)
dirs = dir_searcher.getListOfDirectories()

print(dirs)

# at first assign each scenario the first step and push on the active stack
for dir in dirs:
    scen = Scenario(dir)
    print("creating scenario:", dir)
    if args.disable_xy_cut or ("/no_alignment_correction" in dir and "no_geo_misalignment/" not in dir):
        print("Disabling xy cut!")
        scen.use_xy_cut = False  # for testing purposes
    if args.disable_m_cut or ("/no_alignment_correction" in dir and "no_geo_misalignment/" not in dir):
        print("Disabling m cut!")
        scen.use_m_cut = False  # for testing purposes
    if args.disable_ip_determination or ("/no_alignment_correction" in dir and "no_geo_misalignment/" not in dir):
        print("Disabling IP determination!")
        scen.use_ip_determination = False
    active_scenario_stack.append(scen)


# now just keep processing the active_stack
while len(active_scenario_stack) > 0 or len(waiting_scenario_stack) > 0:
    for scen in active_scenario_stack:
        lumiDetermination(scen)

    active_scenario_stack = []
    # if all scenarios are currently processed just wait a bit and check again
    if len(waiting_scenario_stack) > 0:
        print('currently waiting for 10min to process scenarios again')
        time.sleep(600)  # wait for 10min
        active_scenario_stack = waiting_scenario_stack
        waiting_scenario_stack = []

# ------------------------------------------------------------------------------------------
