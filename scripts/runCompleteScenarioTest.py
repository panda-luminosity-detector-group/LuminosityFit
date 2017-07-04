# ok this whole procedure is extremely tedious and costly... 
# so dont be scared when you see all this...

import random, os, sys, re, time, glob, errno
lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)
import argparse
import himster
from decimal import *
import subprocess
import json

import general, simulation

#this is kind of bad... but I had to quick fix it... im sorry
gen_lumi_per_event={1.5:171.91969551926493, 4.06:36.382231019899820, 8.9:16.294680278930652, 15.0:10.578901730437558}



class Scenario:   
    def __init__(self, dir_path_):
        self.dir_path = dir_path_
        self.filtered_dir_path = ''
        self.acc_and_res_dir_path = ''
        self.ip_rec_file = ''
    
        self.counter = 1
        self.last_counter = 0
    
        self.simulation_info_lists = []

def wasSimulationSuccessful(directory, glob_pattern):
    required_files_percentage=0.8
    # return values:
    # 0: everything is fine
    # >0: its not finished processing, just keep waiting
    # <0: something went wrong...
    return_value=0 
    
    found_files = glob.glob(directory + '/' + glob_pattern)
    good_files=[]
    bad_files=[]
    for file in found_files:
        if os.stat(file).st_size > 20000:
            good_files.append(file)
        else:
            bad_files.append(file)

    m = re.search('\/(\d+?)-(\d+?)_.+?cut', directory)
    num_sim_files=int(m.group(2))-int(m.group(1)) + 1
    
    if 1.0*len(good_files) < required_files_percentage*num_sim_files:
        print 'WARNING: more than 20% of sim files missing... Something went wrong here...'
        print directory
        # (time.time()-os.path.getmtime('path'))/60/60/24        
        if himster.getNumJobsOnHimster() > 0:
            return_value=1
        else:
            return_value=-1
    
    return return_value
    
# ------------------------------------------------------------------------------------------
# ok we do it in such a way we have a step counter for each directory and two stacks
# we try to process 
active_scenario_stack = []
waiting_scenario_stack = []
dead_scenario_stack = []


