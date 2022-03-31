#!/usr/bin/env python3

import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file, check_stage_success
from lumifit.simulation import SimulationParameters, SimulationType

lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]
scriptpath = lmd_build_path + "../python"
dirname = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
pathname = os.environ["pathname"]
macropath = os.environ["macropath"]
force_level = int(os.environ["force_level"])

# parser = argparse.ArgumentParser(
#     description='Call for important variables'
#     ' Detector.',
#     formatter_class=argparse.RawTextHelpFormatter)

# parser.add_argument('force_level', metavar='force_level', type=int, default=0,
#                     help="force level 0: if directories exist with data\n"+
#                           "files no new simulation is started\n"+
#                           "force level 1: will do full reconstruction even if "+
#                           "this data already exists, but not geant simulation\n"+
#                           "force level 2: resimulation of everything!")
# parser.add_argument('dirname', metavar='dirname', type=str, nargs=1,
#                     help='This directory for the outputfiles.')
# parser.add_argument('path_mc_data', metavar='path_mc_data', type=str, nargs=1,
#                     help='Path to MC files.')
# parser.add_argument('pathname', metavar='pathname', type=str, nargs=1,
#                     help='This the path to the outputdirectory')
# parser.add_argument('macropath', metavar='macropath', type=str, nargs=1,
#                     help='This the path to the macros')

# args = parser.parse_args()

# "force_level": force_level,
# "dirname": dirname_full,
# "path_mc_data": path_mc_data,
# "pathname": pathname_full,

# python runLmdSimReco --pathname asdfasdfsafdsa
# class InputParams():

filename_index = 1
debug = True
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

if debug:
    workpathname = pathname
    path_mc_data = workpathname
else:
    workpathname = f"/localscratch/{os.environ['SLURM_JOB_ID']}/{dirname}"

if not os.path.isdir(workpathname):
    os.makedirs(workpathname)

gen_filepath = workpathname + "/gen_mc.root"

# TODO: check if params are loaded correctly, shouldn't be the specified file name be uses?
sim_params = SimulationParameters(
    **load_params_from_file(path_mc_data + "/../sim_params.config")
)
# TODO: read alignment parameters correctly
ali_params = AlignmentParameters()
# ali_params = AlignmentParameters(**load_params_from_file(path_mc_data + "/.."))

verbositylvl = 0
numTrks = 1  # should not be changed
start_evt: int = sim_params.num_events_per_sample * filename_index


if (
    not check_stage_success(path_mc_data + f"/Lumi_MC_{start_evt}.root")
    or force_level == 2
):

    # * prepare box or dpm tracks
    os.chdir(scriptpath)
    # if sim_params.reaction_type -eq -1:
    if sim_params.sim_type == SimulationType.BOX:
        os.system(
            f"root -l -b -q 'standaloneBoxGen.C({sim_params.lab_momentum}, {sim_params.num_events_per_sample}, {sim_params.theta_min_in_mrad}, {sim_params.theta_max_in_mrad},{gen_filepath}, {sim_params.random_seed}, {sim_params.neglect_recoil_momentum})'"
        )
    elif sim_params.sim_type == SimulationType.PBARP_ELASTIC:
        os.system(
            f"{lmd_build_path}/bin/generatePbarPElasticScattering {sim_params.lab_momentum} {sim_params.num_events_per_sample} -l {sim_params.theta_min_in_mrad} -u {sim_params.theta_max_in_mrad} -s {sim_params.random_seed} -o {gen_filepath}"
        )

    # * run simBox or simDPM
    os.chdir(macropath)
    print("starting up a pandaroot simulation...")

    if sim_params.sim_type == SimulationType.NOISE:
        os.system(
            f"root -l -b -q 'runLumiPixel0SimBox.C({sim_params.num_events_per_sample},{start_evt},     {workpathname},{verbositylvl},2112,{sim_params.lab_momentum},{numTrks},{sim_params.random_seed})' > /dev/null 2>&1"
        )
    else:
        os.system(
            f"root -l -b -q 'runLumiPixel0SimDPM.C({sim_params.num_events_per_sample},{start_evt},{sim_params.lab_momentum}, {gen_filepath}, {workpathname}, {sim_params.ip_offset_x}, {sim_params.ip_offset_y}, {sim_params.ip_offset_z}, {sim_params.ip_spread_x}, {sim_params.ip_spread_y}, {sim_params.ip_spread_z}, {sim_params.beam_tilt_x}, {sim_params.beam_tilt_y}, {sim_params.beam_divergence_x}, {sim_params.beam_divergence_y}, {sim_params.lmd_geometry_filename}, {ali_params.misalignment_matrices_path}, {1 if ali_params.use_point_transform_misalignment else 0}, {verbositylvl})'"
        )

    # if debug mode, copy mc data from node to permanent storage
    if debug:
        os.system(
            f"cp {workpathname}/Lumi_MC_{start_evt}.root {path_mc_data}/Lumi_MC_{start_evt}.root"
        )
        os.system(
            f"cp {workpathname}/Lumi_Params_{start_evt}.root {path_mc_data}/Lumi_Params_{start_evt}.root"
        )

# if first stage was successful, copy MC data directly to compute node and don't generate new
else:
    if debug:
        os.system(
            f"cp {path_mc_data}/Lumi_MC_{start_evt}.root {workpathname}/Lumi_MC_{start_evt}.root"
        )
        os.system(
            f"cp {path_mc_data}/Lumi_Params_{start_evt}.root {workpathname}/Lumi_Params_{start_evt}.root"
        )

if (
    not check_stage_success(workpathname + f"/Lumi_digi_{start_evt}.root")
    or force_level == 2
):

    if sim_params.sim_type == SimulationType.NOISE:
        os.system(
            f"root -l -b -q 'runLumiPixel1bDigiNoise.C({sim_params.num_events_per_sample}, {start_evt}, {workpathname}, {verbositylvl}, {sim_params.random_seed})'"
        )
    else:
        os.system(
            f"root -l -b -q 'runLumiPixel1Digi.C({sim_params.num_events_per_sample}, {start_evt}, {workpathname}, {ali_params.misalignment_matrices_path}, {ali_params.use_point_transform_misalignment}, {verbositylvl})'"
        )

os.chdir(scriptpath)
os.system("./runLmdReco.py")