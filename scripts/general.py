import os
import re
import errno
import glob
import json


def addGeneralArgumentsToParser(parser):
    parser.add_argument('num_events_per_sample',
                        metavar='num_events_per_sample',
                        type=int, nargs=1,
                        help='number of events per sample to simulate')
    parser.add_argument('num_samples', metavar='num_samples',
                        type=int, nargs=1,
                        help='number of samples to simulate')
    parser.add_argument('lab_momentum', metavar='lab_momentum', type=float,
                        nargs=1,
                        help='lab momentum of incoming beam antiprotons\n'
                        '(required to set correct magnetic field maps etc)')

    parser.add_argument('--low_index', metavar='low_index', type=int,
                        help='Index of first sample and lowest index of '
                        'job array.')

    parser.add_argument('--lmd_detector_geometry_filename',
                        metavar='lmd_detector_geometry_filename', type=str,
                        help='Filename of the Geant Luminosity Detector '
                        'geometry file in the pandaroot geometry subfolder')

    return parser


def addDebugArgumentsToParser(parser):
    parser.add_argument('--force_level', metavar='force_level', type=int,
                        default=0,
                        help='force level 0: if directories exist with data '
                        'files no new simulation is started\n'
                        'force level 1: will do full reconstruction even if '
                        'this data already exists, but not geant simulation\n'
                        'force level 2: resimulation of everything!')

    parser.add_argument('--use_devel_queue', action='store_true',
                        help='If flag is set, the devel queue is used')
    parser.add_argument('--debug', action='store_true',
                        help='If flag is set, the simulation runs locally for '
                        'debug purposes')

    return parser


def createGeneralRunParameters(num_events_per_sample, num_samples,
                               lab_momentum):
    return {
        'num_events_per_sample': num_events_per_sample,
        'num_samples': num_samples,
        'lab_momentum': lab_momentum,
        'low_index': 1,
        'output_dir': '',
        'lmd_geometry_filename': 'Luminosity-Detector.root'
    }


def addGeneralRunParametersToConfig(params, args):
    run_params = createGeneralRunParameters(
        args.num_events_per_sample[0],
        args.num_samples[0], args.lab_momentum[0])
    if args.low_index:
        run_params['low_index'] = args.low_index
    if args.output_dir:
        run_params['output_dir'] = args.output_dir,
    if args.lmd_detector_geometry_filename:
        run_params['lmd_geometry_filename'
                   ] = args.lmd_detector_geometry_filename,

    if args.debug and run_params['num_samples'] > 1:
        print("Warning: number of samples in debug mode is limited to 1!"
              " Setting to 1!")
        run_params['num_samples'] = 1
    from copy import copy
    new_params = copy(params)
    new_params.update(run_params)
    return new_params


def refactorSimulationParameters(directory):
    # read file
    f = open(directory + '/sim_beam_prop.config', 'r')
    text = f.read()
    f.close()
    os.remove(directory + '/sim_beam_prop.config')

    if text.find("ip_offset_x") != -1:
        # ok this seems to be an old parameter file so read all the values
        # and the write them out in the new format
        text = re.sub('ip_offset_x', 'ip_mean_x', text)
        text = re.sub('ip_offset_y', 'ip_mean_y', text)
        text = re.sub('ip_offset_z', 'ip_mean_z', text)
        text = re.sub('ip_spread_x', 'ip_standard_deviation_x', text)
        text = re.sub('ip_spread_y', 'ip_standard_deviation_y', text)
        text = re.sub('ip_spread_z', 'ip_standard_deviation_z', text)

        text = re.sub('beam_gradient_x', 'beam_tilt_x', text)
        text = re.sub('beam_gradient_y', 'beam_tilt_y', text)
        text = re.sub('beam_emittance_x', 'beam_divergence_x', text)
        text = re.sub('beam_emittance_y', 'beam_divergence_y', text)

        f = open(directory + '/sim_params.config', 'w')
        f.write(text)
        f.close()