def simulateDataOnHimster(sim_info_list, lab_momentum, ip_rec_file=''):
    dir_path = sim_info_list[0]
    type = sim_info_list[1]
    counter = sim_info_list[2]
    last_counter = sim_info_list[3]

    print 'running simulation of type ' + str(type) + ' and path (' + dir_path + ') at counter=' + str(counter) + '/' + str(last_counter)

    data_keywords = []
    data_pattern = ''
    merge_keywords = ['merge_data', 'binning_300']
    if 'v' in type:
        data_keywords = ['uncut', 'bunches', 'binning_300']
        data_pattern = 'lmd_vertex_data_*.root'
    elif 'a' in type:
        data_keywords = ['xy_m_cut_real', 'bunches', 'binning_300']
        data_pattern = 'lmd_data_*.root'
    else:
        data_keywords = ['xy_m_cut_real', 'bunches', 'binning_300']
        data_pattern = 'lmd_res_data_*.root'
    
    # 1. simulate data
    if counter == 1:
        os.chdir(lmd_fit_script_path)
        if 'er' in type:
            found_dirs = []
            status_code=1
            if dir_path != '':
                temp_dir_searcher = general.DirectorySearcher(['box', 'xy_m_cut'])
                temp_dir_searcher.searchListOfDirectories(dir_path, 'Lumi_TrksQA_*.root')
                found_dirs = temp_dir_searcher.getListOfDirectories()

            if found_dirs:
                status_code=wasSimulationSuccessful(found_dirs[0], 'Lumi_TrksQA_*.root')

            elif last_counter < 1:
                # then lets simulate
                file_content = open(ip_rec_file)   
                ip_rec_data = json.load(file_content)

                sim_params = simulation.SimulationParameters()
                sim_params.ip_params.ip_offset_x = float('{0:.3f}'.format(round(float(ip_rec_data["ip_x"]), 3)))  # in cm
                sim_params.ip_params.ip_offset_y = float('{0:.3f}'.format(round(float(ip_rec_data["ip_y"]), 3)))
                sim_params.ip_params.ip_offset_z = float('{0:.3f}'.format(round(float(ip_rec_data["ip_z"]), 3)))
            
                sim_params.num_events = num_events
                sim_params.lab_momentum = lab_momentum
                sim_params.sim_type = 'box'
                sim_params.low_index = low_index
                sim_params.high_index = high_index
                sim_params.use_xy_cut = True
                sim_params.use_m_cut = True
                sim_params.reco_ip_offset = [sim_params.ip_params.ip_offset_x, sim_params.ip_params.ip_offset_y, sim_params.ip_params.ip_offset_z]
                 
                generator_filename_base = simulation.generateGeneratorBaseFilename(sim_params)
                dirname = simulation.generateDirectory(sim_params, generator_filename_base)
                dirname_filter_suffix = simulation.generateFilterSuffix(sim_params)

                dirname_full = dirname + '/' + dirname_filter_suffix
                sim_info_list[0] = os.getenv('DATA_DIR') + '/' + dirname_full
            
                try:
                    os.makedirs(base_out_dir+'/'+dirname)
                except OSError as exception:
                    if exception.errno != errno.EEXIST:
                        print 'warning: thats strange that directory already exists...'
            
                temp_dir_searcher = general.DirectorySearcher(['box', 'xy_m_cut'])
                temp_dir_searcher.searchListOfDirectories(base_out_dir+'/'+dirname, 'Lumi_TrksQA_*.root')
                found_dirs = temp_dir_searcher.getListOfDirectories()
            
                # this command runs the full sim software with box gen data to generate the acceptance and resolution information for this sample
                # note: beam tilt and divergence are not necessary here, because that is handled completely by the model
                if not found_dirs:
                    bashcommand = 'python runSimulations.py --low_index ' + str(low_index) + ' --high_index ' + str(high_index) \
                        + ' --use_ip_offset ' + str(sim_params.ip_params.ip_offset_x) + ' ' + str(sim_params.ip_params.ip_offset_y) + ' ' + str(sim_params.ip_params.ip_offset_z) \
                        + ' ' + str(sim_params.ip_params.ip_spread_x) + ' ' + str(sim_params.ip_params.ip_spread_y) + ' ' + str(sim_params.ip_params.ip_spread_z) \
                        + ' --reco_ip_offset ' + str(sim_params.ip_params.ip_offset_x) +' ' + str(sim_params.ip_params.ip_offset_y) + ' ' + str(sim_params.ip_params.ip_offset_z) \
                        + ' --track_search_algo CA --use_xy_cut --use_m_cut ' + str(num_events) + ' ' + str(lab_momentum) + ' box'
                    returnvalue = subprocess.call(bashcommand.split())
                    last_counter = last_counter + 1
                else:
                    print 'found directory with simulated files, skipping...'
                    dir_path = found_dirs[0]
                    scen.dir_path = found_dirs[0]
                    counter = 2
                    last_counter = 1
            
            if status_code == 0:
                print 'found simulation files, skipping...'
                counter = 2
                last_counter = 1
            elif status_code > 0:
                print 'still waiting for himster simulation jobs for ' + type + ' data to complete...'
            else:
                #ok something went wrong there, exit this scenario and push on bad scenario stack
                last_counter=-1
                
        elif 'a' in type:
            status_code=1
            temp_dir_searcher = general.DirectorySearcher(['dpm_elastic', 'xy_m_cut'])
            temp_dir_searcher.searchListOfDirectories(dir_path, 'Lumi_TrksQA_*.root')
            found_dirs = temp_dir_searcher.getListOfDirectories()
            if found_dirs:
                status_code=wasSimulationSuccessful(found_dirs[0], 'Lumi_TrksQA_*.root')
            elif last_counter < counter:
                # then lets simulate
                file_content = open(ip_rec_file)   
                ip_rec_data = json.load(file_content)
                
                ip_params = simulation.IPParams()
                ip_params.ip_offset_x = float('{0:.3f}'.format(round(float(ip_rec_data["ip_x"]), 3)))  # in cm
                ip_params.ip_offset_y = float('{0:.3f}'.format(round(float(ip_rec_data["ip_y"]), 3)))
                ip_params.ip_offset_z = float('{0:.3f}'.format(round(float(ip_rec_data["ip_z"]), 3)))
                    
                # this command runs the track reco software on the elastic scattering data with the estimated ip position
                # note: beam tilt and divergence are not used here because only the last reco steps are rerun of the track reco software
                bashcommand = 'python runSimulations.py --low_index ' + str(low_index) + ' --high_index ' + str(high_index) \
                    + ' --reco_ip_offset ' + str(ip_params.ip_offset_x) + ' ' + str(ip_params.ip_offset_y) + ' ' + str(ip_params.ip_offset_z) \
                    + ' --output_dir ' + dir_path.replace(base_out_dir + '/', '') \
                    + ' --track_search_algo CA --use_xy_cut --use_m_cut ' + str(num_events) + ' ' + str(lab_momentum) + ' dpm_elastic'
                returnvalue = subprocess.call(bashcommand.split())
                last_counter = last_counter + 1
            

            if status_code == 0:
                print 'found simulation files, skipping...'
                counter = 2
                last_counter = 1
            elif status_code > 0:
                print 'still waiting for himster simulation jobs for ' + type + ' data to complete...'
            else:
                #ok something went wrong there, exit this scenario and push on bad scenario stack
                last_counter=-1
        else:
            # just skip simulation for vertex data... we always have that...
            print 'skipping simulation step...'
            counter = 2
            last_counter = 1 
    # 2. create data (that means bunch data, create data objects)
    if counter == 2:
        # check if data objects already exists and skip!
        temp_dir_searcher = general.DirectorySearcher(data_keywords)
        temp_dir_searcher.searchListOfDirectories(dir_path, data_pattern)
        found_dirs = temp_dir_searcher.getListOfDirectories()
        if found_dirs:
            print 'skipping bunching and data object creation...'
            counter = 3
            last_counter = 2
        elif last_counter < counter:
            os.chdir(lmd_fit_script_path)
            # 1a bunch data
            bashcommand = 'python makeMultipleFileListBunches.py --files_per_bunch 10 ' + dir_path
            returnvalue = subprocess.call(bashcommand.split())
            # 1b create data
            if 'a' in type:
                ref_mom, gen_lumi = min(gen_lumi_per_event.items(), key=lambda(k,v):abs(k-lab_momentum))
		bashcommand = 'python createMultipleLmdData.py --elastic_cross_section ' + str(gen_lumi) + ' --dir_pattern ' + data_keywords[0] + ' ' + str(lab_momentum) + ' ' + type + ' ' + dir_path + ' ../dataconfig_xy.json'
            else:    
		bashcommand = 'python createMultipleLmdData.py --dir_pattern ' + data_keywords[0] + ' ' + str(lab_momentum) + ' ' + type + ' ' + dir_path + ' ../dataconfig_xy.json'
            returnvalue = subprocess.call(bashcommand.split())
            last_counter = last_counter + 1
        else:
            print 'still waiting for himster data creation jobs for ' + type + ' data to complete...'
    
    # 3. merge data
    if counter == 3:
        # check first if merged data already exists and skip it!
        temp_dir_searcher = general.DirectorySearcher(merge_keywords)
        temp_dir_searcher.searchListOfDirectories(dir_path, data_pattern)
        found_dirs = temp_dir_searcher.getListOfDirectories()
        if not found_dirs:
            os.chdir(lmd_fit_script_path)
            # 1c merge vertex data
            bashcommand = 'python mergeMultipleLmdData.py --dir_pattern ' + data_keywords[0] + ' ' + type + ' ' + dir_path
            returnvalue = subprocess.call(bashcommand.split())
        counter = -1
    
    sim_info_list[2] = counter
    sim_info_list[3] = last_counter
    
    return sim_info_list
        
        
        
        
