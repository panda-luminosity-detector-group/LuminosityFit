#!/usr/bin/env python3

import os
import copy
import subprocess
import multiprocessing
from decimal import Decimal
from general import IPParams
import argparse

cpu_cores = multiprocessing.cpu_count()


class XYZLists:
    list_x = []
    list_y = []
    list_z = []

    def clear(self):
        self.list_x = []
        self.list_y = []
        self.list_z = []

    def appendValues(self, x, y, z=None):
        self.list_x.append(x)
        self.list_y.append(y)
        if z is not None:
            self.list_z.append(z)

    def __repr__(self):
        return (
            "x values: "
            + str(self.list_x)
            + "\ny values: "
            + str(self.list_y)
            + "\nz values: "
            + str(self.list_z)
            + "\n"
        )

    def __str__(self):
        return self.__repr__()


temp_ip_params = IPParams()


def setIPOffsetXYZ(ip_offset_x_, ip_offset_y_, ip_offset_z_):
    temp_ip_params.ip_offset_x = ip_offset_x_
    temp_ip_params.ip_offset_y = ip_offset_y_
    temp_ip_params.ip_offset_z = ip_offset_z_
    return copy.deepcopy(temp_ip_params)


def setIPSpreadXYZ(ip_spread_x_, ip_spread_y_, ip_spread_z_):
    temp_ip_params.ip_spread_x = ip_spread_x_
    temp_ip_params.ip_spread_y = ip_spread_y_
    temp_ip_params.ip_spread_z = ip_spread_z_
    return copy.deepcopy(temp_ip_params)


def setBeamTiltXY(beam_tilt_x_, beam_tilt_y_):
    temp_ip_params.beam_tilt_x = beam_tilt_x_
    temp_ip_params.beam_tilt_y = beam_tilt_y_
    return copy.deepcopy(temp_ip_params)


def setBeamDivergenceXY(beam_divergence_x_, beam_divergence_y_):
    temp_ip_params.beam_divergence_x = beam_divergence_x_
    temp_ip_params.beam_divergence_y = beam_divergence_y_
    return copy.deepcopy(temp_ip_params)


def createScenarios(
    ip_param_list, populator_function, mode, value_list, value_z_list=[]
):
    current_ip_param_list = list(ip_param_list)
    del ip_param_list[:]

    mylists = createScenarioLists(mode, value_list, value_z_list)

    global temp_ip_params

    if value_z_list:
        for current_ip_params in current_ip_param_list:
            temp_ip_params = current_ip_params
            ip_param_list.extend(
                map(
                    populator_function,
                    mylists.list_x,
                    mylists.list_y,
                    mylists.list_z,
                )
            )
    else:
        for current_ip_params in current_ip_param_list:
            temp_ip_params = current_ip_params
            ip_param_list.extend(
                map(populator_function, mylists.list_x, mylists.list_y)
            )


only_positive_values = False


def createScenarioLists(mode, value_list, value_z_list=None):
    lists = XYZLists()
    lists.clear()

    if only_positive_values:
        if mode == "s":
            if value_z_list:
                lists.appendValues(
                    abs(value_list[0]),
                    abs(value_list[0]),
                    abs(value_z_list[0]),
                )
            else:
                lists.appendValues(abs(value_list[0]), abs(value_list[0]))
        else:
            values = map(abs, value_list)
            z_value = None
            if value_z_list:
                z_value = abs(value_z_list[0])
            for value in values:
                if value > Decimal("0.0"):
                    if mode == "a":
                        lists.appendValues(value, Decimal("0.0"), z_value)

                        lists.appendValues(Decimal("0.0"), value, z_value)
                    elif mode == "g":
                        for value2 in values:
                            if value2 > Decimal("0.0"):
                                lists.appendValues(value, value2, z_value)
                    elif mode == "c":
                        lists.appendValues(value, value, z_value)

                    else:
                        lists.appendValues(value, Decimal("0.0"), z_value)

                        lists.appendValues(Decimal("0.0"), value, z_value)

                        lists.appendValues(value, value, z_value)
    else:
        if mode == "s":
            if value_z_list:
                lists.appendValues(
                    value_list[0], value_list[0], value_z_list[0]
                )
            else:
                lists.appendValues(value_list[0], value_list[0])
        else:
            values = map(abs, value_list)
            z_value = None
            if value_z_list:
                z_value = value_z_list[0]
            for value in values:
                if value > Decimal("0.0"):
                    if mode == "a":
                        lists.appendValues(value, Decimal("0.0"), z_value)
                        lists.appendValues(-value, Decimal("0.0"), z_value)

                        lists.appendValues(Decimal("0.0"), value, z_value)
                        lists.appendValues(Decimal("0.0"), -value, z_value)

                    elif mode == "c":
                        lists.appendValues(value, value, z_value)
                        lists.appendValues(-value, value, z_value)

                        lists.appendValues(value, -value, z_value)
                        lists.appendValues(-value, -value, z_value)

                    else:
                        lists.appendValues(value, Decimal("0.0"), z_value)
                        lists.appendValues(-value, Decimal("0.0"), z_value)

                        lists.appendValues(Decimal("0.0"), value, z_value)
                        lists.appendValues(Decimal("0.0"), -value, z_value)

                        lists.appendValues(value, value, z_value)
                        lists.appendValues(-value, value, z_value)

                        lists.appendValues(value, -value, z_value)
                        lists.appendValues(-value, -value, z_value)
    return lists


