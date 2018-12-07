import os
import subprocess
import sys
import errno
import glob
import json
import random

import himster
import reconstruction as rec
import alignment


def check_ip_params_zero(ip_params):
    print("check zero...")
    for key, value in ip_params.items():
        if value != 0.0:
            return False
    return True


simulation_types = ['box', 'dpm_elastic',
                    'dpm_elastic_inelastic', 'noise']


def addArgumentsToParser(parser):
    parser.add_argument('sim_type', metavar='simulation_type', type=str,
                        nargs=1,
                        choices=simulation_types,
                        help='Simulation type which can be one of the '
                        'following: box, dpm_elastic, dpm_elastic_inelastic, '
                        'noise.\nThis information is used to automatically '
                        'obtain the generator data and output naming scheme.')

    parser.add_argument('--theta_min', metavar='theta_min', type=float,
                        help='Minimal value of the scattering angle theta in '
                        'mrad. Default: 2.7')
    parser.add_argument('--theta_max', metavar='theta_max', type=float,
                        help='Maximal value of the scattering angle theta in '
                        'mrad. Default: 13.0')
    parser.add_argument('--neglect_recoil_momentum', action='store_false',
                        help='If recoil momentum should not be subtracted from'
                        ' scattered antiprotons. Only for box generator')
    parser.add_argument('--random_seed', metavar='random_seed', type=int,
                        help='random seed used in data generation and mc '
                        'simulation')

    parser.add_argument('--output_dir', metavar='output_dir', type=str,
                        help='This directory is used for the '
                        'output.\nDefault is the generator directory as a '
                        'prefix, with beam offset infos etc. added')

    parser.add_argument('--use_ip_offset',
                        metavar=("ip_offset_x", "ip_offset_y", "ip_offset_z",
                                 "ip_spread_x", "ip_spread_y", "ip_spread_z"),
                        type=float, nargs=6,
                        help="ip_offset_x: interaction vertex mean X position "
                        "(in cm)\n"
                        "ip_offset_y: interaction vertex mean Y position "
                        "(in cm)\n"
                        "ip_offset_z: interaction vertex mean Z position "
                        "(in cm)\n"
                        "ip_spread_x: interaction vertex X position "
                        "distribution width (in cm)\n"
                        "ip_spread_y: interaction vertex Y position "
                        "distribution width (in cm)\n"
                        "ip_spread_z: interaction vertex Z position "
                        "distribution width (in cm)")

    parser.add_argument('--use_beam_gradient',
                        metavar=("beam_gradient_x", "beam_gradient_y",
                                 "beam_emittance_x", "beam_emittance_y"),
                        type=float, nargs=4,
                        help="beam_gradient_x: mean beam inclination on target"
                        " in x direction dPx/dPz (in rad)\n"
                        "beam_gradient_y: mean beam inclination on target in y"
                        " direction dPy/dPz (in rad)\n"
                        "beam_divergence_x: beam divergence in x direction "
                        "(in rad)\n"
                        "beam_divergence_y: beam divergence in y direction "
                        "(in rad)")
    return parser


def createSimulationParameters(sim_type):
    if sim_type not in simulation_types:
        raise ValueError("specified simulation type is invalid!")
    sim_params = {
        'sim_type': sim_type,
        'theta_min_in_mrad': 2.7,
        'theta_max_in_mrad': 13.0,
        'neglect_recoil_momentum': False,
        'random_seed': random.randint(10, 9999)
    }
    ip_params = {
        'ip_offset_x': 0.0,  # in cm
        'ip_offset_y': 0.0,  # in cm
        'ip_offset_z': 0.0,  # in cm
        'ip_spread_x': 0.0,  # in cm
        'ip_spread_y': 0.0,  # in cm
        'ip_spread_z': 0.0,  # in cm

        'beam_tilt_x': 0.0,  # in rad
        'beam_tilt_y': 0.0,  # in rad
        'beam_divergence_x': 0.0,  # in rad
        'beam_divergence_y': 0.0  # in rad
    }
    sim_params['ip_params'] = ip_params
    return sim_params