def refactor2SimulationParameters(directory):
    # read file
    f = open(directory + '/sim_params.config', 'r')
    text = f.read()
    f.close()

    newtext = text
    # ok in case the simulation parameter file is just a bit ill formatted fix that
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
        print('fixing sim params file')
        os.remove(directory + '/sim_params.config')
        f = open(directory + '/sim_params.config', 'w')
        f.write(newtext)
        f.close()


def fixLine(text, search_str):
    p = re.compile('^' + str(search_str) + '=\d*\.\d*$', re.MULTILINE)
    m = p.search(text)
    if not m:
        p = re.compile('^\s*(' + str(search_str) +
                       ')\s*=\s*(\d*\.\d*)\s*$', re.MULTILINE)
        mm = p.search(text)
        if mm:
            text = re.sub(p, mm.group(1)+'='+mm.group(2), text)
        else:
            p = re.compile('^\s*(' + str(search_str) +
                           ')\s*=\s*(\d*\.\d*)\s*', re.MULTILINE)
            mm = p.search(text)
            if mm:
                p = re.compile('^\s*' + str(search_str) +
                               '\s*=\s*\d*\.\d*\s*', re.MULTILINE)
                text = re.sub(p, mm.group(1)+'='+mm.group(2)+'\n', text)

    return text


def fixMCData():
            # check if this directory has MC files
    mc_files = glob.glob(path + '/Lumi_MC_*.root')
    param_files = glob.glob(path + '/Lumi_Params_*.root')
    # if that is the case, their parent folder has to be named mc_data
    if mc_files or param_files:
        if not os.path.split(path)[1] == 'mc_data':
            import shutil
            print('Found mc files in ' + path +
                  '. Since they are not inside a folder mc_data, proposing to create mc_data dir and moving files there.')
            try:
                os.mkdir(path + '/mc_data')
            except OSError as exception:
                if exception.errno != errno.EEXIST:
                    print('dont need to create mc_data dir...')
            for mc_file in mc_files:
                mc_filename = os.path.split(mc_file)[1]
                shutil.move(path + '/' + mc_filename,
                            path + '/mc_data/' + mc_filename)
            for param_file in param_files:
                param_filename = os.path.split(param_file)[1]
                shutil.move(path + '/' + param_filename,
                            path + '/mc_data/' + param_filename)
            print('successfully moved all mc files there!')


class DirectorySearcher:
    def __init__(self, patterns_, not_contain_pattern_=''):
        self.patterns = patterns_
        self.not_contain_pattern = not_contain_pattern_
        self.dirs = []

    def getListOfDirectories(self):
        return self.dirs

    def searchListOfDirectories(self, path, glob_patterns):
        #print("looking for files with pattern: ", glob_patterns)
        #print("dirpath forbidden patterns:", self.not_contain_pattern)
        #print("dirpath patterns:", self.patterns)
        file_patterns = [glob_patterns]
        if isinstance(glob_patterns, list):
            file_patterns = glob_patterns

        for dirpath, dirs, files in os.walk(path):
            #print('currently looking at directory', dirpath)
            if dirpath == 'mc_data' or dirpath == 'Pairs':
                continue

            # first check if dirpath does not contain pattern
            if self.not_contain_pattern is not '':
                m = re.search(self.not_contain_pattern, dirpath)
                if m:
                    continue

            is_good = True
            # print path
            for pattern in self.patterns:
                m = re.search(pattern, dirpath)
                if not m:
                    is_good = False
                    break
            if is_good:
                # check if there are useful files here
                if len(file_patterns) == 1:
                    found_files = [x for x in files if glob_patterns in x]
                else:
                    for filename in files:
                        found_file = True
                        for pattern in file_patterns:
                            if pattern not in filename:
                                found_file = False
                                break
                        if found_file:
                            found_files = True
                            break
                
                if found_files:
                    self.dirs.append(dirpath)


class ConfigModifier:
    def __init__(self):
        pass

    def loadConfig(self, config_file_path):
        f = open(config_file_path, 'r')
        return json.loads(f.read())

    def writeConfigToPath(self, config, config_file_path):
        f = open(config_file_path, 'w')
        f.write(json.dumps(config, indent=2, separators=(',', ': ')))
