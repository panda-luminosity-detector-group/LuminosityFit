#!/usr/bin/env python3

import argparse
import os
import random
import subprocess
from decimal import Decimal

import wrappers.createSimRecoJob as createSimRecoJob

base_out_dir = "/data/work/himspecf/pflueger/realistic_scenarios"
os.environ["DATA_DIR"] = base_out_dir
low_index = 1
high_index = 100

parser = argparse.ArgumentParser(description="", formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument(
    "--lab_momentum",
    metavar="lab_momentum",
    type=float,
    default=-1.0,
    help="lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)",
)
parser.add_argument(
    "--num_events",
    metavar="num_events",
    type=int,
    default=100000,
    help="number of events to simulate",
)

args = parser.parse_args()

beam_momenta = [1.5, 4.06, 8.9, 15.0]
lab_momentum = random.choice(beam_momenta)

if args.lab_momentum >= 1.5:
    lab_momentum = args.lab_momentum

scen_params = createSimRecoJob.IPParams()

# set random values for parameters
scen_params.ip_offset_x = Decimal("{0:.3f}".format(round(random.uniform(-0.2, 0.2), 2)))  # in cm
scen_params.ip_offset_y = Decimal("{0:.3f}".format(round(random.uniform(-0.2, 0.2), 2)))  # in cm
scen_params.ip_offset_z = Decimal("{0:.3f}".format(round(random.uniform(-0.2, 0.2), 2)))  # in cm
scen_params.ip_spread_x = Decimal("{0:.3f}".format(round(random.uniform(0.04, 0.12), 2)))  # in cm
scen_params.ip_spread_y = Decimal("{0:.3f}".format(round(random.uniform(0.04, 0.12), 2)))  # in cm
scen_params.ip_spread_z = Decimal("{0:.3f}".format(round(random.uniform(0.3, 0.4), 2)))  # in cm

scen_params.beam_tilt_x = Decimal("{0:.5f}".format(round(random.uniform(-0.0002, 0.0002), 5)))  # in rad
scen_params.beam_tilt_y = Decimal("{0:.5f}".format(round(random.uniform(-0.0002, 0.0002), 5)))  # in rad
scen_params.beam_divergence_x = Decimal("{0:.5f}".format(round(random.uniform(0.00005, 0.0002), 5)))  # in rad
scen_params.beam_divergence_y = Decimal("{0:.5f}".format(round(random.uniform(0.00005, 0.0002), 5)))  # in rad

print(scen_params)

bashcommand = (
    "python runSimulations.py --low_index "
    + str(low_index)
    + " --high_index "
    + str(high_index)
    + " --use_ip_offset "
    + str(scen_params.ip_offset_x)
    + " "
    + str(scen_params.ip_offset_y)
    + " "
    + str(scen_params.ip_offset_z)
    + " "
    + str(scen_params.ip_spread_x)
    + " "
    + str(scen_params.ip_spread_y)
    + " "
    + str(scen_params.ip_spread_z)
    + " --use_beam_gradient "
    + str(scen_params.beam_tilt_x)
    + " "
    + str(scen_params.beam_tilt_y)
    + " "
    + str(scen_params.beam_divergence_x)
    + " "
    + str(scen_params.beam_divergence_y)
    + " --track_search_algo CA "
    + str(args.num_events)
    + " "
    + f"{lab_momentum:.2f}"
    + " dpm_elastic"
)
returnvalue = subprocess.call(bashcommand.split())