def createIPParameterScenarios():
    default_ip_params = IPParams()

    ip_spread_abs_list = args.ip_spread_abs
    ip_spread_z_abs_list = args.ip_spread_z_abs
    ip_spread_mode = args.ip_spread_mode
    ip_offset_abs_list = args.ip_offset_abs
    ip_offset_z_abs_list = args.ip_offset_z_abs
    ip_offset_mode = args.ip_offset_mode
    beam_tilt_abs_list = args.beam_tilt_abs
    beam_tilt_mode = args.beam_tilt_mode
    beam_div_abs_list = args.beam_div_abs
    beam_div_mode = args.beam_div_mode

    ip_param_list = []
    ip_param_list.append(default_ip_params)

    global only_positive_values
    only_positive_values = True
    if ip_spread_abs_list or ip_spread_z_abs_list:
        createScenarios(
            ip_param_list,
            setIPSpreadXYZ,
            ip_spread_mode,
            ip_spread_abs_list,
            ip_spread_z_abs_list,
        )

    only_positive_values = False
    if ip_offset_abs_list:
        createScenarios(
            ip_param_list,
            setIPOffsetXYZ,
            ip_offset_mode,
            ip_offset_abs_list,
            ip_offset_z_abs_list,
        )

    if beam_tilt_abs_list:
        createScenarios(
            ip_param_list, setBeamTiltXY, beam_tilt_mode, beam_tilt_abs_list
        )

    only_positive_values = True
    if beam_div_abs_list:
        createScenarios(
            ip_param_list,
            setBeamDivergenceXY,
            beam_div_mode,
            beam_div_abs_list,
        )

    return ip_param_list


parser = argparse.ArgumentParser(
    description="Script for full simulation of PANDA Luminosity Detector via externally generated MC data.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser.add_argument(
    "num_events",
    metavar="num_events",
    type=int,
    nargs=1,
    help="number of events to simulate",
)
parser.add_argument(
    "lab_momentum",
    metavar="lab_momentum",
    type=float,
    nargs=1,
    help="lab momentum of incoming beam antiprotons\n(required to set correct magnetic field maps etc)",
)
parser.add_argument(
    "sim_type",
    metavar="simulation_type",
    type=str,
    nargs=1,
    choices=["box", "dpm_elastic", "dpm_elastic_inelastic", "noise"],
    help="four kinds: box, dpm_elastic, dpm_elastic_inelastic and noise",
)

parser.add_argument(
    "--force_level",
    metavar="force_level",
    type=int,
    default=0,
    help="force level 0: if directories exist with data files no new simulation is started\n"
    "force level 1: will do full reconstruction even if this data already exists, but not geant simulation\n"
    "force level 2: resimulation of everything!",
)

parser.add_argument(
    "--low_index",
    metavar="low_index",
    type=int,
    default=-1,
    help="Lowest index of generator file which is supposed to be used in the simulation. Default setting is -1 which will take the lowest found index.",
)
parser.add_argument(
    "--high_index",
    metavar="high_index",
    type=int,
    default=-1,
    help="Highest index of generator file which is supposed to be used in the simulation. Default setting is -1 which will take the highest found index.",
)

parser.add_argument(
    "--gen_data_dir",
    metavar="gen_data_dir",
    type=str,
    default=os.getenv("GEN_DATA"),
    help="Base directory to input files created by external generator. By default the environment variable $GEN_DATA will be used!",
)

parser.add_argument(
    "--output_dir",
    metavar="output_dir",
    type=str,
    default="",
    help="This directory is used for the output. Default is the generator directory as a prefix, with beam offset infos etc. added",
)


parser.add_argument(
    "--ip_spread_mode",
    metavar="ip_spread_mode",
    choices=["a", "c", "f", "s"],
    default="s",
    help="a=axis, c=corners, f=full round, s=single",
)

parser.add_argument(
    "--ip_spread_abs",
    metavar="ip_spread_abs",
    type=Decimal,
    nargs="*",
    default=[Decimal("0.08")],
    help="ip_spread_abs: x and y ip distribution widths that are used as a template value (in cm)",
)

parser.add_argument(
    "--ip_spread_z_abs",
    metavar="ip_spread_z_abs",
    type=Decimal,
    nargs="*",
    default=[Decimal("0.35")],
    help="ip_spread_z_abs: z ip distribution widths that are used as a template value (in cm)",
)


