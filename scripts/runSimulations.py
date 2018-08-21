#!/usr/bin/env python3

import himster
import simulation
import random
import math
import os, errno, sys, glob, re
import argparse

parser = argparse.ArgumentParser(description='Script for full simulation of PANDA Luminosity Detector via externally generated MC data.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('num_events', metavar='num_events', type=int, nargs=1, help='number of events to simulate')
parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('sim_type', metavar='simulation_type', type=str, nargs=1, choices=['box', 'dpm_elastic', 'dpm_elastic_inelastic', 'noise'],
                    help='Simulation type which can be one of the following: box, dpm_elastic, dpm_elastic_inelastic, noise.\n'
                        'This information is used to automatically obtain the generator data and output naming scheme.')

parser.add_argument('--theta_min', metavar='theta_min', type=float, default=2.0,
                   help='Minimal value of the scattering angle theta in mrad.')
parser.add_argument('--theta_max', metavar='theta_max', type=float, default=12.0,
                   help='Maximal value of the scattering angle theta in mrad. Only used for box generator')
parser.add_argument('--neglect_recoil_momentum', action='store_false', 
                    help='If recoil momentum should not be subtracted from scattered antiprotons. Only for box generator')
parser.add_argument('--random_seed', metavar='random_seed', type=int, default=random.randint(10, 9999),
                   help='random seed used in data generation and mc simulation')

parser.add_argument('--force_level', metavar='force_level', type=int, default=0,
                    help='force level 0: if directories exist with data files no new simulation is started\n'
                    'force level 1: will do full reconstruction even if this data already exists, but not geant simulation\n'
                    'force level 2: resimulation of everything!')

parser.add_argument('--low_index', metavar='low_index', type=int, default=-1,
                   help='Lowest index of generator file which is supposed to be used in the simulation.\n'
                   'Default setting is -1 which will take the lowest found index.')
parser.add_argument('--high_index', metavar='high_index', type=int, default=-1,
                   help='Highest index of generator file which is supposed to be used in the simulation.\n'
                   'Default setting is -1 which will take the highest found index.')

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

parser.add_argument('--reco_ip_offset', metavar=("rec_ip_offset_x", "rec_ip_offset_y", "rec_ip_offset_z"), type=float, nargs=3, default=[0.0, 0.0, 0.0],
                   help="rec_ip_offset_x: interaction vertex mean X position (in cm)\n"
            "rec_ip_offset_y: interaction vertex mean Y position (in cm)\n"
            "rec_ip_offset_z: interaction vertex mean Z position (in cm)\n")

parser.add_argument('--lmd_detector_geometry_filename', metavar='lmd_detector_geometry_filename', type=str, default='Luminosity-Detector.root', help='Filename of the Geant Luminosity Detector geometry file in the pandaroot geometry subfolder')
parser.add_argument('--use_devel_queue', action='store_true', help='If flag is set, the devel queue is used')

parser.add_argument('--misalignment_matrices_path', metavar='misalignment_matrices_path', type=str, default='', help='')
parser.add_argument('--alignment_matrices_path', metavar='alignment_matrices_path', type=str, default='', help='')


args = parser.parse_args()

sim_params=simulation.generateSimulationParameters(args)

dirname = simulation.generateDirectory(sim_params)
dirname_filter_suffix = simulation.generateFilterSuffix(sim_params)

low_index_used = sim_params['low_index']
high_index_used = sim_params['high_index']

pathname_base = os.getenv('LMDFIT_DATA_DIR') + '/' + dirname
path_mc_data = pathname_base + '/mc_data'
dirname_full = dirname + '/' + dirname_filter_suffix
pathname_full = os.getenv('LMDFIT_DATA_DIR') + '/' + dirname_full

print('using output folder structure: ' + pathname_full)

try:
    os.makedirs(pathname_full)
    os.makedirs(pathname_full+'/Pairs')
    os.makedirs(path_mc_data)
except OSError as exception:
    if exception.errno != errno.EEXIST:
        print('error: thought dir does not exists but it does...')

min_file_size = 3000 #in bytes
if args.force_level == 0:
    # check if the directory already has the reco data in it 
    reco_files = glob.glob(pathname_full + '/Lumi_TrksQA_*.root')
    total_requested_jobs=(high_index_used-low_index_used+1)
    reco_files = [x for x in reco_files if os.path.getsize(x) > min_file_size]
    if total_requested_jobs == 1:
        if len(reco_files) == total_requested_jobs:
            print('directory of with fully reconstructed track file already exists! Skipping...')
            sys.exit()
    else:
        if len(reco_files) >= int(0.8*total_requested_jobs):
            print('directory with at least 80% (compared to requested number of simulated files) of fully reconstructed track files already exists! Skipping...')
            sys.exit()
            
# generate simulation config parameter file
simulation.generateSimulationParameterPropertyFile(pathname_base, sim_params)

use_recoil_momentum = 1
if args.neglect_recoil_momentum:
    use_recoil_momentum = 0

# box gen: -1
# dpm only inelastic: 0
# dpm elastic & inelastic: 1
# dpm only elastic: 2
print("simulation type:", args.sim_type[0])
reaction_type = 2
if args.sim_type[0] == 'box':
    reaction_type = -1
elif args.sim_type[0] == 'dpm_elastic':
    reaction_type = 2
elif args.sim_type[0] == 'dpm_elastic_inelastic':
    reaction_type = 1

joblist = []

resource_request = himster.JobResourceRequest(12*60)
resource_request.number_of_nodes = 1
resource_request.processors_per_node = 1
resource_request.memory_in_mb = 3000
resource_request.node_scratch_filesize_in_mb = 0

if args.use_devel_queue:
    resource_request = himster.make_test_resource_request()

job = himster.Job(resource_request, './runLumiFullSimPixel.sh', 'lmd_fullsim_' + args.sim_type[0], pathname_full + '/sim-%a.log')
job.set_job_array_indices(list(range(low_index_used, high_index_used+1)))

job.add_exported_user_variable('num_evts', str(args.num_events[0]))
job.add_exported_user_variable('mom', str(args.lab_momentum[0]))
job.add_exported_user_variable('reaction_type', reaction_type)
job.add_exported_user_variable('theta_min_in_mrad', args.theta_min)
job.add_exported_user_variable('theta_min_in_deg', args.theta_min*0.180/math.pi)
job.add_exported_user_variable('theta_max_in_mrad', args.theta_max)
job.add_exported_user_variable('use_recoil_momentum', use_recoil_momentum)
job.add_exported_user_variable('random_seed', args.random_seed)
                               
job.add_exported_user_variable('dirname', dirname_full)
job.add_exported_user_variable('path_mc_data', path_mc_data)
job.add_exported_user_variable('pathname', pathname_full)
job.add_exported_user_variable('beamX0', str(args.use_ip_offset[0]))
job.add_exported_user_variable('beamY0', str(args.use_ip_offset[1]))
job.add_exported_user_variable('targetZ0', str(args.use_ip_offset[2]))
job.add_exported_user_variable('beam_widthX', str(args.use_ip_offset[3]))
job.add_exported_user_variable('beam_widthY', str(args.use_ip_offset[4]))
job.add_exported_user_variable('target_widthZ', str(args.use_ip_offset[5]))
job.add_exported_user_variable('beam_gradX', str(args.use_beam_gradient[0]))
job.add_exported_user_variable('beam_gradY', str(args.use_beam_gradient[1]))
job.add_exported_user_variable('beam_grad_sigmaX', str(args.use_beam_gradient[2]))
job.add_exported_user_variable('beam_grad_sigmaY', str(args.use_beam_gradient[3]))
job.add_exported_user_variable('SkipFilt', str(not args.use_xy_cut).lower())
job.add_exported_user_variable('XThetaCut', str(args.use_xy_cut).lower())
job.add_exported_user_variable('YPhiCut', str(args.use_xy_cut).lower())
job.add_exported_user_variable('CleanSig', str(args.use_m_cut).lower())
job.add_exported_user_variable('track_search_algorithm', args.track_search_algo)
job.add_exported_user_variable('lmd_geometry_filename', args.lmd_detector_geometry_filename)
job.add_exported_user_variable('misalignment_matrices_path', args.misalignment_matrices_path)
job.add_exported_user_variable('alignment_matrices_path', args.alignment_matrices_path)
job.add_exported_user_variable('force_level', args.force_level)
if args.sim_type[0] == 'noise':
  job.add_exported_user_variable('simulate_noise', '1')
job.add_exported_user_variable('rec_ipx', str(args.reco_ip_offset[0]))
job.add_exported_user_variable('rec_ipy', str(args.reco_ip_offset[1]))
job.add_exported_user_variable('rec_ipz', str(args.reco_ip_offset[2]))

joblist.append(job)

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = himster.HimsterJobManager(2000, 3600)

job_manager.submit_jobs_to_himster(joblist)
job_manager.manage_jobs()