def lumiDetermination(scen):
    dir_path = scen.dir_path

    counter = scen.counter
    last_counter = scen.last_counter
    
    print 'processing scenario ' + dir_path + ' at step ' + str(counter)
    
    m = re.search('(\d*?\.\d*?)GeV', dir_path)
    momentum = float(m.group(1))
    print momentum
    
    finished = False
    # 1. create vertex data (that means bunch data, create data objects and merge)
    if counter == 1:
        if len(scen.simulation_info_lists) == 0:
            scen.simulation_info_lists.append([dir_path, 'v', 1, 0])
        
        scen.simulation_info_lists[0] = simulateDataOnHimster(scen.simulation_info_lists[0], momentum)
        if scen.simulation_info_lists[0][2] == -1:
            counter = counter + 1
            last_counter = last_counter + 1
            scen.simulation_info_lists = []
        elif scen.simulation_info_lists[0][3] == -1:
            dead_scenario_stack.append(scen)
            return

    if counter == 2:
        # check if ip was already determined
        temp_dir_searcher = general.DirectorySearcher(['merge_data', 'binning_300'])
        temp_dir_searcher.searchListOfDirectories(dir_path, 'reco_ip.json')
        found_dirs = temp_dir_searcher.getListOfDirectories()
        if not found_dirs:
            # 2. determine offset on the vertex data sample
            os.chdir(lmd_fit_bin_path)
            temp_dir_searcher = general.DirectorySearcher(['merge_data', 'binning_300'])
            temp_dir_searcher.searchListOfDirectories(dir_path, 'lmd_vertex_data_*of1.root')
            found_dirs = temp_dir_searcher.getListOfDirectories()
            bashcommand = './determineBeamOffset -p ' + found_dirs[0] + ' -c ' + '../../vertex_fitconfig.json'
            returnvalue = subprocess.call(bashcommand.split())
            scen.ip_rec_file = found_dirs[0] + '/reco_ip.json'
        else:
            scen.ip_rec_file = found_dirs[0] + '/reco_ip.json'
        counter = counter + 1
        last_counter = last_counter + 1 
            
    if counter == 3:
        # 3a. track filter the dpm data using the ip values and create ang dist objects
        # (that again means bunch -> create -> merge)
        # 3b. generate acceptance and resolution with these reconstructed ip values
        # (that means simulation + bunching + creating data objects + merging....)
        if len(scen.simulation_info_lists) == 0:
            scen.filtered_dir_path = os.path.dirname(scen.dir_path)
            scen.simulation_info_lists.append([scen.filtered_dir_path, 'a', 1, 0])
            if not os.path.exists(scen.filtered_dir_path):
                os.makedirs(scen.filtered_dir_path)
            scen.simulation_info_lists.append(['', 'er', 1, 0])
        
        temp_sim_lists = scen.simulation_info_lists
        scen.simulation_info_lists = []
        for sim in temp_sim_lists:
            temp_sim = simulateDataOnHimster(sim, momentum, scen.ip_rec_file)
            if 'er' in temp_sim[1]:
                scen.acc_and_res_dir_path = temp_sim[0]
            if temp_sim[2] != -1:
               scen.simulation_info_lists.append(temp_sim)
            if temp_sim[3] == -1:
                dead_scenario_stack.append(scen)
                return
        
        if len(scen.simulation_info_lists) == 0:
                counter = counter + 1
                last_counter = last_counter + 1
            
    if counter == 4:    
        # 4. runLmdFit!
        temp_dir_searcher = general.DirectorySearcher(['merge_data', 'binning_300'])
        temp_dir_searcher.searchListOfDirectories(scen.filtered_dir_path, 'lmd_fitted_data*.root')
        found_dirs = temp_dir_searcher.getListOfDirectories()
        if not found_dirs:
            os.chdir(lmd_fit_script_path)
            print 'running lmdfit!'
            bashcommand = 'python doMultipleLuminosityFits.py --forced_box_gen_data ' + scen.acc_and_res_dir_path + ' ' + scen.filtered_dir_path + ' xy_m_cut_real ' + lmd_fit_path+'/fitconfig.json'
            returnvalue = subprocess.call(bashcommand.split())        
        
        print 'this scenario is fully processed!!!'
        finished = True

    # if we are in an intermediate step then push on the waiting stack and increase step counter
    if not finished:
        scen.counter = counter
        scen.last_counter = last_counter
        waiting_scenario_stack.append(scen)
        


