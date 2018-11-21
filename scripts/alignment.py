def addArgumentsToParser(parser):
    parser.add_argument('--misalignment_matrices_path',
                        metavar='misalignment_matrices_path', type=str,
                        default='', help='')
    parser.add_argument('--alignment_matrices_path',
                        metavar='alignment_matrices_path', type=str,
                        default='', help='')
    parser.add_argument('--use_point_transform_misalignment',
                        action='store_true',
                        help='Instead of misaligning the geometry, '
                        'the points are misaligned after the simulation.')

    return parser


def createAlignmentParameters():
    return {
        'misalignment_matrices_path': '',
        'use_point_transform_misalignment': False,
        'alignment_matrices_path': ''
    }


def addAlignmentParametersToConfig(params, args):
    alignment_params = createAlignmentParameters()
    if args.misalignment_matrices_path:
        alignment_params[
            'misalignment_matrices_path'] = args.misalignment_matrices_path
    if args.use_point_transform_misalignment:
        alignment_params['use_point_transform_misalignment'
                         ] = args.use_point_transform_misalignment
    if args.alignment_matrices_path:
        alignment_params[
            'alignment_matrices_path'] = args.alignment_matrices_path
    from copy import copy
    new_params = copy(params)
    new_params.update(alignment_params)
    return new_params


def appendAlignmentInfoToJob(job, align_params):
    job.add_exported_user_variable(
        'misalignment_matrices_path',
        align_params['misalignment_matrices_path'])
    job.add_exported_user_variable('use_point_transform_misalignment', str(
        align_params['use_point_transform_misalignment']).lower())
    job.add_exported_user_variable(
        'alignment_matrices_path', align_params['alignment_matrices_path'])
    return job
