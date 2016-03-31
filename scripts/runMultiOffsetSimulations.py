#!/usr/bin/python

import os, sys, re, errno, glob, time, copy
import subprocess
import multiprocessing

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse


class IPParams:
  ip_offset_x = 0.0  # in cm
  ip_offset_y = 0.0  # in cm
  ip_offset_z = 0.0  # in cm
  ip_spread_x = 0.08  # in cm
  ip_spread_y = 0.08  # in cm
  ip_spread_z = 0.35  # in cm
  
  beam_tilt_x = 0.0  # in mrad
  beam_tilt_y = 0.0  # in mrad
  beam_divergence_x = 0.0  # in mrad
  beam_divergence_y = 0.0  # in mrad
  
  def setIPOffsetXYZ(self, ip_offset_x_, ip_offset_y_, ip_offset_z_):
    self.ip_offset_x = ip_offset_x_
    self.ip_offset_y = ip_offset_y_
    self.ip_offset_z = ip_offset_z_
    
  def setIPSpreadXYZ(self, ip_spread_x_, ip_spread_y_, ip_spread_z_):
    self.ip_spread_x = ip_spread_x_
    self.ip_spread_y = ip_spread_y_
    self.ip_spread_z = ip_spread_z_
    
  def setBeamTiltXY(self, beam_tilt_x_, beam_tilt_y_):
    self.beam_tilt_x = beam_tilt_x_
    self.beam_tilt_y = beam_tilt_y_
    
  def setBeamDivergenceXY(self, beam_divergence_x_, beam_divergence_y_):
    self.beam_divergence_x = beam_divergence_x_
    self.beam_divergence_y = beam_divergence_y_

  def __repr__(self):
    return 'IP center: [' + str(self.ip_offset_x) + ',' + str(self.ip_offset_y) + ',' + str(self.ip_offset_z) \
           + '] IP spread: [' + str(self.ip_spread_x) + ',' + str(self.ip_spread_y) + ',' + str(self.ip_spread_z) \
           + '] Tilt: [' + str(self.beam_tilt_x) + ',' + str(self.beam_tilt_y) \
           + '] Divergence: [' + str(self.beam_divergence_x) + ',' + str(self.beam_divergence_y) + ']\n'
  def __str__(self):
    return self.__repr__()

