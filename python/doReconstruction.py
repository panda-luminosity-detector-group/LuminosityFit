#!/usr/bin/env python3

import argparse

import reconstruction
import alignment
import general

parser = argparse.ArgumentParser(
    description="Script for full reconstruction of the PANDA Luminosity"
    " Detector.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser = general.addGeneralArgumentsToParser(parser)
parser = general.addDebugArgumentsToParser(parser)
parser = alignment.addArgumentsToParser(parser)
parser = reconstruction.addArgumentsToParser(parser)
parser.add_argument(
    "output_dir",
    metavar="output_dir",
    type=str,
    nargs=1,
    help="This directory is used for the output.",
)

args = parser.parse_args()

reco_params = reconstruction.addReconstructionParametersToConfig({}, args)
align_params = alignment.addAlignmentParametersToConfig({}, args)

reconstruction.startReconstruction(
    reco_params,
    align_params,
    dirname=args.output_dir[0],
    force_level=args.force_level,
    debug=args.debug,
    use_devel_queue=args.use_devel_queue,
)
