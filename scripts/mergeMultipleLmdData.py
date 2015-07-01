import os, sys, re, errno, glob, time, glob
import subprocess
import multiprocessing

cpu_cores = multiprocessing.cpu_count()

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []

dir_pattern = ''

class DataTypeInfo:
  def __init__(self, data_type, pattern, glob_pattern):
    self.data_type =  data_type;
    self.pattern = pattern;
    self.glob_pattern = glob_pattern;
  
def getListOfDirectories(path, glob_pattern):
  if os.path.isdir(path):
    print 'looking at path ' + path
    
    if os.path.split(path)[1] == 'mc_data':
      return
  
    for dir in os.listdir(path):
      bunch_dirs = glob.glob(path + '/' + dir + '/bunches_*')
      if bunch_dirs:
        for bunch_dir in bunch_dirs:
          filelists = glob.glob(bunch_dir + '/' + glob_pattern)
          if filelists:
            m = re.search(dir_pattern, bunch_dir)
            print m
            if m:
              dirs.append(bunch_dir)
              return
      else:
        if glob.glob(path + '/Lumi_TrksQA_*.root'):
          return
      dirpath = path + '/' + dir
      if os.path.isdir(dirpath):
        getListOfDirectories(dirpath, glob_pattern)


parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('type', metavar='type', type=str, nargs=1,
                    help='type of data to create (a = angular, e = efficiency, r = resolution, v = vertex/ip')
parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')
parser.add_argument('--dir_pattern', metavar='path name pattern', type=str, default='.*', help='')

args = parser.parse_args()

dir_pattern = args.dir_pattern

data_type_list = []

if args.type[0].find('a') >= 0:
  data_type_list.append(DataTypeInfo('a', ' -f lmd_data_\\d*.root', 'lmd_data_*.root'))  
if args.type[0].find('e') >= 0:
  data_type_list.append(DataTypeInfo('e', ' -f lmd_acc_data_\\d*.root', 'lmd_acc_data_*.root'))  
if args.type[0].find('r') >= 0:
  data_type_list.append(DataTypeInfo('r', ' -f lmd_res_data_\\d*.root', 'lmd_res_data_*.root'))  
if args.type[0].find('v') >= 0: 
  data_type_list.append(DataTypeInfo('v', ' -f lmd_vertex_data_\\d*.root', 'lmd_vertex_data_*.root'))  


for data_type_info in data_type_list:
  dirs = []
  getListOfDirectories(args.dirname[0], data_type_info.glob_pattern)    
  for dir in dirs:
    bashcommand = default=os.getenv('VMCWORKDIR') + '/build/bin/mergeLmdData -p ' + dir + ' -t ' + data_type_info.data_type + data_type_info.pattern 
                    
    returnvalue = subprocess.call(bashcommand.split())
