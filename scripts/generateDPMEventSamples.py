#!/usr/bin/env python3

import os, sys, re, errno
import argparse
import himster

parser = argparse.ArgumentParser(description='Script for external generation of DPM MC data.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('evts_per_sample', metavar='evts_per_sample', type=int, nargs=1, help='number of events per sample to simulate')
parser.add_argument('low_index', metavar='low_index', type=int, nargs=1, help='starting sample index')
parser.add_argument('high_index', metavar='high_index', type=int, nargs=1, help='ending sample index (difference with starting index is total number of samples that will be generated)')

parser.add_argument('--reaction_type', metavar='reaction_type', type=int, default=2,
choices=[0, 1, 2], help='0 = only inelastic; 1 = elastic & inelastic; 2 = only elastic')

parser.add_argument('--theta_min', metavar='theta_min', type=float, default=0.06,
                   help='Minimal value of the scattering angle theta. Cutoff required due to divergence of Coulomb cross section.')

parser.add_argument('--gen_data_dir', metavar='gen_data_dir', type=str, default=os.getenv('LMDFIT_GEN_DATA'),
                   help='Base directory for generator output files. By default the environment variable $LMDFIT_GEN_DATA will be used!')
parser.add_argument('--use_devel_queue', action='store_true', help='If flag is set, the devel queue is used')


args = parser.parse_args()

# generate output directory name
typename = 'elastic'
if(args.reaction_type == 0):
  typename = 'inelastic'
elif(args.reaction_type == 1):
  typename = 'elastic_inelastic'

dirname = str(args.evts_per_sample[0]) + '_dpm_' + typename + '_plab_' + str(args.lab_momentum[0]) + 'GeV_thmin_' + str(args.theta_min) + 'deg'
dirname_cleaned = re.sub('\.', 'o', dirname)

basedir = args.gen_data_dir
while not os.path.isdir(basedir):
	basedir = raw_input('Please enter valid generator base path: ')

low_index_used = args.low_index[0]
high_index_used = args.high_index[0]

try:
    os.makedirs(basedir + '/' + dirname)
except OSError as exception:
    if exception.errno != errno.EEXIST:
        print('error: thought dir does not exists but it does...')

print('preparing simulations in index range ' + str(low_index_used) + ' - ' + str(high_index_used))

resource_request = himster.JobResourceRequest(30)
resource_request.number_of_nodes = 1
resource_request.processors_per_node = 1
resource_request.memory_in_mb = 1000
resource_request.node_scratch_filesize_in_mb = 0

if args.use_devel_queue:
    resource_request = himster.make_test_resource_request()

job = himster.Job(resource_request, './runDPMGen.sh', 'runDPMGen_' + dirname_cleaned, basedir + '/' + dirname + '/runDPMGen_' + dirname_cleaned + '-%a.log')
job.set_job_array_indices(list(range(low_index_used, high_index_used+1)))
  
job.add_exported_user_variable('lab_momentum', str(args.lab_momentum[0]))
job.add_exported_user_variable('num_events', str(args.evts_per_sample[0]))
job.add_exported_user_variable('reaction_type', str(args.reaction_type))
job.add_exported_user_variable('minimal_theta_value', str(args.theta_min))
job.add_exported_user_variable('dirname', dirname)
job.add_exported_user_variable('dirname_cleaned', dirname_cleaned)
job.add_exported_user_variable('basedir', basedir)

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = himster.HimsterJobManager(2000, 3600)

job_manager.submit_jobs_to_himster([job])
job_manager.manage_jobs()
