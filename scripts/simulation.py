import os, sys, re
from decimal import *

def check_ip_params_zero(ip_params):
  print("check zero...")
  for key, value in ip_params.items():
    if value != Decimal('0.0'):
      return False
  return True

def generateSimulationParameters(args):
    sim_params = {
        'num_events_per_sample': args.num_events_per_sample[0],
        'num_samples': args.num_samples[0],
        'lab_momentum': args.lab_momentum[0],
        'sim_type': args.sim_type[0],
        'theta_min_in_mrad': args.theta_min,
        'theta_max_in_mrad': args.theta_max,
        'neglect_recoil_momentum': not args.neglect_recoil_momentum,
        'random_seed': args.random_seed,
        'low_index': args.low_index,
        'output_dir': args.output_dir,
        'use_xy_cut': args.use_xy_cut,
        'use_m_cut': args.use_m_cut,
        'track_search_algo': args.track_search_algo, 
        'reco_ip_offset': args.reco_ip_offset,
        'lmd_geometry_filename': args.lmd_detector_geometry_filename,
        'misalignment_matrices_path': args.misalignment_matrices_path,
        'alignment_matrices_path': args.alignment_matrices_path
    }
    if args.debug and sim_params['num_samples'] > 1:
        print("Warning: number of samples in debug mode is limited to 1! Setting to 1!")
        sim_params['num_samples'] = 1
    
    ip_params = {
        'ip_offset_x': Decimal(args.use_ip_offset[0]),  # in cm
        'ip_offset_y': Decimal(args.use_ip_offset[1]),  # in cm
        'ip_offset_z': Decimal(args.use_ip_offset[2]),  # in cm
        'ip_spread_x': Decimal(args.use_ip_offset[3]),  # in cm
        'ip_spread_y': Decimal(args.use_ip_offset[4]),  # in cm
        'ip_spread_z': Decimal(args.use_ip_offset[5]),  # in cm
  
        'beam_tilt_x': Decimal(args.use_beam_gradient[0]),  # in rad
        'beam_tilt_y': Decimal(args.use_beam_gradient[1]),  # in rad
        'beam_divergence_x': Decimal(args.use_beam_gradient[2]),  # in rad
        'beam_divergence_y': Decimal(args.use_beam_gradient[3])  # in rad
    }
    sim_params['ip_params'] = ip_params
     
    return sim_params


def generateSimulationParameterPropertyFile(output_dirname, sim_params):
  config_fileurl = output_dirname + '/sim_params.config'
  if not os.path.isfile(config_fileurl):
    f = open(config_fileurl, 'w')
    for k,v in sim_params.items():
      f.write(k + '=' + str(v) + '\n')
    f.close()


def generateDirectory(sim_params):
  print('preparing simulations in index range ' + str(sim_params['low_index']) 
        + ' - ' + str(sim_params['low_index']+sim_params['num_samples']-1))

  if sim_params['output_dir'] == '':
    # generate output directory name
    # lets generate a folder structure based on the input
    dirname = 'plab_' + str(sim_params['lab_momentum']) + 'GeV'
    gen_part = sim_params['sim_type'] + '_theta_' + str(sim_params['theta_min_in_mrad'])
    #if sim_params['sim_type'] is 'box':
    gen_part += '-' + str(sim_params['theta_max_in_mrad'])
    gen_part += 'mrad'
    if not sim_params['neglect_recoil_momentum']:
        gen_part += '_recoil_corrected'
  
    dirname += '/' + gen_part
  
    if not check_ip_params_zero(sim_params['ip_params']):
        dirname += '/ip_offset_XYZDXDYDZ_'+str(sim_params['ip_params']['ip_offset_x'])+'_'\
            +str(sim_params['ip_params']['ip_offset_y'])+'_'\
            +str(sim_params['ip_params']['ip_offset_z'])+'_'+str(sim_params['ip_params']['ip_spread_x'])+'_'\
            +str(sim_params['ip_params']['ip_spread_y'])+'_'+str(sim_params['ip_params']['ip_spread_z'])

        dirname += '/beam_grad_XYDXDY_'+str(sim_params['ip_params']['beam_tilt_x'])+'_'\
            +str(sim_params['ip_params']['beam_tilt_y'])+'_'\
            +str(sim_params['ip_params']['beam_divergence_x'])+'_'+str(sim_params['ip_params']['beam_divergence_y'])

    #dirname += '/' + str(os.path.splitext(sim_params['lmd_geometry_filename'])[0])
    if sim_params['misalignment_matrices_path'] == '':
        dirname += '/aligned'
    else:
        dirname += '/' + str(os.path.basename(sim_params['misalignment_matrices_path']))

    dirname += '/' + str(sim_params['num_events_per_sample'])
  else:
    dirname = sim_params['output_dir']
    
  return dirname


def generateFilterSuffix(sim_params):
  dirname_filter_suffix = str(sim_params['low_index']) + '-' + str(sim_params['low_index']+sim_params['num_samples']-1) + '_'
  if sim_params['use_xy_cut']:
    dirname_filter_suffix += 'xy_'
  if sim_params['use_m_cut']:
    dirname_filter_suffix += 'm_'
  if not sim_params['use_xy_cut'] and not sim_params['use_m_cut']:
    dirname_filter_suffix += 'un'
  dirname_filter_suffix += 'cut'
  if sim_params['use_xy_cut']:
    if (sim_params['reco_ip_offset'][0] != 0.0 
        or sim_params['reco_ip_offset'][1] != 0.0 
        or sim_params['reco_ip_offset'][2] != 0.0):
      dirname_filter_suffix += '_real'

  return dirname_filter_suffix
