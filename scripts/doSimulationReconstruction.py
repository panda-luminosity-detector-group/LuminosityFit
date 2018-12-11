#!/usr/bin/env python3

import argparse

import simulation
import reconstruction
import alignment
import general

parser = argparse.ArgumentParser(
    description='Script for full simulation of PANDA Luminosity Detector via '
    'externally generated MC data.',
    formatter_class=argparse.RawTextHelpFormatter)

parser = general.addGeneralArgumentsToParser(parser)
parser = general.addDebugArgumentsToParser(parser)
parser = simulation.addArgumentsToParser(parser)
parser = alignment.addArgumentsToParser(parser)
parser = reconstruction.addArgumentsToParser(parser)

args = parser.parse_args()

align_params = alignment.addAlignmentParametersToConfig({}, args)
general_params = general.addGeneralRunParametersToConfig(align_params, args)
sim_params = simulation.addSimulationParametersToConfig(general_params, args)
reco_params = reconstruction.addReconstructionParametersToConfig(
    general_params, args)

output_dir = ''
if args.output_dir:
    output_dir = args.output_dir
simulation.startSimulationAndReconstruction(
    sim_params, align_params, reco_params, output_dir,
    force_level=args.force_level, debug=args.debug,
    use_devel_queue=args.use_devel_queue)
