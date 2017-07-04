import os, sys, re
from decimal import *

class IPParams:
  ip_offset_x = Decimal('0.0')  # in cm
  ip_offset_y = Decimal('0.0')  # in cm
  ip_offset_z = Decimal('0.0')  # in cm
  ip_spread_x = Decimal('0.08')  # in cm
  ip_spread_y = Decimal('0.08')  # in cm
  ip_spread_z = Decimal('0.35')  # in cm
  
  beam_tilt_x = Decimal('0.0')  # in rad
  beam_tilt_y = Decimal('0.0')  # in rad
  beam_divergence_x = Decimal('0.0')  # in rad
  beam_divergence_y = Decimal('0.0')  # in rad

  def __repr__(self):
    return 'IP center: [' + str(self.ip_offset_x) + ',' + str(self.ip_offset_y) + ',' + str(self.ip_offset_z) \
           + '] IP spread: [' + str(self.ip_spread_x) + ',' + str(self.ip_spread_y) + ',' + str(self.ip_spread_z) \
           + '] Tilt: [' + str(self.beam_tilt_x) + ',' + str(self.beam_tilt_y) \
           + '] Divergence: [' + str(self.beam_divergence_x) + ',' + str(self.beam_divergence_y) + ']\n'
  def __str__(self):
    return self.__repr__()

class SimulationParameters:
    ip_params = IPParams()
    
    num_events = 0
    lab_momentum = 0.0
    sim_type = ''
    force_level = 0
    gen_data_dirname = ''
    low_index=0
    high_index=0
    gen_data_dir=os.getenv('GEN_DATA')
    output_dir=''
    use_xy_cut=False
    use_m_cut=False
    track_search_algo='CA'
    reco_ip_offset=[]
    lmd_geometry_filename='Luminosity-Detector.root'

def generateSimulationParameterPropertyFile(output_dirname, sim_params):
  config_fileurl = output_dirname + '/sim_params.config'
  if not os.path.isfile(config_fileurl):
    f = open(config_fileurl, 'w')
    f.write('ip_mean_x=' + str(sim_params.ip_params.ip_offset_x) + '\n')
    f.write('ip_mean_y=' + str(sim_params.ip_params.ip_offset_y) + '\n')
    f.write('ip_mean_z=' + str(sim_params.ip_params.ip_offset_z) + '\n')
  
    f.write('ip_standard_deviation_x=' + str(sim_params.ip_params.ip_spread_x) + '\n')
    f.write('ip_standard_deviation_y=' + str(sim_params.ip_params.ip_spread_y) + '\n')
    f.write('ip_standard_deviation_z=' + str(sim_params.ip_params.ip_spread_z) + '\n')
  
    f.write('beam_tilt_x=' + str(sim_params.ip_params.beam_tilt_x) + '\n')
    f.write('beam_tilt_y=' + str(sim_params.ip_params.beam_tilt_y) + '\n')
    f.write('beam_divergence_x=' + str(sim_params.ip_params.beam_divergence_x) + '\n')
    f.write('beam_divergence_y=' + str(sim_params.ip_params.beam_divergence_y) + '\n')

    f.close()

def generateGeneratorBaseFilename(sim_params):
  # generator file prefix
  if sim_params.gen_data_dirname != '':
    generator_filename_base = sim_params.gen_data_dirname
  else:
    if sim_params.sim_type != 'noise':
      generator_filename_base = getGeneratedDataDirectory(sim_params)
    else:
      if sim_params.low_index == -1 or sim_params.high_index == -1:
        print 'Please specify job array boundaries for noise simulations via --low_index and --high_index since the data will be generated here directly!'
        sys.exit(1)
      else:
        low_index_used = sim_params.low_index
        high_index_used = sim_params.high_index
      generator_filename_base = str(sim_params.num_events) + '_noise_plab_' + str(sim_params.lab_momentum) + 'GeV'

  print 'using generator dir: ' + generator_filename_base
  return generator_filename_base

