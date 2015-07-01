import himster
import os, sys, re, errno, glob
import shutil

lib_path = os.path.abspath('argparse-1.2.1/build/lib')
sys.path.append(lib_path)

import argparse

dirs = []

pattern = ''


def refactorSimulationParameters(directory):
    #read file
    f = open(directory + '/sim_beam_prop.config', 'r')
    text = f.read()
    f.close()
    os.remove(directory + '/sim_beam_prop.config')
    if text.find("ip_offset_x") != -1:
      # ok this seems to be an old parameter file so read all the values
      # and the write them out in the new format
      text = re.sub('ip_offset_x', 'ip_mean_x', text);
      text = re.sub('ip_offset_y', 'ip_mean_y', text);
      text = re.sub('ip_offset_z', 'ip_mean_z', text);
      text = re.sub('ip_spread_x', 'ip_standard_deviation_x', text);
      text = re.sub('ip_spread_y', 'ip_standard_deviation_y', text);
      text = re.sub('ip_spread_z', 'ip_standard_deviation_z', text);
      
      text = re.sub('beam_gradient_x', 'beam_tilt_x', text);
      text = re.sub('beam_gradient_y', 'beam_tilt_y', text);
      text = re.sub('beam_emittance_x', 'beam_divergence_x', text);
      text = re.sub('beam_emittance_y', 'beam_divergence_y', text);
      
     
      f = open(directory + '/sim_params.config', 'w')
      f.write(text)
      f.close()

def getListOfDirectories(path, force):
  if os.path.isdir(path):
    print 'currently looking at directory ' + path
    
    sim_params = glob.glob(path + '/sim_beam_prop.config')
    if sim_params:
        refactorSimulationParameters(path)
    
    if os.path.split(path)[1] == 'mc_data':
      return
    
    # check if this directory has MC files
    mc_files = glob.glob(path + '/Lumi_MC_*.root')
    param_files = glob.glob(path + '/Lumi_Params_*.root')
    # if that is the case, their parent folder has to be named mc_data
    if mc_files or param_files:
      if not os.path.split(path)[1] == 'mc_data':
        print 'Found mc files in ' + path + '. Since they are not inside a folder mc_data, proposing to create mc_data dir and moving files there.'
        try:
          os.mkdir(path + '/mc_data')
        except OSError as exception:
          if exception.errno != errno.EEXIST:
            print 'dont need to create mc_data dir...'
        for mc_file in mc_files:
          mc_filename = os.path.split(mc_file)[1]
          shutil.move(path+'/'+mc_filename, path+'/mc_data/'+mc_filename)
        for param_file in param_files:
          param_filename = os.path.split(param_file)[1]
          shutil.move(path+'/'+param_filename, path+'/mc_data/'+param_filename)
        print 'successfully moved all mc files there!'
    
    bunch_dirs = glob.glob(path + '/bunches_*')
    if bunch_dirs:
      for bunch_dir in bunch_dirs:
        if not force:
          filelists = glob.glob(bunch_dir + '/lmd_*data_*.root')
        else:
          filelists = []
        if not filelists:
          filelists = glob.glob(bunch_dir + '/filelist_*.txt')
          if filelists:
            m = re.search(pattern, bunch_dir)
            if m:
              dirs.append(bunch_dir)
              print "using this directory!"
        return
    else:
      if glob.glob(path + '/Lumi_TrksQA_*.root'):
        return

    # if we did not get any exit criterium then change into subdirectories    
    for dir in os.listdir(path):
      dirpath = path + '/' + dir
      if os.path.isdir(dirpath):
        getListOfDirectories(dirpath, force)


parser = argparse.ArgumentParser(description='Script for going through whole directory trees and looking for bunches directories with filelists in them creating lmd data objects.', formatter_class=argparse.RawTextHelpFormatter)


parser.add_argument('lab_momentum', metavar='lab_momentum', type=float, nargs=1, help='lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)')
parser.add_argument('type', metavar='type', type=str, nargs=1,
                    help='type of data to create (a = angular, e = efficiency, r = resolution, v = vertex/ip')
parser.add_argument('dirname', metavar='dirname_to_scan', type=str, nargs=1,
                    help='Name of directory to scan recursively for qa files and create bunches')
parser.add_argument('config_url', metavar='config_url', type=str, nargs=1,
                    help='Path to data config file in json format.')

parser.add_argument('--dir_pattern', metavar='path name pattern', type=str, default='.*', help='')
parser.add_argument('--force', action='store_true', help='number of events to use')
parser.add_argument('--num_events', metavar='num_events', type=int, default=0, help='number of events to use')
parser.add_argument('--elastic_cross_section', metavar='elastic_cross_section', type=float, default=1.0, help='Total elastic cross section. Relevant for luminosity extraction performance tests!')

args = parser.parse_args()

pattern = args.dir_pattern

failed_submit_commands = []

getListOfDirectories(args.dirname[0], args.force)


joblist = []

for dir in dirs:
  num_filelists = len(glob.glob(dir + '/filelist_*.txt'))

  input_path = os.path.split(dir)[0]
  filelist_path = dir
  output_path = filelist_path

  resource_request = himster.JobResourceRequest(2*60)
  resource_request.number_of_nodes=1
  resource_request.processors_per_node=1
  resource_request.memory_in_mb=2000
  resource_request.virtual_memory_in_mb=2000
  job = himster.Job(resource_request, './createLumiFitData.sh', 'createLumiFitData', output_path + '/createLumiFitData_pbs.log')
  job.setJobArraySize(1, num_filelists)
  
  job.addExportedUserVariable('numEv', args.num_events)
  job.addExportedUserVariable('pbeam', args.lab_momentum[0])
  job.addExportedUserVariable('input_path', input_path)
  job.addExportedUserVariable('filelist_path', filelist_path)
  job.addExportedUserVariable('output_path', output_path)
  job.addExportedUserVariable('config_path', args.config_url[0])
  job.addExportedUserVariable('type', args.type[0])
  job.addExportedUserVariable('elastic_cross_section', args.elastic_cross_section)
  
  joblist.append(job)

# job threshold of this type (too many jobs could generate to much io load
# as quite a lot of data is read in from the storage...)
job_manager = himster.HimsterJobManager(1600)

job_manager.submitJobsToHimster(joblist)
job_manager.manageJobs()