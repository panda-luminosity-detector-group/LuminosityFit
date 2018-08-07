import os, sys, re, errno, glob, time, glob
import subprocess
import general
import argparse

class DataTypeInfo:
  def __init__(self, data_type, pattern, glob_pattern):
    self.data_type = data_type;
    self.pattern = pattern;
    self.glob_pattern = glob_pattern;


parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('type', metavar='type', type=str, nargs=1,
                    help='type of data to create (a = angular, e = efficiency, r = resolution, v = vertex/ip')
parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')
parser.add_argument('--dir_pattern', metavar='path name pattern', type=str, default='.*', help='')
parser.add_argument('--num_samples', metavar='num_samples', type=int, default=1, help='')
parser.add_argument('--sample_size', metavar='sample_size', type=int, default=0, help='')

args = parser.parse_args()

data_type_list = []

if args.type[0].find('a') >= 0:
  data_type_list.append(DataTypeInfo('a', ' -f lmd_data_\\d*.root', 'lmd_data_*.root'))  
if args.type[0].find('e') >= 0:
  data_type_list.append(DataTypeInfo('e', ' -f lmd_acc_data_\\d*.root', 'lmd_acc_data_*.root'))  
if args.type[0].find('r') >= 0:
  data_type_list.append(DataTypeInfo('r', ' -f lmd_res_data_\\d*.root', 'lmd_res_data_*.root'))
if args.type[0].find('h') >= 0:
  data_type_list.append(DataTypeInfo('h', ' -f lmd_res_data_\\d*.root', 'lmd_res_data_*.root'))  
if args.type[0].find('v') >= 0: 
  data_type_list.append(DataTypeInfo('v', ' -f lmd_vertex_data_\\d*.root', 'lmd_vertex_data_*.root'))  


patterns=[]
patterns.append(args.dir_pattern)
# to avoid merge_data directories to be used recursively 
# we forbid the occurence merge_data in path name
dir_searcher = general.DirectorySearcher(patterns, 'merge_data')

for data_type_info in data_type_list:
  dir_searcher.searchListOfDirectories(args.dirname[0], data_type_info.glob_pattern)
  dirs = dir_searcher.getListOfDirectories()    
  for dir in dirs:
    print('starting merge for ' + dir)
    bashcommand = default = os.getenv('LMDFIT_BUILD_PATH') + '/bin/mergeLmdData -p ' + dir + ' -t ' + data_type_info.data_type + ' -n ' + str(args.num_samples) + ' -s ' + str(args.sample_size) + data_type_info.pattern 
    print(bashcommand)    
    returnvalue = subprocess.call(bashcommand.split())
