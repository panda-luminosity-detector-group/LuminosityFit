import os, sys, re, errno
import subprocess
import multiprocessing

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

parser = argparse.ArgumentParser(description='Script for external generation of box MC data with elastic energy loss correction.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('evts_per_sample', metavar='evts_per_sample', type=int, nargs=1, help='number of events per sample to simulate')
parser.add_argument('low_index', metavar='low_index', type=int, nargs=1, help='starting sample index')
parser.add_argument('high_index', metavar='high_index', type=int, nargs=1, help='ending sample index (difference with starting index is total number of samples that will be generated)')


parser.add_argument('--theta_min', metavar='theta_min', type=float, default=0.0,
                   help='Minimal value of the scattering angle theta in mrad.')
parser.add_argument('--theta_max', metavar='theta_max', type=float, default=15.0,
                   help='Maximal value of the scattering angle theta in mrad.')

parser.add_argument('--gen_data_dir', metavar='gen_data_dir', type=str, default=os.getenv('GEN_DATA'),
                   help='Base directory for generator output files. By default the environment variable $GEN_DATA will be used!')

parser.add_argument('--neglect_recoil_momentum', action='store_false', help='If recoil momentum should not be subtracted from scattered antiprotons.')

args = parser.parse_args()

suffix = '_recoil_corrected'
boolval = '1'
if not args.neglect_recoil_momentum:
  suffix = ''
  boolval = '0'  

dirname = str(args.evts_per_sample[0]) + '_box_plab_' + str(args.lab_momentum[0]) + 'GeV_th_' + str(args.theta_min) + '-' + str(args.theta_max) + 'mrad' + suffix
dirname_cleaned = re.sub('\.', 'o', dirname)

basedir = args.gen_data_dir
while not os.path.isdir(basedir):
    basedir = raw_input('Please enter valid generator base path: ')

print 'using theta range of [' + str(args.theta_min) + ' - ' + str(args.theta_max) + ']'

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
    bashcommand = 'qsub -t ' + str(job_index) + '-' + str(min(job_index + max_jobarray_size - 1, high_index_used)) + ' -N runBoxGen_' + dirname_cleaned \
                  + ' -l nodes=1:ppn=1,walltime=00:30:00 -j oe -o ' + basedir + '/' + dirname + '/runBoxGen_' + dirname_cleaned + ' -v lab_momentum="' \
                  + str(args.lab_momentum[0]) + '",num_events="' + str(args.evts_per_sample[0]) + '",theta_min="' + str(args.theta_min) + '",theta_max="' \
                  + str(args.theta_max) + '",dirname="' + dirname + '",dirname_cleaned="' + dirname_cleaned + '",basedir="' + basedir + '",use_recoil_mom="' + boolval + '" -V ./runBoxGen.sh'
    subprocess.call(bashcommand.split())

elif is_parallel:
  print 'This is not a cluster environment, but gnu parallel was found! Using gnu parallel!'
  
  bashcommand = 'parallel -j' + str(cpu_cores) + ' ./runBoxGen.sh ' + str(args.lab_momentum[0]) + ' ' + str(args.evts_per_sample[0]) + ' ' + str(args.theta_min) + ' ' + str(args.theta_max) + ' ' + dirname + ' ' + dirname_cleaned + ' ' + basedir + ' {}'
  
  inputcommand = 'seq ' + str(low_index_used) + ' 1 ' + str(high_index_used)
  inproc = subprocess.Popen(inputcommand.split(), stdout=subprocess.PIPE)
  mainproc = subprocess.Popen(bashcommand.split(), stdin=subprocess.PIPE)
  mainproc.communicate(input=inproc.communicate()[0])
else:
  print 'This is not a cluster environment, and unable to find gnu parallel! Please install gnu parallel!'
