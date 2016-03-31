import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []

glob_pattern = 'lmd_fitted_vertex_data.root'

def getListOfDirectories(path):
  if os.path.isdir(path):
    print 'looking at path ' + path
    
    if os.path.split(path)[1] == 'mc_data':
      return  
    
    for dir in os.listdir(path):
      bunch_dirs = glob.glob(path + '/' + dir + '/bunches_*/merge_data')
      if bunch_dirs:
        for bunch_dir in bunch_dirs:
          filelists = glob.glob(bunch_dir + '/' + glob_pattern)
          if filelists:
            if re.search('uncut', bunch_dir):
              dirs.append(bunch_dir)
              return
      else:
        if glob.glob(path + '/' + dir + '/Lumi_TrksQA_*.root'):
          return
        dirpath = path + '/' + dir
        if os.path.isdir(dirpath):
          getListOfDirectories(dirpath)


parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')

args = parser.parse_args()
  
getListOfDirectories(args.dirname[0])

bashcommand = default = os.getenv('VMCWORKDIR') + '/build/bin/plotIPDistribution'
for dir in dirs:
  bashcommand += ' ' + dir

returnvalue = subprocess.call(bashcommand.split())
