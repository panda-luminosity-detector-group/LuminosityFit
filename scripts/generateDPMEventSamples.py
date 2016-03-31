import os, sys, re, errno
import subprocess
import multiprocessing

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

parser = argparse.ArgumentParser(description='Script for external generation of DPM MC data.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('evts_per_sample', metavar='evts_per_sample', type=int, nargs=1, help='number of events per sample to simulate')
parser.add_argument('low_index', metavar='low_index', type=int, nargs=1, help='starting sample index')
parser.add_argument('high_index', metavar='high_index', type=int, nargs=1, help='ending sample index (difference with starting index is total number of samples that will be generated)')

parser.add_argument('--reaction_type', metavar='reaction_type', type=int, default=2,
choices=[0, 1, 2], help='0 = only inelastic; 1 = elastic & inelastic; 2 = only elastic')

parser.add_argument('--theta_min', metavar='theta_min', type=float, default=0.06,
                   help='Minimal value of the scattering angle theta. Cutoff required due to divergence of Coulomb cross section.')

parser.add_argument('--gen_data_dir', metavar='gen_data_dir', type=str, default=os.getenv('GEN_DATA'),
                   help='Base directory for generator output files. By default the environment variable $GEN_DATA will be used!')

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

print 'preparing simulations in index range ' + str(low_index_used) + ' - ' + str(high_index_used)

# now chop all jobs into bunches of 100 which is max job array size on himster atm
max_jobarray_size = 100

def is_exe(fpath):
  return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

program = 'qsub'
is_cluster = 0
for path in os.environ["PATH"].split(os.pathsep):
  path = path.strip('"')
  exe_file = os.path.join(path, program)
  if is_exe(exe_file):
    is_cluster = 1

program = 'parallel'
is_parallel = 0
for path in os.environ["PATH"].split(os.pathsep):
  path = path.strip('"')
  exe_file = os.path.join(path, program)
  if is_exe(exe_file):
    is_parallel = 1


if is_cluster:
  print 'This is a cluster environment... submitting jobs to cluster!'
  
  for job_index in range(low_index_used, high_index_used + 1, max_jobarray_size):
	bashcommand = 'qsub -t ' + str(job_index) + '-' + str(min(job_index + max_jobarray_size - 1, high_index_used)) + ' -N runDPMGen_' + dirname_cleaned + ' -l nodes=1:ppn=1,walltime=00:30:00 -j oe -o ' + basedir + '/' + dirname + '/runDPMGen_' + dirname_cleaned + ' -v lab_momentum="' + str(args.lab_momentum[0]) + '",num_events="' + str(args.evts_per_sample[0]) + '",reaction_type="' + str(args.reaction_type) + '",minimal_theta_value="' + str(args.theta_min) + '",dirname="' + dirname + '",dirname_cleaned="' + dirname_cleaned + '",basedir="' + basedir + '" -V ./runDPMGen.sh'
	subprocess.call(bashcommand.split())

elif is_parallel:
  print 'This is not a cluster environment, but gnu parallel was found! Using gnu parallel!'
  
  bashcommand = 'parallel -j' + str(cpu_cores) + ' ./runDPMGen.sh ' + str(args.lab_momentum[0]) + ' ' + str(args.evts_per_sample[0]) + ' ' + str(args.reaction_type) + ' ' + str(args.theta_min) + ' ' + dirname + ' ' + dirname_cleaned + ' ' + basedir + ' {}'
  
  inputcommand = 'seq ' + str(low_index_used) + ' 1 ' + str(high_index_used)
  inproc = subprocess.Popen(inputcommand.split(), stdout=subprocess.PIPE)
  mainproc = subprocess.Popen(bashcommand.split(), stdin=subprocess.PIPE)
  mainproc.communicate(input=inproc.communicate()[0])
else:
  print 'This is not a cluster environment, and unable to find gnu parallel! Please install gnu parallel!'