parser = argparse.ArgumentParser(description='Script for realistic full PANDA Luminosity Detector simulations.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('--output_data_dir', metavar='output_data_dir', type=str, nargs=1,
                   help='Base directory for output files created by this script.')
parser.add_argument('--luminosity_fit_dir', metavar='luminosity_fit_dir', type=str, nargs=1,
                   help='Base directory of the luminosity fit software.')


parser.add_argument('num_events', metavar='num_events', type=int, default=100000, help='number of events to simulate')
parser.add_argument('--low_index', metavar='low_index', type=int, default=1,
                   help='Lowest index of generator file which is supposed to be used in the simulation. Default setting is -1 which will take the lowest found index.')
parser.add_argument('--high_index', metavar='high_index', type=int, default=100,
                   help='Highest index of generator file which is supposed to be used in the simulation. Default setting is -1 which will take the highest found index.')


base_out_dir = args.output_data_dir[0]
os.environ["DATA_DIR"] = base_out_dir

lmd_fit_path=args.luminosity_fit_dir[0]
lmd_fit_script_path = lmd_fit_path+'/scripts'
lmd_fit_bin_path = lmd_fit_path+'/build/bin'

low_index = Integer(args.low_index)
high_index = Integer(args.high_index)
num_events = Integer(args.num_events)

        
# first lets try to find all directories and their status/step
dir_searcher = general.DirectorySearcher(['dpm_elastic', 'uncut'])

dir_searcher.searchListOfDirectories(base_out_dir, 'Lumi_TrksQA_*.root')
dirs = dir_searcher.getListOfDirectories()

print dirs

# at first assign each scenario the first step and push on the active stack
for dir in dirs:
    scen = Scenario(dir)
    active_scenario_stack.append(scen)


# now just keep processing the active_stack
while len(active_scenario_stack) > 0 or len(waiting_scenario_stack) > 0:
    for scen in active_scenario_stack:
        lumiDetermination(scen)

    active_scenario_stack = []
    # if all scenarios are currently processed just wait a bit and check again
    if len(waiting_scenario_stack) > 0:
        print 'currently waiting for 10min to process scenarios again'
        time.sleep(600)  # wait for 5min
        active_scenario_stack = waiting_scenario_stack    
        waiting_scenario_stack = []

# ------------------------------------------------------------------------------------------

