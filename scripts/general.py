import os, sys, re, errno, glob
import json

def refactorSimulationParameters(directory):
    # read file
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

class DirectorySearcher:    
    def __init__(self, pattern_):
        self.pattern = pattern_
        self.dirs = []
        
    def getListOfDirectories(self):
        return self.dirs
    
    def searchListOfDirectories(self, path, glob_pattern):
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
              shutil.move(path + '/' + mc_filename, path + '/mc_data/' + mc_filename)
            for param_file in param_files:
              param_filename = os.path.split(param_file)[1]
              shutil.move(path + '/' + param_filename, path + '/mc_data/' + param_filename)
            print 'successfully moved all mc files there!'
        
        
          
        files = glob.glob(path + '/' + glob_pattern)
        if files:
          m = re.search(self.pattern, path)
          if m:
            self.dirs.append(path)
          
        # if we did not get any exit criterium then change into subdirectories
        for dir in next(os.walk(path))[1]:
          dirpath = path + '/' + dir
          if os.path.isdir(dirpath):
            self.searchListOfDirectories(dirpath, glob_pattern)            

            
class ConfigModifier:
    def __init__(self):
        pass
    def loadConfig(self, config_file_path):
        f = open(config_file_path, 'r')
        return json.loads(f.read())
        
    def writeConfigToPath(self, config, config_file_path):
        f = open(config_file_path, 'w')
        f.write(json.dumps(config, indent=2, separators=(',', ': ')))