def addSimulationParametersToConfig(params, args):
    sim_params = createSimulationParameters(args.sim_type[0])
    if args.theta_min:
        sim_params['theta_min_in_mrad'] = args.theta_min
    if args.theta_max:
        sim_params['theta_max_in_mrad'] = args.theta_max
    if args.neglect_recoil_momentum:
        sim_params['neglect_recoil_momentum'] = not args.neglect_recoil_momentum
    if args.random_seed:
        sim_params['random_seed'] = args.random_seed

    if args.use_ip_offset:
        ip_params = {
            'ip_offset_x': args.use_ip_offset[0],  # in cm
            'ip_offset_y': args.use_ip_offset[1],  # in cm
            'ip_offset_z': args.use_ip_offset[2],  # in cm
            'ip_spread_x': args.use_ip_offset[3],  # in cm
            'ip_spread_y': args.use_ip_offset[4],  # in cm
            'ip_spread_z': args.use_ip_offset[5],  # in cm
        }
        sim_params['ip_params'].update(ip_params)
    if args.use_beam_gradient:
        ip_params = {
            'beam_tilt_x': args.use_beam_gradient[0],  # in rad
            'beam_tilt_y': args.use_beam_gradient[1],  # in rad
            'beam_divergence_x': args.use_beam_gradient[2],  # in rad
            'beam_divergence_y': args.use_beam_gradient[3]  # in rad
        }
        sim_params['ip_params'].update(ip_params)

    from copy import copy
    new_params = copy(params)
    new_params.update(sim_params)
    return new_params


def generateDirectory(sim_params, align_params, output_dir=''):
    if output_dir == '':
        # generate output directory name
        # lets generate a folder structure based on the input
        dirname = 'plab_' + str(sim_params['lab_momentum']) + 'GeV'
        gen_part = sim_params['sim_type'] + '_theta_' + \
            str(sim_params['theta_min_in_mrad'])
        # if sim_params['sim_type'] is 'box':
        gen_part += '-' + str(sim_params['theta_max_in_mrad'])
        gen_part += 'mrad'
        if not sim_params['neglect_recoil_momentum']:
            gen_part += '_recoil_corrected'

        dirname += '/' + gen_part

        if not check_ip_params_zero(sim_params['ip_params']):
            dirname += '/ip_offset_XYZDXDYDZ_'\
                + str(sim_params['ip_params']['ip_offset_x'])+'_'\
                + str(sim_params['ip_params']['ip_offset_y'])+'_'\
                + str(sim_params['ip_params']['ip_offset_z'])+'_'\
                + str(sim_params['ip_params']['ip_spread_x'])+'_'\
                + str(sim_params['ip_params']['ip_spread_y'])+'_'\
                + str(sim_params['ip_params']['ip_spread_z'])

            dirname += '/beam_grad_XYDXDY_'\
                + str(sim_params['ip_params']['beam_tilt_x'])+'_'\
                + str(sim_params['ip_params']['beam_tilt_y'])+'_'\
                + str(sim_params['ip_params']['beam_divergence_x'])+'_'\
                + str(sim_params['ip_params']['beam_divergence_y'])

        # dirname += '/' + str(os.path.splitext(sim_params['lmd_geometry_filename'])[0])
        if (align_params['use_point_transform_misalignment'] or
                align_params['misalignment_matrices_path'] == ''):
            dirname += '/no_geo_misalignment'
        else:
            dirname += '/geo_misalignment' + \
                str(os.path.splitext(os.path.basename(
                    align_params['misalignment_matrices_path']))[0])

        dirname += '/' + str(sim_params['num_events_per_sample'])
    else:
        dirname = output_dir

    return dirname


