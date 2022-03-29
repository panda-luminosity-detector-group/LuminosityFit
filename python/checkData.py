
# TODO: port to python3, add shebang
import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing
import general, himster

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []
data_glob_pattern = 'createLumiFitData_pbs.log-*'

def checkData(directory):
  faulty_files = []
  files = glob.glob(str(directory) + '/' + data_glob_pattern)
  for file in files:
    text = open(file).read()
    if 'Error in <TChain::LoadTree>:' in text:
      faulty_files.append(file)
    elif 'root has no keys' in text:
      faulty_files.append(file)
  return faulty_files
  
parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')

parser.add_argument('dirname_pattern', metavar='directory pattern', type=str, nargs=1,
                    help='Only found directories with this pattern are used!')

args = parser.parse_args()

patterns = [args.dirname_pattern[0]]
dir_searcher = general.DirectorySearcher(patterns)

dir_searcher.searchListOfDirectories(args.dirname[0], data_glob_pattern)
dirs = dir_searcher.getListOfDirectories()
  
for dir in dirs:
  faulty_files = checkData(dir)
  if faulty_files:
    print 'we found ' + str(len(faulty_files)) + ' for directory ' + str(dir)