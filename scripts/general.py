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

def refactor2SimulationParameters(directory):
    # read file
    f = open(directory + '/sim_params.config', 'r')
    text = f.read()
    f.close()
    
    newtext = text
    #ok in case the simulation parameter file is just a bit ill formatted fix that
    newtext = fixLine(newtext, 'ip_mean_x')
    newtext = fixLine(newtext, 'ip_mean_y')
    newtext = fixLine(newtext, 'ip_mean_z')
    newtext = fixLine(newtext, 'ip_standard_deviation_x')
    newtext = fixLine(newtext, 'ip_standard_deviation_y')
    newtext = fixLine(newtext, 'ip_standard_deviation_z')
    newtext = fixLine(newtext, 'beam_tilt_x')
    newtext = fixLine(newtext, 'beam_tilt_y')
    newtext = fixLine(newtext, 'beam_divergence_x')
    newtext = fixLine(newtext, 'beam_divergence_y')
    
    if newtext != text:
      print 'fixing sim params file'
      os.remove(directory + '/sim_params.config')
      f = open(directory + '/sim_params.config', 'w')
      f.write(newtext)
      f.close()
      

def fixLine(text, search_str):
    p = re.compile('^' + str(search_str) + '=\d*\.\d*$', re.MULTILINE)
    m = p.search(text)
    if not m:
        p = re.compile('^\s*(' + str(search_str) + ')\s*=\s*(\d*\.\d*)\s*$', re.MULTILINE)
        mm = p.search(text)
        if mm:
            text = re.sub(p, mm.group(1)+'='+mm.group(2), text)
        else:
            p = re.compile('^\s*(' + str(search_str) + ')\s*=\s*(\d*\.\d*)\s*', re.MULTILINE)
            mm = p.search(text)
            if mm:
                p = re.compile('^\s*' + str(search_str) + '\s*=\s*\d*\.\d*\s*', re.MULTILINE)
                text = re.sub(p, mm.group(1)+'='+mm.group(2)+'\n', text)
                
    
    return text

class DirectorySearcher:    
    def __init__(self, patterns_, not_contain_pattern_ = ''):
        self.patterns = patterns_
        self.not_contain_pattern = not_contain_pattern_
        self.dirs = []
        
    def getListOfDirectories(self):
        return self.dirs
    
    def searchListOfDirectories(self, path, glob_pattern):
        #if os.path.isdir(path):
          #print 'currently looking at directory ' + path
    
        sim_params = glob.glob(path + '/sim_beam_prop.config')
        if sim_params:
            refactorSimulationParameters(path)
            
        sim_params = glob.glob(path + '/sim_params.config')
        if sim_params:
            refactor2SimulationParameters(path)
    
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
        
        if self.not_contain_pattern is not '':
          m = re.search(self.not_contain_pattern, path)
          if m:
            return;
        files = glob.glob(path + '/' + glob_pattern)
        if files:
          is_good = True
          #print path
          for pattern in self.patterns:
            m = re.search(pattern, path)
            if not m:
              is_good = False
              break
          if is_good:
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