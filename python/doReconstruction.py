#!/usr/bin/env python3

#! =======================================================
#! ====   DEPRECATED? NOT USED ANYWHERE!              ====
#! =======================================================

# TODO: rename to runReconstruction.py and make it like the
# runSimulationReconstruction.py script

import argparse

import lumifit.alignment as alignment
import lumifit.general as general
import wrappers.createRecoJob as createRecoJob

parser = argparse.ArgumentParser(
    description="Script for full reconstruction of the PANDA Luminosity" " Detector.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser = general.addGeneralArgumentsToParser(parser)
parser = general.addDebugArgumentsToParser(parser)
parser = alignment.addArgumentsToParser(parser)
parser = createRecoJob.addArgumentsToParser(parser)
parser.add_argument(
    "output_dir",
    metavar="output_dir",
    type=str,
    nargs=1,
    help="This directory is used for the output.",
)

args = parser.parse_args()

reco_params = createRecoJob.addReconstructionParametersToConfig({}, args)
align_params = alignment.addAlignmentParametersToConfig({}, args)

createRecoJob.startReconstruction(
    reco_params,
    align_params,
    dirname=args.output_dir[0],
    force_level=args.force_level,
    debug=args.debug,
    use_devel_queue=args.use_devel_queue,
)
