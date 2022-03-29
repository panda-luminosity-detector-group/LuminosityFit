#!/usr/bin/python

# TODO: this entire file must be ported to python3

import os, sys, re, errno

# TODO: remove hard coded path
lib_path = os.path.abspath('../steve/argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse


parser = argparse.ArgumentParser(description='Script for the determination of the luminosity from reconstructed data of PANDA.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('lab_momentum', metavar='The beam antiproton lab momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('elastic_data_dir', metavar='Elastic data directory', type=str, nargs=1,
                help='The directory containing the TrackQA files from which an LmdData object will be created.')

parser.add_argument('--output_dir', metavar='Output directory', type=str, default='lumi_results',
                    help='Output directory, in which the fit results and plots will be saved.')
parser.add_argument('--data_config_url', metavar='Path to data config file (json)', type=str, default='dataconfig_xy.json',
                    help='Path to data config file in json format.')
parser.add_argument('--fit_config_url', metavar='Path to fit config file (json)', type=str, default='fitconfig.json',
                    help='Specify the full path of the config file that will be used for the fit.')

parser.add_argument('--box_data_dir', metavar='Box generated directory', type=str, default="",
                    help='The directory containing the TrackQA files from which the acceptance and resolution LmdData objects will be created.')
parser.add_argument('--acc_res_dir', metavar='Acceptance and Resolution file directory', type=str, default=".",
                    help='The directory containing the files for the angular acceptance and resolution objects.')
parser.add_argument('--elastic_cross_section', metavar='elastic_cross_section', type=float, default=1.0, help='Total elastic cross section. Relevant for luminosity extraction performance tests!')
parser.add_argument('--number_of_threads', metavar='number of concurrent threads', type=int, default='2',
                    help='Number of concurrent threads within the estimator that is used for fitting. Default: 8 threads.')


args = parser.parse_args()

gen_data_dir_path = args.gen_data_dir


# create lmd data objects from angular data
bashcommand = 'createLmdFitData -m ' + str(args.lab_momentum[0]) + ' -t av -d '\
     + args.elastic_data_dir[0] + ' -c ' + args.data_config_url + ' -e ' + str(args.elastic_cross_section)\
     + ' -o ' + args.output_dir
returnvalue = subprocess.call(bashcommand.split())
if returnvalue == 0:
    print 'Aborting due to failed lmd data creation.'
    exit(returnvalue)

# determine the beam offset

# if new acceptance and resolutions are required start a new simulation of those
# and create the lmd data objects for these

# run the lumi fit
bashcommand = 'runLmdFit -d ' + args.output_dir + ' -a '\
     + args.acc_res_dir + ' -c ' + args.fit_config_url + ' -m ' + args.number_of_threads
returnvalue = subprocess.call(bashcommand.split())
if returnvalue == 0:
    print 'Aborting due to failed lmd fit.'
    exit(returnvalue)

# plot results
bashcommand = 'plotLumiFitResults ' + args.output_dir
returnvalue = subprocess.call(bashcommand.split())
if returnvalue == 0:
    print 'Failed to create plots.'
    exit(returnvalue)
    
print 'Successfully finished luminosity determination!'
