import os
import sys
import errno
import glob
import json

import himster
import alignment

track_search_algorithms = ['CA', 'Follow']


def addArgumentsToParser(parser):
    parser.add_argument('--use_xy_cut', action='store_true',
                        help='Use the x-theta & y-phi filter after the '
                        'tracking stage to remove background.')
    parser.add_argument('--use_m_cut', action='store_true',
                        help='Use the tmva based momentum cut filter after the'
                        ' backtracking stage to remove background.')

    parser.add_argument('--track_search_algo',
                        metavar='track_search_algorithm',
                        type=str, choices=track_search_algorithms,
                        help='Track Search algorithm to be used.')

    parser.add_argument('--reco_ip_offset',
                        metavar=("rec_ip_offset_x", "rec_ip_offset_y",
                                 "rec_ip_offset_z"),
                        type=float, nargs=3,
                        help="rec_ip_offset_x: interaction vertex mean X "
                        "position (in cm)\n"
                        "rec_ip_offset_y: interaction vertex mean Y position "
                        "(in cm)\n"
                        "rec_ip_offset_z: interaction vertex mean Z position "
                        "(in cm)\n")

    return parser


def createReconstructionParameters():
    return {
        'use_xy_cut': False,
        'use_m_cut': False,
        'track_search_algo': 'CA',
        'reco_ip_offset': None
    }


def addReconstructionParametersToConfig(params, args):
    reco_params = createReconstructionParameters()
    if args.use_xy_cut:
        reco_params['use_xy_cut'] = args.use_xy_cut
    if args.use_m_cut:
        reco_params['use_m_cut'] = args.use_m_cut
    if args.track_search_algo:
        reco_params['track_search_algo'] = args.track_search_algo
    if args.reco_ip_offset:
        reco_params['reco_ip_offset'] = args.reco_ip_offset

    from copy import copy
    new_params = copy(params)
    new_params.update(reco_params)

    return new_params


def generateRecoDirSuffix(reco_params, align_params):
    reco_dirname_suffix = str(reco_params['low_index']) + '-' + str(
        reco_params['low_index']+reco_params['num_samples']-1) + '_'
    if reco_params['use_xy_cut']:
        reco_dirname_suffix += 'xy_'
    if reco_params['use_m_cut']:
        reco_dirname_suffix += 'm_'
    if not reco_params['use_xy_cut'] and not reco_params['use_m_cut']:
        reco_dirname_suffix += 'un'
    reco_dirname_suffix += 'cut'
    if reco_params['use_xy_cut']:
        if reco_params['reco_ip_offset']:
            reco_dirname_suffix += '_real'
    if align_params['use_point_transform_misalignment']:
        if align_params['misalignment_matrices_path'] == '':
            reco_dirname_suffix += '_no_data_misalignment'
        else:
            reco_dirname_suffix += '_' + \
                str(os.path.splitext(os.path.basename(
                    align_params['misalignment_matrices_path']))[0])
    if align_params['alignment_matrices_path']:
        reco_dirname_suffix += '_aligned'
    return reco_dirname_suffix


def startReconstruction(reco_params, align_params, dirname, force_level=0,
                        debug=False, use_devel_queue=False):
    print('preparing reconstruction in index range '
          + str(reco_params['low_index']) + ' - '
          + str(reco_params['low_index']+reco_params['num_samples']-1))

    dirname_filter_suffix = generateRecoDirSuffix(reco_params, align_params)

    low_index_used = reco_params['low_index']
    num_samples = reco_params['num_samples']
    if debug and reco_params['num_samples'] > 1:
        print("Warning: number of samples in debug mode is limited to 1!"
              " Setting to 1!")
        num_samples = 1

    pathname_base = dirname
    path_mc_data = pathname_base + '/mc_data'
    dirname_full = dirname + '/' + dirname_filter_suffix
    pathname_full = dirname_full

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
                print("directory of with fully reconstructed track file "
                      "already exists! Skipping...")
                return (pathname_full, True)
        else:
            if len(reco_files) >= int(0.8*total_requested_jobs):
                print("directory with at least 80% (compared to requested "
                      "number of simulated files) of fully reconstructed "
                      " track files already exists! Skipping...")
                return (pathname_full, True)

    writeRecoParamsToFile(reco_params, pathname_full)

    joblist = []

    resource_request = himster.JobResourceRequest(2*60)
    resource_request.number_of_nodes = 1
    resource_request.processors_per_node = 1
    resource_request.memory_in_mb = 3000
    resource_request.node_scratch_filesize_in_mb = 0

    if use_devel_queue:
        resource_request = himster.make_test_resource_request()

    job = himster.Job(resource_request, './runLmdReco.sh',
                      'lmd_reco_', pathname_full + '/reco-%a.log')

    job.set_job_array_indices(
        list(range(low_index_used, low_index_used+num_samples)))

    job.add_exported_user_variable(
        'num_evts', str(reco_params['num_events_per_sample']))
    job.add_exported_user_variable('mom', str(reco_params['lab_momentum']))

    job.add_exported_user_variable('dirname', dirname_full)
    job.add_exported_user_variable('path_mc_data', path_mc_data)
    job.add_exported_user_variable('pathname', pathname_full)

    job.add_exported_user_variable('force_level', force_level)

    job = alignment.appendAlignmentInfoToJob(job, align_params)
    job = appendRecoInfoToJob(job, reco_params)

    joblist.append(job)

    # job threshold of this type (too many jobs could generate to much io load
    # as quite a lot of data is read in from the storage...)
    job_manager = himster.HimsterJobManager(2000, 3600, debug=debug)

    job_manager.submit_jobs_to_himster(joblist)
    job_manager.manage_jobs()

    return (pathname_full, False)


def writeRecoParamsToFile(reco_params, pathname):
        # generate reconstruction config parameter file
    print("creating config file: " + pathname + "/reco_params.config")
    with open(pathname + '/reco_params.config', 'w') as json_file:
        json.dump(reco_params, json_file, sort_keys=True, indent=4)


def appendRecoInfoToJob(job, reco_params):
    job.add_exported_user_variable('SkipFilt', str(
        not reco_params['use_xy_cut']).lower())
    job.add_exported_user_variable(
        'XThetaCut', str(reco_params['use_xy_cut']).lower())
    job.add_exported_user_variable(
        'YPhiCut', str(reco_params['use_xy_cut']).lower())
    job.add_exported_user_variable(
        'CleanSig', str(reco_params['use_m_cut']).lower())
    job.add_exported_user_variable(
        'track_search_algorithm', reco_params['track_search_algo'])
    job.add_exported_user_variable(
        'lmd_geometry_filename', reco_params['lmd_geometry_filename'])

    reco_ip_offsets = [0.0, 0.0, 0.0]
    if reco_params['reco_ip_offset']:
        reco_ip_offsets = reco_params['reco_ip_offset']
    job.add_exported_user_variable(
        'rec_ipx', str(reco_ip_offsets[0]))
    job.add_exported_user_variable(
        'rec_ipy', str(reco_ip_offsets[1]))
    job.add_exported_user_variable(
        'rec_ipz', str(reco_ip_offsets[2]))

    return job
