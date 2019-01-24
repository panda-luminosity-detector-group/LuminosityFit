import os
import subprocess
import argparse
import general

glob_pattern = 'gen_mc.root'
pattern = ''

parser = argparse.ArgumentParser(
    description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for lmd data files and call merge!')

parser.add_argument('dirname_pattern', metavar='directory pattern', type=str, nargs=1,
                    help='Only found directories with this pattern are used!')

parser.add_argument('output_path', type=str, nargs=1,
                    help='Path to the output directory')


args = parser.parse_args()

pattern = args.dirname_pattern[0]

dir_searcher = general.DirectorySearcher([pattern])

dir_searcher.searchListOfDirectories(args.dirname[0], glob_pattern)
dirs = dir_searcher.getListOfDirectories()

if len(dirs) == 0:
    raise FileNotFoundError("No files found!!")

os.makedirs(args.output_path[0], exist_ok=True)

bashcommand = os.getenv('LMDFIT_BUILD_PATH') + '/bin/plotLumiFitResults -f '\
    + args.dirname_pattern[0] + ' -o ' + args.output_path[0]

for dir in dirs:
    bashcommand += ' ' + dir

returnvalue = subprocess.call(bashcommand.split())