def startSimulationAndReconstruction(sim_params, align_params, reco_params,
                                     output_dir='', force_level=0, debug=False,
                                     use_devel_queue=False):
    print('preparing simulations in index range '
          + str(sim_params['low_index']) + ' - '
          + str(sim_params['low_index']+sim_params['num_samples']-1))

    if debug and sim_params['num_samples'] > 1:
        print("Warning: number of samples in debug mode is limited to 1! "
              "Setting to 1!")
        sim_params['num_samples'] = 1

    dirname = generateDirectory(sim_params, align_params, output_dir)
    dirname_filter_suffix = rec.generateRecoDirSuffix(
        reco_params, align_params)

    low_index_used = sim_params['low_index']
    num_samples = sim_params['num_samples']

    pathname_base = os.getenv('LMDFIT_DATA_DIR') + '/' + dirname
    path_mc_data = pathname_base + '/mc_data'
    dirname_full = dirname + '/' + dirname_filter_suffix
    pathname_full = os.getenv('LMDFIT_DATA_DIR') + '/' + dirname_full

    print('using output folder structure: ' + pathname_full)

    try:
        os.makedirs(pathname_full)
        os.makedirs(pathname_full+'/Pairs')
        os.makedirs(path_mc_data)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            print('error: thought dir does not exists but it does...')

    min_file_size = 3000  # in bytes
    if force_level == 0:
        # check if the directory already has the reco data in it
        reco_files = glob.glob(pathname_full + '/Lumi_TrksQA_*.root')
        total_requested_jobs = num_samples
        reco_files = [
            x for x in reco_files if os.path.getsize(x) > min_file_size]
        if total_requested_jobs == 1:
            if len(reco_files) == total_requested_jobs:
                print("directory of with fully reconstructed track file"
                      " already exists! Skipping...")
                return (pathname_full, True)
        else:
            if len(reco_files) >= int(0.8*total_requested_jobs):
                print("directory with at least 80% (compared to requested"
                      " number of simulated files) of fully reconstructed"
                      " track files already exists! Skipping...")
                return (pathname_full, True)

    # generate simulation config parameter file
    if 'elastic' in sim_params['sim_type']:
        # determine the elastic cross section in the theta range
        bashcommand = os.getenv('LMDFIT_BUILD_PATH') + \
            '/bin/generatePbarPElasticScattering ' + \
            str(sim_params['lab_momentum']) + ' 0'
        returnvalue = subprocess.call(bashcommand.split())
        import shutil
        shutil.move(os.getcwd()+"/elastic_cross_section.txt",
                    pathname_full+"/elastic_cross_section.txt")

    print("creating config file: " + pathname_base + "/sim_params.config")
    with open(pathname_base + '/sim_params.config', 'w') as json_file:
        json.dump(sim_params, json_file, sort_keys=True, indent=4)

    rec.writeRecoParamsToFile(reco_params, pathname_full)

    use_recoil_momentum = 1
    if sim_params['neglect_recoil_momentum']:
        use_recoil_momentum = 0

    # box gen: -1
    # dpm only inelastic: 0
    # dpm elastic & inelastic: 1
    # dpm only elastic: 2
    sim_type = sim_params['sim_type']

    reaction_type = 2
    if sim_params == 'box':
        reaction_type = -1
    elif sim_params == 'dpm_elastic':
        reaction_type = 2
    elif sim_params == 'dpm_elastic_inelastic':
        reaction_type = 1

    joblist = []

    resource_request = himster.JobResourceRequest(12*60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 3000
    resource_request.node_scratch_filesize_in_mb = 0

    if use_devel_queue:
        resource_request = himster.make_test_resource_request()

    job = himster.Job(resource_request, './runLmdSimReco.sh',
                      'lmd_simreco_' + sim_type,
                      pathname_full + '/simreco-%a.log')
    job.set_job_array_indices(
        list(range(low_index_used, low_index_used+num_samples)))

    if sim_params['sim_type'] == 'noise':
        job.add_exported_user_variable('simulate_noise', '1')
    job.add_exported_user_variable(
        'num_evts', str(sim_params['num_events_per_sample']))
    job.add_exported_user_variable('mom', str(sim_params['lab_momentum']))
    job.add_exported_user_variable('reaction_type', reaction_type)
    job.add_exported_user_variable(
        'theta_min_in_mrad', sim_params['theta_min_in_mrad'])
    job.add_exported_user_variable(
        'theta_max_in_mrad', sim_params['theta_max_in_mrad'])
    job.add_exported_user_variable('use_recoil_momentum', use_recoil_momentum)
    job.add_exported_user_variable('random_seed', sim_params['random_seed'])

    job.add_exported_user_variable('dirname', dirname_full)
    job.add_exported_user_variable('path_mc_data', path_mc_data)
    job.add_exported_user_variable('pathname', pathname_full)

    ip_params = sim_params['ip_params']
    job.add_exported_user_variable('beamX0', str(ip_params['ip_offset_x']))
    job.add_exported_user_variable('beamY0', str(ip_params['ip_offset_y']))
    job.add_exported_user_variable('targetZ0', str(ip_params['ip_offset_z']))
    job.add_exported_user_variable(
        'beam_widthX', str(ip_params['ip_spread_x']))
    job.add_exported_user_variable(
        'beam_widthY', str(ip_params['ip_spread_y']))
    job.add_exported_user_variable(
        'target_widthZ', str(ip_params['ip_spread_z']))
    job.add_exported_user_variable('beam_gradX', str(ip_params['beam_tilt_x']))
    job.add_exported_user_variable('beam_gradY', str(ip_params['beam_tilt_y']))
    job.add_exported_user_variable(
        'beam_grad_sigmaX', str(ip_params['beam_divergence_x']))
    job.add_exported_user_variable(
        'beam_grad_sigmaY', str(ip_params['beam_divergence_y']))

    job = alignment.appendAlignmentInfoToJob(job, align_params)
    job = rec.appendRecoInfoToJob(job, reco_params)

    joblist.append(job)

    # job threshold of this type (too many jobs could generate to much io load
    # as quite a lot of data is read in from the storage...)
    job_manager = himster.HimsterJobManager(30000, 3600, debug=debug)

    job_manager.submit_jobs_to_himster(joblist)
    job_manager.manage_jobs()

    return (pathname_full, False)
