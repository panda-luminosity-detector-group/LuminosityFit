import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []
box_dirs = []

dpm_glob_pattern = 'lmd_data.root'
dir_pattern = ''
tail_dir_pattern = ''

def getListOfDPMDirectories(path):
  if os.path.isdir(path):
    if os.path.split(path)[1] == 'mc_data':
      return
    for dir in os.listdir(path):
      bunch_dirs = glob.glob(path + '/bunches_*/' + tail_dir_pattern)
      if bunch_dirs:
        for bunch_dir in bunch_dirs:
          filelists = glob.glob(bunch_dir + '/' + dpm_glob_pattern)
          if filelists:
            m = re.search(dir_pattern, bunch_dir)
            if m:
              dirs.append(bunch_dir)
        return
      else:
        if glob.glob(path + '/Lumi_TrksQA_*.root'):
          return
      dirpath = path + '/' + dir
      if os.path.isdir(dirpath):
        getListOfDPMDirectories(dirpath)

box_res_glob_pattern = 'lmd_res_data.root'
box_acc_glob_pattern = 'lmd_acc_data.root'

top_level_box_directory = ''

def getListOfBoxDirectories(path):
  if os.path.isdir(path):
    print 'currently looking at directory ' + path
    
    if os.path.split(path)[1] == 'mc_data':
      return
  
    for dir in os.listdir(path):
      bunch_dirs = glob.glob(path + '/bunches_*/' + tail_dir_pattern)
      if bunch_dirs:
        for bunch_dir in bunch_dirs:
          filelists = glob.glob(bunch_dir + '/' + box_acc_glob_pattern)
          if filelists:
            filelists = glob.glob(bunch_dir + '/' + box_res_glob_pattern)
            if filelists:
              box_dirs.append(bunch_dir)
        return
      else:
        if glob.glob(path + '/Lumi_TrksQA_*.root'):
          return
      dirpath = path + '/' + dir
      if os.path.isdir(dirpath):
        getListOfBoxDirectories(dirpath)

def getTopBoxDirectory(path):
  if os.path.isdir(path):
    if not re.search('box', path):
      if not os.listdir(path):
        return
      if glob.glob(path + '/*.root'):
        return
      for dir in os.listdir(path):
        getTopBoxDirectory(path + '/' + dir)
    else:
      global top_level_box_directory
      top_level_box_directory = path

def findMatchingDirs(box_data_path):
  matching_dir_pairs = []
  if box_data_path == '':
    for dpm_dir in dirs:
      match = re.search('^(.*/)dpm_.*?(/.*/)\d*-\d*x\d*_(.*cut)/.*(/.*?)$', dpm_dir)
      pattern = '^' + match.group(1) + 'box_.*?' + match.group(2) + '.*' + match.group(3) + '/.*' + match.group(4) + '$'
      print pattern
      for box_dir in box_dirs:
        print box_dir
        box_match = re.search(pattern, box_dir)
        if box_match:
          matching_dir_pairs.append([dpm_dir, box_dir])
          break
  else:
    for dpm_dir in dirs:
      matching_dir_pairs.append([dpm_dir, box_data_path])
  return matching_dir_pairs
          

parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')

parser.add_argument('dirname_pattern', metavar='directory pattern', type=str, nargs=1,
                    help='Only found directories with this pattern are used!')

parser.add_argument('config_url', metavar='Path to fit config file (json)', type=str, nargs=1,
                    help='Specify the full path of the config file that will be used for the fit.')

parser.add_argument('--tail_dir_pattern', metavar='tail directory pattern', type=str, default='merge_data',
                    help='Only found directories with this pattern are used!')

parser.add_argument('--ref_box_gen_data', metavar='ref box gen data', type=str, default='',
                    help='If specified then this path will be used for all fits as the reference box gen data.')

parser.add_argument('--forced_box_gen_data', metavar='forced box gen data', type=str, default='',
                    help='If specified then this path will be used for all fits as the box gen data, ignoring other box gen data directories.')

parser.add_argument('--number_of_threads', metavar='number of concurrent threads', type=int, default='8',
                    help='Number of concurrent threads within the estimator that is used for fitting. Default: 8 threads.')

args = parser.parse_args()

number_of_threads = str(args.number_of_threads)
if args.number_of_threads > 16:
  number_of_threads = '16'

dir_pattern = args.dirname_pattern[0]

tail_dir_pattern = args.tail_dir_pattern

getListOfDPMDirectories(args.dirname[0])
print dirs

if args.forced_box_gen_data == '':
  getTopBoxDirectory(args.dirname[0])
  print top_level_box_directory
  getListOfBoxDirectories(top_level_box_directory)
print box_dirs

matches = findMatchingDirs(args.forced_box_gen_data)

command_suffix = '" -V';
if args.ref_box_gen_data != '':
  command_suffix = '",reference_acceptance_path="'+ args.ref_box_gen_data + '" -V'

for match in matches:
  bashcommand = 'qsub -N runLmdFit -l nodes=1:ppn='+number_of_threads+',walltime=02:00:00,mem=6000mb,vmem=6000mb -j oe -o ' + match[0] + '/runLmdFit_pbs.log ' \
                + ' -v config_url="' + args.config_url[0] + '",data_path="' + match[0] + '",acceptance_resolution_path="' + match[1] \
                + '",number_of_threads="' + number_of_threads + command_suffix + ' ./runLmdFit.sh'
  returnvalue = subprocess.call(bashcommand.split())