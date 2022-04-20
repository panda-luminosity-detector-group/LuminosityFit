#!/usr/bin/env python3

#! =======================================================
#! ====   DEPRECATED? NOT USED ANYWHERE!              ====
#! =======================================================

import json
import argparse
import os

import lumifit.alignment as alignment
import lumifit.general as general
import lumifit.reconstruction as reconstruction

parser = argparse.ArgumentParser(
    description="Script to run a reconstruction of the PANDA "
    "Luminosity Detector from parameter config files.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser.add_argument(
    "reco_params",
    metavar="reco_params",
    type=str,
    nargs=1,
    help="Path to reconstruction properties json file\n",
)

parser = general.addDebugArgumentsToParser(parser)

args = parser.parse_args()

with open(args.reco_params[0], "r") as json_file:
    reco_params = json.load(json_file)

align_params = alignment.getAlignmentParameters(reco_params)

reconstruction.startReconstruction(
    reco_params,
    align_params,
    os.path.split(os.path.split(args.reco_params[0])[0])[0],
    force_level=args.force_level,
    debug=args.debug,
    use_devel_queue=args.use_devel_queue,
)
