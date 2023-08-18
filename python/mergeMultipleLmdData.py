#!/usr/bin/env python3

"""
This is a wrapper for bin/mergeLmdData. because the wrapped binary can already be called from the command line with a million arguments, this script doen't need to be callabe from the command line. It just needs to be usable as module. 

TODO: shorten this down and make it a module!
TODO: also this seems to use the same a,e,r,h,v flags as the determineLuminosity script, so it should use the same enums? 
"""

import argparse
import subprocess
from pathlib import Path
from typing import List

from attrs import define
from lumifit.general import envPath, getGoodFiles
from lumifit.paths import (
    generateRelativeBinningDir,
    generateRelativeBunchesDir,
)


@define(frozen=True)
class DataTypeInfo:
    data_type: str
    patternForBinary: str
    patternForGetGoodFiles: str


parser = argparse.ArgumentParser(
    description="This script merges the lumi data (type must be specified) in the given directory.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser.add_argument(
    "type",
    metavar="type",
    type=str,
    help="type of data to merge (a = angular, e = efficiency, r = resolution, v = vertex/ip, er=efficiency and resolution)",
)
parser.add_argument(
    "dirname",
    metavar="dirname_to_scan",
    type=Path,
    help="Path where the lmd data files are. a merge_data subdirectory will be created here.",
)

parser.add_argument("--num_samples", metavar="num_samples", type=int, default=1, help="")
parser.add_argument("--sample_size", metavar="sample_size", type=int, default=0, help="")


args = parser.parse_args()

pathToBinning = args.dirname / generateRelativeBunchesDir() / generateRelativeBinningDir()

data_type_list: List[DataTypeInfo] = []

# This also catches multiple modes at once, like "er". This is by design, don't change it!
if args.type.find("a") >= 0:
    data_type_list.append(DataTypeInfo("a", "lmd_data_\\d*.root", "lmd_data_*.root"))
if args.type.find("e") >= 0:
    data_type_list.append(DataTypeInfo("e", "lmd_acc_data_\\d*.root", "lmd_acc_data_*.root"))
if args.type.find("r") >= 0:
    data_type_list.append(DataTypeInfo("r", "lmd_res_data_\\d*.root", "lmd_res_data_*.root"))
if args.type.find("h") >= 0:
    data_type_list.append(DataTypeInfo("h", "lmd_res_data_\\d*.root", "lmd_res_data_*.root"))
if args.type.find("v") >= 0:
    data_type_list.append(DataTypeInfo("v", "lmd_vertex_data_\\d*.root", "lmd_vertex_data_*.root"))

# This is either one mode (a,v) or two modes (er, which combines e and r)
for data_type_info in data_type_list:
    goodFilesList = getGoodFiles(pathToBinning, data_type_info.patternForGetGoodFiles)
    if len(goodFilesList) < 1:
        raise RuntimeError(f"no files found for {data_type_info.data_type} in {pathToBinning}")

    print(f"starting merge for {pathToBinning}")
    bashcommand = f"{envPath('LMDFIT_BUILD_PATH')}/bin/mergeLmdData -p {pathToBinning} -t {data_type_info.data_type} -n {args.num_samples} -s {args.sample_size} -f {data_type_info.patternForBinary}"
    print(bashcommand)
    returnvalue = subprocess.call(bashcommand.split())