def generateDirectory(sim_params, generator_filename_base):
  print 'preparing simulations in index range ' + str(sim_params.low_index) + ' - ' + str(sim_params.high_index)

  if sim_params.output_dir == '':
    # generate output directory name
    # lets generate a folder structure based on the input
    dirname = 'plab_' + str(sim_params.lab_momentum) + 'GeV'
  
    gen_part = re.sub('_plab_.*GeV', '', generator_filename_base) 
    gen_part = re.sub('^\d*_', '', gen_part)
  
    dirname += '/' + gen_part
  
    dirname += '/ip_offset_XYZDXDYDZ_'+str(sim_params.ip_params.ip_offset_x)+'_'+str(sim_params.ip_params.ip_offset_y)+'_'\
        +str(sim_params.ip_params.ip_offset_z)+'_'+str(sim_params.ip_params.ip_spread_x)+'_'\
        +str(sim_params.ip_params.ip_spread_y)+'_'+str(sim_params.ip_params.ip_spread_z)

    dirname += '/beam_grad_XYDXDY_'+str(sim_params.ip_params.beam_tilt_x)+'_'+str(sim_params.ip_params.beam_tilt_y)+'_'\
        +str(sim_params.ip_params.beam_divergence_x)+'_'+str(sim_params.ip_params.beam_divergence_y)

    dirname += '/' + str(os.path.splitext(sim_params.lmd_geometry_filename)[0])

    dirname += '/' + str(sim_params.num_events)
  else:
    dirname = sim_params.output_dir
    
  return dirname
  
def generateFilterSuffix(sim_params):
  dirname_filter_suffix = str(sim_params.low_index) + '-' + str(sim_params.high_index) + '_'
  if sim_params.use_xy_cut:
    dirname_filter_suffix += 'xy_'
  if sim_params.use_m_cut:
    dirname_filter_suffix += 'm_'
  if not sim_params.use_xy_cut and not sim_params.use_m_cut:
    dirname_filter_suffix += 'un'
  dirname_filter_suffix += 'cut'
  if sim_params.use_xy_cut:
    if sim_params.reco_ip_offset[0] != 0.0 or sim_params.reco_ip_offset[1] != 0.0 or sim_params.reco_ip_offset[2] != 0.0:
      dirname_filter_suffix += '_real'

  return dirname_filter_suffix

  
def getGeneratedDataDirectory(sim_params):  
  found_dir_dict = dict()
  
  gen_data_dir_path = sim_params.gen_data_dir

  while not os.path.isdir(gen_data_dir_path):
    gen_data_dir_path = raw_input('Please enter valid generator base path: ')
  
  for gen_dir_name in os.listdir(gen_data_dir_path):
    match_data_type_and_momentum = re.search(sim_params.sim_type + '_plab_' + str(sim_params.lab_momentum) + 'GeV', gen_dir_name)
    if not match_data_type_and_momentum:
      continue
    
    match_num_events = re.search('^(\d*)_', gen_dir_name)
    if match_num_events:
      if sim_params.num_events <= int(match_num_events.group(1)):
        found_dir_dict[gen_dir_name] = int(match_num_events.group(1))
   
  # filter for possible index ranges
  found_dir_dict_temp = found_dir_dict.copy()
  found_dir_dict.clear()
  for (dirname, num_events) in found_dir_dict_temp.items():
    found_index_range = checkIndexRangeForGeneratedDataDirectory(gen_data_dir_path + '/' + dirname)
    used_start_index = sim_params.low_index
    used_end_index = sim_params.high_index
    if(sim_params.low_index == -1):
      used_start_index = found_index_range[0]
    if(sim_params.high_index == -1):
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
          + 'List of possibilities is shown below:'
    print found_dir_dict
    return_result = ''
    while return_result not in found_dir_dict.keys():
      return_result = raw_input('Please enter the dirname of one of the found generator directory names (sorted by most possible user request): ')
  else:
    return_result = found_dir_dict.keys()[0]

  sim_params.low_index = found_dir_dict[return_result][1]
  sim_params.high_index = found_dir_dict[return_result][2]

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