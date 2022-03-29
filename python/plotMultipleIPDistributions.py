# TODO: port to python3, add shebang

import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing
import general

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []

glob_pattern = 'lmd_fitted_vertex_data.root'

parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')
parser.add_argument('--dir_pattern', metavar='path name pattern', type=str, default='.*', help='')

args = parser.parse_args()
  
patterns=[]
patterns.append(args.dir_pattern)
dir_searcher = general.DirectorySearcher(patterns)

dir_searcher.searchListOfDirectories(args.dirname[0], glob_pattern)
dirs = dir_searcher.getListOfDirectories()    

bashcommand = default = os.getenv('LMDFIT_BUILD_PATH') + '/bin/plotIPDistribution'
for dir in dirs:
  bashcommand += ' ' + dir

print bashcommand

returnvalue = subprocess.call(bashcommand.split())