parser.add_argument(
    "--ip_offset_mode",
    metavar="ip_offset_mode",
    choices=["a", "c", "f", "s"],
    default="s",
    help="a=axis, c=corners, f=full round, s=single",
)

parser.add_argument(
    "--ip_offset_abs",
    metavar="ip_offset_abs",
    type=Decimal,
    nargs="*",
    default=[Decimal("0.0")],
    help="ip_offset_abs: distances from center that are used as a template value (in cm)",
)

parser.add_argument(
    "--ip_offset_z_abs",
    metavar="ip_offset_z_abs",
    type=Decimal,
    nargs="*",
    default=[Decimal("0.0")],
    help="ip_offset_z_abs: distances from center that are used as a template value (in cm)",
)


parser.add_argument(
    "--beam_tilt_mode",
    metavar="beam_tilt_mode",
    choices=["a", "c", "f", "s"],
    default="s",
    help="a = axis, c=corners, f=full round, s=single",
)

parser.add_argument(
    "--beam_tilt_abs",
    metavar="beam_tilt_abs",
    type=float,
    nargs="*",
    default=[0.0],
    help="beam_tilt_abs: tilts with respect to the beam axis, which are used as a template value for the scans (in mrad)",
)


parser.add_argument(
    "--beam_div_mode",
    metavar="beam_div_mode",
    choices=["g", "s"],
    default="s",
    help="g = grid scan, s=single",
)

parser.add_argument(
    "--beam_div_abs",
    metavar="beam_div_abs",
    type=float,
    nargs="*",
    default=[0.0],
    help="beam_div_abs: divergence with respect to the tilt mean, which are used as a template value for the scans (in mrad)",
)


parser.add_argument(
    "--track_search_algo",
    metavar="track_search_algorithm",
    type=str,
    choices=["CA", "Follow"],
    default="CA",
    help="Track Search algorithm to be used.",
)

parser.add_argument(
    "--use_xy_cut",
    action="store_true",
    help="Use the x-theta & y-phi filter after the tracking stage to remove background.",
)
parser.add_argument(
    "--use_m_cut",
    action="store_true",
    help="Use the tmva based momentum cut filter after the backtracking stage to remove background.",
)

parser.add_argument(
    "--reco_ip_offset",
    metavar=(
        "rec_ip_offset_x",
        "rec_ip_offset_y",
        "rec_ip_spread_x",
        "rec_ip_spread_y",
    ),
    type=float,
    nargs=4,
    default=[0.0, 0.0, -1.0, -1.0],
    help="rec_ip_offset_x: interaction vertex mean X position (in cm)\n"
    "rec_ip_offset_y: interaction vertex mean Y position (in cm)\n"
    "rec_ip_spread_x: interaction vertex X position distribution width (in cm)\n"
    "rec_ip_spread_y: interaction vertex Y position distribution width (in cm)\n",
)

args = parser.parse_args()

ip_params_list = createIPParameterScenarios()

additional_flags = ""
if args.use_xy_cut:
    additional_flags += " --use_xy_cut"
if args.use_m_cut:
    additional_flags += " --use_m_cut"

for ip_params in ip_params_list:
    rec_ip_info = ""
    # if we are using the xy cut
    if args.use_xy_cut or args.use_m_cut:
        if args.reco_ip_offset[2] >= 0.0 and args.reco_ip_offset[3] >= 0.0:
            rec_ip_info = (
                " --reco_ip_offset "
                + str(args.reco_ip_offset[0])
                + " "
                + str(args.reco_ip_offset[1])
                + " "
                + str(args.reco_ip_offset[2])
                + " "
                + str(args.reco_ip_offset[3])
            )

    bashcommand = (
        "python runSimulations.py --force_level "
        + str(args.force_level)
        + " --low_index "
        + str(args.low_index)
        + " --high_index "
        + str(args.high_index)
        + " --use_ip_offset "
        + str(ip_params.ip_offset_x)
        + " "
        + str(ip_params.ip_offset_y)
        + " "
        + str(ip_params.ip_offset_z)
        + " "
        + str(ip_params.ip_spread_x)
        + " "
        + str(ip_params.ip_spread_y)
        + " "
        + str(ip_params.ip_spread_z)
        + " --use_beam_gradient "
        + str(ip_params.beam_tilt_x)
        + " "
        + str(ip_params.beam_tilt_y)
        + " "
        + str(ip_params.beam_divergence_x)
        + " "
        + str(ip_params.beam_divergence_y)
        + additional_flags
        + rec_ip_info
        + " --track_search_algo "
        + args.track_search_algo
        + " "
        + str(args.num_events[0])
        + " "
        + str(args.lab_momentum[0])
        + " "
        + args.sim_type[0]
    )
    # print bashcommand
    returnvalue = subprocess.call(bashcommand.split())
