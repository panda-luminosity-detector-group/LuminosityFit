#!/usr/bin/python

import himster
import os, sys, re, errno

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

low_index_used = -1
high_index_used = -1

def getGeneratedDataDirectory():  
  found_dir_dict = dict()
  
  for gen_dir_name in os.listdir(gen_data_dir_path):
    match_data_type_and_momentum = re.search(args.sim_type[0]+'_plab_'+str(args.lab_momentum[0])+'GeV', gen_dir_name)
    if not match_data_type_and_momentum:
      continue
    
    match_num_events = re.search('^(\d*)_', gen_dir_name)
    if match_num_events:
      if args.num_events[0] <= int(match_num_events.group(1)):
        found_dir_dict[gen_dir_name] = int(match_num_events.group(1))
   
  # filter for possible index ranges
  found_dir_dict_temp = found_dir_dict.copy()
  found_dir_dict.clear()
  for (dirname, num_events) in found_dir_dict_temp.items():
    found_index_range = checkIndexRangeForGeneratedDataDirectory(gen_data_dir_path+'/'+dirname)
    used_start_index = args.low_index
    used_end_index = args.high_index
    if(args.low_index == -1):
      used_start_index = found_index_range[0]
    if(args.high_index == -1):
      used_end_index = found_index_range[1]
    if found_index_range[0] <= used_start_index and used_end_index <= found_index_range[1]:
      found_dir_dict[dirname] = [num_events, used_start_index, used_end_index]
  
  return_result = ''
  
  if len(found_dir_dict) == 0:
    print 'Found no suitable generated data directory in ' + gen_data_dir_path + ' .'\
          + 'Please specify another path or generate the needed data first!'
    sys.exit(1)
  if len(found_dir_dict) > 1:
    print 'Found more than one suitable generated data directory in ' + gen_data_dir_path + ' .'\
          +'List of possibilities is shown below:'
    print found_dir_dict
    return_result = ''
    while return_result not in found_dir_dict.keys():
      return_result = raw_input('Please enter the dirname of one of the found generator directory names (sorted by most possible user request): ')
  else:
    return_result = found_dir_dict.keys()[0]

  global low_index_used
  global high_index_used
  low_index_used = found_dir_dict[return_result][1]
  high_index_used = found_dir_dict[return_result][2]

  return return_result
  
def checkIndexRangeForGeneratedDataDirectory(dir_path):
  # check for the index range in the specified generator folder
  first = 1
  lowest_index = -1
  highest_index = -1

  dircontent = os.listdir(dir_path)

  for file in dircontent:
    result = re.search('_(\d*).root$', file)
    if result:
      if first:
        lowest_index = int(result.group(1))
        highest_index = int(result.group(1))
        first = 0
      else:
        if int(result.group(1)) < lowest_index:
          lowest_index = int(result.group(1))
        elif int(result.group(1)) > highest_index:
          highest_index = int(result.group(1))

  return [lowest_index, highest_index]
    
def generateSimulationParameterPropertyFile(output_dirname):
  f = open(output_dirname + '/sim_params.config', 'w')
  f.write('ip_mean_x=' + str(args.use_ip_offset[0]))
  f.write('ip_mean_y=' + str(args.use_ip_offset[1]))
  f.write('ip_mean_z=' + str(args.use_ip_offset[2]))
  
  f.write('ip_standard_deviation_x=' + str(args.use_ip_offset[3]))
  f.write('ip_standard_deviation_y=' + str(args.use_ip_offset[4]))
  f.write('ip_standard_deviation_z=' + str(args.use_ip_offset[5]))
  
  f.write('beam_tilt_x=' + str(args.use_beam_gradient[0]))
  f.write('beam_tilt_y=' + str(args.use_beam_gradient[1]))
  f.write('beam_divergence_x=' + str(args.use_beam_gradient[2]))
  f.write('beam_divergence_y=' + str(args.use_beam_gradient[3]))

  f.close()