def createIPOffsetScenarios(ip_param_list, ip_offset_abs_list, ip_offset_mode):
  current_ip_param_list = list(ip_param_list)
  del ip_param_list[:]
  offset_values = map(abs, ip_offset_abs_list)
  for current_ip_params in current_ip_param_list:
    for offset_value in offset_values:
      if offset_value > 0.0:
          if ip_offset_mode == 'a':
            current_ip_params.setIPOffsetXYZ(offset_value, 0.0, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(-offset_value, 0.0, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(0.0, offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(0.0, -offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
          elif ip_offset_mode == 'c':
            current_ip_params.setIPOffsetXYZ(offset_value, offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(offset_value, -offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(-offset_value, offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(-offset_value, -offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
          else:
            current_ip_params.setIPOffsetXYZ(offset_value, 0.0, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(-offset_value, 0.0, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(0.0, offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(0.0, -offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(offset_value, offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(offset_value, -offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(-offset_value, offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setIPOffsetXYZ(-offset_value, -offset_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            
def createBeamTiltScenarios(ip_param_list, beam_tilt_abs_list, beam_tilt_mode):
  current_ip_param_list = list(ip_param_list)
  del ip_param_list[:]
  beam_tilt_values = map(abs, beam_tilt_abs_list)
  for current_ip_params in current_ip_param_list:
    for beam_tilt_value in beam_tilt_values:
      beam_tilt_value = beam_tilt_value / 1000  # convert to rad
      if beam_tilt_value > 0.0:
          if beam_tilt_mode == 'a':
            current_ip_params.setBeamTiltXY(beam_tilt_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(-beam_tilt_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(0.0, beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(0.0, -beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
          elif beam_tilt_mode == 'c':
            current_ip_params.setBeamTiltXY(beam_tilt_value, beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(beam_tilt_value, -beam_tilt_value,)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(-beam_tilt_value, beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(-beam_tilt_value, -beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
          else:
            current_ip_params.setBeamTiltXY(beam_tilt_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(-beam_tilt_value, 0.0)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(0.0, beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(0.0, -beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(beam_tilt_value, beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(beam_tilt_value, -beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(-beam_tilt_value, beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
            current_ip_params.setBeamTiltXY(-beam_tilt_value, -beam_tilt_value)
            ip_param_list.append(copy.deepcopy(current_ip_params))
      
def createIPParameterScenarios(default_ip_params, ip_offset_abs_list, ip_offset_mode, beam_tilt_abs_list, beam_tilt_mode):
  ip_param_list = []
  ip_param_list.append(default_ip_params)
  if ip_offset_abs_list:
    createIPOffsetScenarios(ip_param_list, ip_offset_abs_list, ip_offset_mode)
  if beam_tilt_abs_list:
    createBeamTiltScenarios(ip_param_list, beam_tilt_abs_list, beam_tilt_mode)
    
  return ip_param_list


parser = argparse.ArgumentParser(description='Script for full simulation of PANDA Luminosity Detector via externally generated MC data.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('num_events', metavar='num_events', type=int, nargs=1, help='number of events to simulate')
parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('sim_type', metavar='simulation_type', type=str, nargs=1, choices=['box', 'dpm_elastic', 'dpm_elastic_inelastic', 'noise'], help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')

parser.add_argument('--low_index', metavar='low_index', type=int, default=-1,
                   help='Lowest index of generator file which is supposed to be used in the simulation. Default setting is -1 which will take the lowest found index.')
parser.add_argument('--high_index', metavar='high_index', type=int, default=-1,
                   help='Highest index of generator file which is supposed to be used in the simulation. Default setting is -1 which will take the highest found index.')

parser.add_argument('--gen_data_dir', metavar='gen_data_dir', type=str, default=os.getenv('GEN_DATA'),
                   help='Base directory to input files created by external generator. By default the environment variable $GEN_DATA will be used!')

parser.add_argument('--output_dir', metavar='output_dir', type=str, default='', help='This directory is used for the output. Default is the generator directory as a prefix, with beam offset infos etc. added')


parser.add_argument('--ip_offset_mode', metavar='ip_offset_mode', choices=['a', 'c', 'f'], help='a=axis, c=corners, f=full round')

parser.add_argument('--ip_offset_abs', metavar='ip_offset_abs', type=float, nargs='*', default=0.0,
                   help="ip_offset_abs: distances from center that are used as a template value (in cm)")


parser.add_argument('--beam_tilt_mode', metavar='beam_tilt_mode', choices=['a', 'c', 'f'], help='a = axis, c=corners, f=full round')

parser.add_argument('--beam_tilt_abs', metavar='beam_tilt_abs', type=float, nargs='*', default=0.0,
                   help="beam_tilt_abs: tilts with respect to the beam axis, which are used as a template value for the scans (in mrad)")


parser.add_argument('--use_ip_spread', metavar=('ip_spread_x', 'ip_spread_y', 'ip_spread_z'), type=float, nargs=3, default=[0.08, 0.08, 0.35],
                   help="ip_spread in xyz direction (in cm)")

parser.add_argument('--use_beam_divergence', metavar=("beam_divergence_x", "beam_divergence_y"), type=float, nargs=2, default=[0.0, 0.0],
                   help="beam_divergence_x: beam divergence in x direction (in mrad)\n"
			"beam_divergence_y: beam divergence in y direction (in mrad)")

parser.add_argument('--track_search_algo', metavar='track_search_algorithm', type=str, choices=['CA', 'Follow'], default='CA', help='Track Search algorithm to be used.')

parser.add_argument('--use_xy_cut', action='store_true', help='Use the x-theta & y-phi filter after the tracking stage to remove background.')
parser.add_argument('--use_m_cut', action='store_true', help='Use the tmva based momentum cut filter after the backtracking stage to remove background.')

parser.add_argument('--reco_ip_offset', metavar=("rec_ip_offset_x", "rec_ip_offset_y", "rec_ip_spread_x", "rec_ip_spread_y"), type=float, nargs=4, default=[0.0, 0.0, -1.0, -1.0],
                   help="rec_ip_offset_x: interaction vertex mean X position (in cm)\n"
            "rec_ip_offset_y: interaction vertex mean Y position (in cm)\n"
            "rec_ip_spread_x: interaction vertex X position distribution width (in cm)\n"
            "rec_ip_spread_y: interaction vertex Y position distribution width (in cm)\n")

args = parser.parse_args()

default_ip_params = IPParams()
default_ip_params.setIPSpreadXYZ(args.use_ip_spread[0], args.use_ip_spread[1], args.use_ip_spread[2])
default_ip_params.setBeamDivergenceXY(args.use_beam_divergence[0], args.use_beam_divergence[1])
ip_params_list = createIPParameterScenarios(default_ip_params, args.ip_offset_abs, args.ip_offset_mode, args.beam_tilt_abs, args.beam_tilt_mode)

additional_flags = ''
if args.use_xy_cut:
  additional_flags += ' --use_xy_cut'
if args.use_m_cut:
  additional_flags += ' --use_m_cut'

for ip_params in ip_params_list:
  rec_ip_info = ''
  # if we are using the xy cut 
  if args.use_xy_cut or args.use_m_cut:
    if args.reco_ip_offset[2] >= 0.0 and args.reco_ip_offset[3] >= 0.0:
      rec_ip_info = ' --reco_ip_offset ' + str(args.reco_ip_offset[0]) + ' ' + str(args.reco_ip_offset[1]) + ' ' + str(args.reco_ip_offset[2]) + ' ' + str(args.reco_ip_offset[3])
    
  bashcommand = 'python runSimulations.py --low_index ' + str(args.low_index) + ' --high_index ' + str(args.high_index) \
                + ' --use_ip_offset ' + str(ip_params.ip_offset_x) + ' ' + str(ip_params.ip_offset_y) + ' ' + str(ip_params.ip_offset_z) + ' ' \
                + str(ip_params.ip_spread_x) + ' ' + str(ip_params.ip_spread_y) + ' ' + str(ip_params.ip_spread_z) \
                + ' --use_beam_gradient ' + str(ip_params.beam_tilt_x) + ' ' + str(ip_params.beam_tilt_y) + ' ' + str(ip_params.beam_divergence_x) + ' ' + str(ip_params.beam_divergence_x) \
                + additional_flags + rec_ip_info + ' --track_search_algo ' + args.track_search_algo + ' ' + str(args.num_events[0]) + ' ' + str(args.lab_momentum[0]) + ' ' + args.sim_type[0]
  returnvalue = subprocess.call(bashcommand.split())

