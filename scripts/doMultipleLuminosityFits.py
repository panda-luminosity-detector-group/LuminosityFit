import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing
import general, himster

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []
box_dirs = []

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
      #attempt to find directory with same binning
      match = re.search('^.*(binning_\d*)/.*$', dpm_dir)
      if match:
        dir_searcher = general.DirectorySearcher(match.group(1))
        dir_searcher.searchListOfDirectories(box_data_path, box_acc_glob_pattern)
        correct_dirs = dir_searcher.getListOfDirectories()
      
        if correct_dirs:
          matching_dir_pairs.append([dpm_dir, correct_dirs[0]])
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

number_of_threads = args.number_of_threads
if args.number_of_threads > 16:
  number_of_threads = 16

dir_pattern = args.dirname_pattern[0]

tail_dir_pattern = args.tail_dir_pattern


dpm_glob_pattern = 'lmd_data.root'

dir_searcher = general.DirectorySearcher(dir_pattern)
dir_searcher.searchListOfDirectories(args.dirname[0], dpm_glob_pattern)
dirs = dir_searcher.getListOfDirectories()


if args.forced_box_gen_data == '':
  getTopBoxDirectory(args.dirname[0])
  print top_level_box_directory
  getListOfBoxDirectories(top_level_box_directory)


matches = findMatchingDirs(args.forced_box_gen_data)

command_suffix = '" -V';


joblist = []

for match in matches:
  elastic_data_path = match[0]
  acc_res_data_path = match[1]

  resource_request = himster.JobResourceRequest(12 * 60)
  resource_request.number_of_nodes = 1
  resource_request.processors_per_node = number_of_threads
  resource_request.memory_in_mb = 24000
  resource_request.virtual_memory_in_mb = 24000
  job = himster.Job(resource_request, './runLmdFit.sh', 'runLmdFit', elastic_data_path + '/runLmdFit_pbs.log')
  job.setJobArraySize(1, 1) 

  job.addExportedUserVariable('config_url', args.config_url[0])
  job.addExportedUserVariable('data_path', elastic_data_path)
  job.addExportedUserVariable('acceptance_resolution_path', acc_res_data_path)
  job.addExportedUserVariable('number_of_threads', number_of_threads)
  if args.ref_box_gen_data != '':
    job.addExportedUserVariable('reference_acceptance_path', args.ref_box_gen_data)

  joblist.append(job)
  
# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = himster.HimsterJobManager(20)

job_manager.submitJobsToHimster(joblist)
job_manager.manageJobs()
