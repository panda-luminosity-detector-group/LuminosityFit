import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing
import general

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')
parser.add_argument('dir_pattern', metavar='dir_pattern', type=str, nargs=1,
                    help='directory name pattern')
parser.add_argument('config_url', metavar='config_url', type=str, nargs=1,
                    help='url to fit config json file')

args = parser.parse_args()
  
patterns=[]
patterns.append(args.dir_pattern[0])
dir_searcher = general.DirectorySearcher(patterns)

print 'searching for directories...'
dir_searcher.searchListOfDirectories(args.dirname[0], 'lmd_vertex_data_*of1.root')
dirs = dir_searcher.getListOfDirectories()    

print dirs
for dir in dirs:
  bashcommand = default = os.getenv('LMDFIT_BUILD_PATH') + '/bin/determineBeamOffset -m 4 -p ' + dir + ' -c ' + args.config_url[0]
  returnvalue = subprocess.call(bashcommand.split())