parser = argparse.ArgumentParser(description='Script for full simulation of PANDA Luminosity Detector via externally generated MC data.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('num_events', metavar='num_events', type=int, nargs=1, help='number of events to simulate')
parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('sim_type', metavar='simulation_type', type=str, nargs=1, choices=['box', 'dpm_elastic', 'dpm_elastic_inelastic', 'noise'], 
                    help='Simulation type which can be one of the following: box, dpm_elastic, dpm_elastic_inelastic, noise.\n'
                        'This information is used to automatically obtain the generator data and output naming scheme.')

parser.add_argument('--gen_data_dirname', metavar='gen_data_dirname', type=str, default='',
                    help='Name of directory containing the generator data that is used as input.\n'
                    'Note that this is only the name of the directory and NOT the full path.\n'
                    'The base path of the directory should be specified with the\n'
                    '--gen_data_dir flag. Default will be either an empty string for direct simulations\n'
                    'or the same generated name as for the generated data, based on the simulation type')

parser.add_argument('--low_index', metavar='low_index', type=int, default= -1,
                   help='Lowest index of generator file which is supposed to be used in the simulation.\n'
                   'Default setting is -1 which will take the lowest found index.')
parser.add_argument('--high_index', metavar='high_index', type=int, default= -1,
                   help='Highest index of generator file which is supposed to be used in the simulation.\n'
                   'Default setting is -1 which will take the highest found index.')

parser.add_argument('--gen_data_dir', metavar='gen_data_dir', type=str, default=os.getenv('GEN_DATA'),
                   help='Base directory to input files created by external generator.\n'
                   'By default the environment variable $GEN_DATA will be used!')

parser.add_argument('--output_dir', metavar='output_dir', type=str, default='', help='This directory is used for the output.\n'
                    'Default is the generator directory as a prefix, with beam offset infos etc. added')

parser.add_argument('--use_ip_offset', metavar=("ip_offset_x", "ip_offset_y", "ip_offset_z", "ip_spread_x", "ip_spread_y", "ip_spread_z"), type=float, nargs=6, default=[0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                   help="ip_offset_x: interaction vertex mean X position (in cm)\n"
            "ip_offset_y: interaction vertex mean Y position (in cm)\n"
            "ip_offset_z: interaction vertex mean Z position (in cm)\n"
            "ip_spread_x: interaction vertex X position distribution width (in cm)\n"
            "ip_spread_y: interaction vertex Y position distribution width (in cm)\n"
            "ip_spread_z: interaction vertex Z position distribution width (in cm)")

parser.add_argument('--use_beam_gradient', metavar=("beam_gradient_x", "beam_gradient_y", "beam_emittance_x", "beam_emittance_y"), type=float, nargs=4, default=[0.0, 0.0, 0.0, 0.0],
                   help="beam_gradient_x: mean beam inclination on target in x direction dPx/dPz (in rad)\n"
            "beam_gradient_y: mean beam inclination on target in y direction dPy/dPz (in rad)\n"
            "beam_divergence_x: beam divergence in x direction (in rad)\n"
            "beam_divergence_y: beam divergence in y direction (in rad)")

parser.add_argument('--use_xy_cut', action='store_true', help='Use the x-theta & y-phi filter after the tracking stage to remove background.')
parser.add_argument('--use_m_cut', action='store_true', help='Use the tmva based momentum cut filter after the backtracking stage to remove background.')

parser.add_argument('--track_search_algo', metavar='track_search_algorithm', type=str, choices=['CA', 'Follow'], default='CA', help='Track Search algorithm to be used.')


parser.add_argument('--reco_ip_offset', metavar=("rec_ip_offset_x", "rec_ip_offset_y", "rec_ip_spread_x", "rec_ip_spread_y"), type=float, nargs=4, default=[0.0, 0.0, -1.0, -1.0],
                   help="rec_ip_offset_x: interaction vertex mean X position (in cm)\n"
            "rec_ip_offset_y: interaction vertex mean Y position (in cm)\n"
            "rec_ip_spread_x: interaction vertex X position distribution width (in cm)\n"
            "rec_ip_spread_y: interaction vertex Y position distribution width (in cm)\n")

args = parser.parse_args()

gen_data_dir_path = args.gen_data_dir
while not os.path.isdir(gen_data_dir_path):
    gen_data_dir_path = raw_input('Please enter valid generator base path: ')
  
# generator file prefix
if args.gen_data_dirname != '':
  generator_filename_base = args.gen_data_dirname[0]
else:
  if args.sim_type[0] != 'noise':
    generator_filename_base = getGeneratedDataDirectory()
  else:
    if args.low_index == -1 or args.high_index == -1:
      print 'Please specify job array boundaries for noise simulations via --low_index and --high_index since the data will be generated here directly!'
      sys.exit(1)
    else:
      low_index_used = args.low_index
      high_index_used = args.high_index
    generator_filename_base = str(args.num_events[0]) + '_noise_plab_' + str(args.lab_momentum[0]) + 'GeV'

filename_base = re.sub('\.', 'o', generator_filename_base)

print 'using generator dir: ' + generator_filename_base
print 'preparing simulations in index range ' + str(low_index_used) + ' - ' + str(high_index_used)

if args.output_dir == '':
  # generate output directory name
  # lets generate a folder structure based on the input
  dirname = 'plab_' + str(args.lab_momentum[0]) + 'GeV'
  
  gen_part = re.sub('_plab_.*GeV', '', generator_filename_base) 
  gen_part = re.sub('^\d*_', '', gen_part)
  
  dirname += '/' + gen_part
  
  dirname += '/ip_offset_XYZDXDYDZ'
  for val in args.use_ip_offset:
    dirname = dirname + '_' + str(val)
  dirname += '/beam_grad_XYDXDY'
  for val in args.use_beam_gradient:
    dirname = dirname + '_' + str(val)
  dirname += '/' + str(args.num_events[0]) 
  
  dirname_filter_suffix = str(args.low_index) + '-' + str(args.high_index) + '_'
  if args.use_xy_cut:
    dirname_filter_suffix += 'xy_'
  if args.use_m_cut:
    dirname_filter_suffix += 'm_'
  if not args.use_xy_cut and not args.use_m_cut:
    dirname_filter_suffix += 'un'
  dirname_filter_suffix += 'cut'
  if args.use_xy_cut:
    if args.reco_ip_offset[2] >= 0.0 and args.reco_ip_offset[3] >= 0.0:
      dirname_filter_suffix += '_real'
else:
  dirname = args.output_dir

pathname_base = os.getenv('DATA_DIR') + '/' + dirname
path_mc_data = pathname_base + '/mc_data'
dirname_full = dirname + '/' + dirname_filter_suffix
pathname_full = os.getenv('DATA_DIR') + '/' + dirname_full

print 'using output folder structure: ' + dirname_full

try:
    os.makedirs(pathname_full)
    os.makedirs(path_mc_data)
except OSError as exception:
    if exception.errno != errno.EEXIST:
        print 'error: thought dir does not exists but it does...'

# generate simulation config parameter file
generateSimulationParameterPropertyFile(pathname_base)

joblist = []

resource_request = himster.JobResourceRequest(20*60)
resource_request.number_of_nodes=1
resource_request.processors_per_node=1
resource_request.memory_in_mb=2000
resource_request.virtual_memory_in_mb=2000
resource_request.node_scratch_filesize_in_mb = 3000
job = himster.Job(resource_request, './runLumiFullSimPixel.sh', 'lmd_fullsim_' + args.sim_type[0], pathname_full + '/sim.log')
job.setJobArraySize(low_index_used, high_index_used)
  
job.addExportedUserVariable('num_evts', str(args.num_events[0]))
job.addExportedUserVariable('mom', str(args.lab_momentum[0]))
job.addExportedUserVariable('gen_input_file_stripped', args.gen_data_dir + '/' + generator_filename_base + '/' + filename_base)
job.addExportedUserVariable('dirname', dirname_full)
job.addExportedUserVariable('path_mc_data', path_mc_data)
job.addExportedUserVariable('pathname', pathname_full)
job.addExportedUserVariable('beamX0', str(args.use_ip_offset[0]))
job.addExportedUserVariable('beamY0', str(args.use_ip_offset[1]))
job.addExportedUserVariable('targetZ0', str(args.use_ip_offset[2]))
job.addExportedUserVariable('beam_widthX', str(args.use_ip_offset[3]))
job.addExportedUserVariable('beam_widthY', str(args.use_ip_offset[4]))
job.addExportedUserVariable('target_widthZ', str(args.use_ip_offset[5]))
job.addExportedUserVariable('beam_gradX', str(args.use_beam_gradient[0]))
job.addExportedUserVariable('beam_gradY', str(args.use_beam_gradient[1]))
job.addExportedUserVariable('beam_grad_sigmaX', str(args.use_beam_gradient[2]))
job.addExportedUserVariable('beam_grad_sigmaY', str(args.use_beam_gradient[3]))
job.addExportedUserVariable('SkipFilt', str(not args.use_xy_cut).lower())
job.addExportedUserVariable('XThetaCut', str(args.use_xy_cut).lower())
job.addExportedUserVariable('YPhiCut', str(args.use_xy_cut).lower())
job.addExportedUserVariable('CleanSig', str(args.use_m_cut).lower())
job.addExportedUserVariable('track_search_algorithm', args.track_search_algo)
if args.sim_type[0] == 'noise':
  job.addExportedUserVariable('simulate_noise', '1')
job.addExportedUserVariable('rec_targetZ0', '0.0')
  
if args.use_xy_cut or args.use_m_cut:
  if args.reco_ip_offset[2] >= 0.0 and args.reco_ip_offset[3] >= 0.0:
    job.addExportedUserVariable('rec_beamX0', str(args.reco_ip_offset[0]))
    job.addExportedUserVariable('rec_beamY0', str(args.reco_ip_offset[1]))
  else:
    job.addExportedUserVariable('rec_beamX0', str(args.use_ip_offset[0]))
    job.addExportedUserVariable('rec_beamY0', str(args.use_ip_offset[1]))

joblist.append(job)

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = himster.HimsterJobManager(1600)

job_manager.submitJobsToHimster(joblist)
job_manager.manageJobs()
