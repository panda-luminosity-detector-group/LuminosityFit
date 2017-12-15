import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []

glob_pattern = 'lmd_fitted_data.root'
pattern = ''

def getListOfDirectories(path):
  if os.path.isdir(path):
      
    if os.path.split(path)[1] == 'mc_data':
      return
      
    for dir in os.listdir(path):
      bunch_dirs = glob.glob(path + '/bunches_*/merge_data')
      if bunch_dirs:
        for bunch_dir in bunch_dirs:
          filelists = glob.glob(bunch_dir + '/' + glob_pattern)
          if filelists:
            m = re.search(pattern, bunch_dir)
            if m:
              dirs.append(bunch_dir)
        return
      else:
        if glob.glob(path + '/Lumi_TrksQA_*.root'):
          return
      dirpath = path + '/' + dir
      if os.path.isdir(dirpath):
        getListOfDirectories(dirpath)


parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')

parser.add_argument('dirname_pattern', metavar='directory pattern', type=str, nargs=1,
                    help='Only found directories with this pattern are used!')

args = parser.parse_args()

pattern = args.dirname_pattern[0]  

getListOfDirectories(args.dirname[0])

bashcommand = default = os.getenv('VMCWORKDIR') + '/build/bin/plotLumiFitResults -f ' + args.dirname_pattern[0]

for dir in dirs:
  bashcommand += ' ' + dir

# print bashcommand
returnvalue = subprocess.call(bashcommand.split())
