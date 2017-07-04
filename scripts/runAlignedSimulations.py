#!/usr/bin/python

import subprocess
import argparse
import os

parser = argparse.ArgumentParser(description='Script for full simulation of PANDA Luminosity Detector via externally generated MC data.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('num_events', metavar='num_events', type=int, nargs=1, help='number of events to simulate')
parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('--sim_type', metavar='simulation_type', type=str, choices=['box', 'dpm_elastic', 'dpm_elastic_inelastic', 'noise'], default='dpm_elastic', help='four kinds: box, dpm_elastic, dpm_elastic_inelastic and noise')

parser.add_argument('--force_level', metavar='force_level', type=int, default=0,
                    help='force level 0: if directories exist with data files no new simulation is started\n'
                    'force level 1: will do full reconstruction even if this data already exists, but not geant simulation\n'
                    'force level 2: resimulation of everything!')

parser.add_argument('--low_index', metavar='low_index', type=int, default=1,
                   help='Lowest index of generator file which is supposed to be used in the simulation. Default setting is 1')
parser.add_argument('--high_index', metavar='high_index', type=int, default=100,
                   help='Highest index of generator file which is supposed to be used in the simulation. Default setting is 100')

parser.add_argument('--gen_data_dir', metavar='gen_data_dir', type=str, default=os.getenv('GEN_DATA'),
                   help='Base directory to input files created by external generator. By default the environment variable $GEN_DATA will be used!')

parser.add_argument('--output_dir', metavar='output_dir', type=str, default='', help='This directory is used for the output. Default is the generator directory as a prefix, with beam offset infos etc. added')

parser.add_argument('--lmd_detector_geometry_filename', metavar='lmd_detector_geometry_filename', type=str, default='Luminosity-Detector.root', help='Filename of the Geant Luminosity Detector geometry file in the pandaroot geometry subfolder')


args = parser.parse_args()

    


bashcommand = 'python runSimulations.py --force_level ' + str(args.force_level) + ' --low_index ' + str(args.low_index) + ' --high_index ' + str(args.high_index) \
                + ' --lmd_detector_geometry_filename ' + args.lmd_detector_geometry_filename + ' ' \
                + str(args.num_events[0]) + ' ' + str(args.lab_momentum[0]) + ' ' + args.sim_type
#print bashcommand
returnvalue = subprocess.call(bashcommand.split())

