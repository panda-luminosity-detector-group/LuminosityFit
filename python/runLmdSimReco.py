#!/usr/bin/env python3

import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import (
    check_stage_success,
    load_params_from_file,
    matrixMacroFileName,
    toCbool,
)
from lumifit.simulation import SimulationParameters
from python.lumifit.simulationGeneratorTypes import SimulationGeneratorType

lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]
PNDmacropath = os.environ["LMDFIT_MACROPATH"]
LMDscriptpath = os.environ["LMDFIT_SCRIPTPATH"]

pathToTrkQAFiles = os.environ["pathname"]
relativeDirToTrksQAFiles = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
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
    workpathname = pathToTrkQAFiles
    path_mc_data = workpathname
else:
    workpathname = f"/localscratch/{os.environ['SLURM_JOB_ID']}/{relativeDirToTrksQAFiles}"

if not os.path.isdir(workpathname):
    os.makedirs(workpathname)

gen_filepath = workpathname + "/gen_mc.root"

# the path pathToTrkQAFiles is automatically eihter the dpm or the resAcc path
#! however, check this path again!
simParams: SimulationParameters = load_params_from_file(path_mc_data + "/../sim_params.config", SimulationParameters)
alignParams: AlignmentParameters = load_params_from_file(pathToTrkQAFiles + "/align_params.config", AlignmentParameters)

verbositylvl = 0
numTrks = 1  # should not be changed
start_evt: int = simParams.num_events_per_sample * filename_index


print(f"\n\nINFO:\ndirname is {relativeDirToTrksQAFiles}\npathname is {pathToTrkQAFiles}\nworkpathname is {workpathname}\n\n")

# * ------------------- MC Data Step -------------------
if not check_stage_success(f"{path_mc_data}/Lumi_MC_{start_evt}.root") or force_level == 2:

    # * prepare box or dpm tracks
    if simParams.simGeneratorType == SimulationGeneratorType.BOX or simParams.simGeneratorType == SimulationGeneratorType.RESACCBOX:
        os.chdir(LMDscriptpath)

        cmd = f"""root -l -b -q 'standaloneBoxGen.C({simParams.lab_momentum}, {simParams.num_events_per_sample}, {simParams.theta_min_in_mrad}, {simParams.theta_max_in_mrad}, {simParams.phi_min_in_rad}, {simParams.phi_max_in_rad},"{gen_filepath}", {simParams.random_seed + start_evt}, {toCbool(not simParams.neglect_recoil_momentum)})'"""

        print(f"\ncalling stand alone box gen with:\n")
        print(cmd)
        os.system(cmd)

    elif simParams.simGeneratorType == SimulationGeneratorType.PBARP_ELASTIC or simParams.simGeneratorType == SimulationGeneratorType.RESACCPBARP_ELASTIC:

        # cmd = f"{lmd_build_path}/bin/generatePbarPElasticScattering {sim_params.lab_momentum} {sim_params.num_events_per_sample} -l {sim_params.theta_min_in_mrad} -u {sim_params.theta_max_in_mrad} -s {sim_params.random_seed + start_evt} -o {gen_filepath}"

        # print(f"\ncalling elastic P Pbar generator with:\n")
        # print(cmd)

        # os.system(cmd)

        os.system(
            f"{lmd_build_path}/bin/generatePbarPElasticScattering"
            + f" {simParams.lab_momentum} {simParams.num_events_per_sample}"
            + f" -l {simParams.theta_min_in_mrad}"
            + f" -u {simParams.theta_max_in_mrad}"
            + f" -n {simParams.phi_min_in_rad}"
            + f" -g {simParams.phi_max_in_rad}"
            + f" -s {simParams.random_seed + start_evt}"
            + f" -o {gen_filepath}"
        )

    # * run simBox or simDPM
    print("starting up a pandaroot simulation...")
    os.chdir(PNDmacropath)
    if simParams.simGeneratorType == SimulationGeneratorType.NOISE:

        os.system(
            f"""root -l -b -q 'runLumiPixel0SimBox.C({simParams.num_events_per_sample}, {start_evt}, "{workpathname}",{verbositylvl},-2212,{simParams.lab_momentum},{numTrks},{simParams.random_seed + start_evt}, 0, , "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)})' > /dev/null 2>&1"""
        )
    else:
        os.system(
            f"""root -l -b -q 'runLumiPixel0SimDPM.C({simParams.num_events_per_sample}, {start_evt}, {simParams.lab_momentum}, "{gen_filepath}", "{workpathname}", {simParams.ip_offset_x}, {simParams.ip_offset_y}, {simParams.ip_offset_z}, {simParams.ip_spread_x}, {simParams.ip_spread_y}, {simParams.ip_spread_z}, {simParams.beam_tilt_x}, {simParams.beam_tilt_y}, {simParams.beam_divergence_x}, {simParams.beam_divergence_y}, "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
        )


# if first stage was successful, copy MC data directly to compute node and don't generate new
else:
    if not debug:
        os.system(f"cp {path_mc_data}/Lumi_MC_{start_evt}.root {workpathname}/Lumi_MC_{start_evt}.root")
        os.system(f"cp {path_mc_data}/Lumi_Params_{start_evt}.root {workpathname}/Lumi_Params_{start_evt}.root")

# * ------------------- Digi Step -------------------
if not check_stage_success(workpathname + f"/Lumi_digi_{start_evt}.root") or force_level == 2:
    os.chdir(PNDmacropath)
    if simParams.simGeneratorType == SimulationGeneratorType.NOISE:
        os.system(
            f"""root -l -b -q 'runLumiPixel1bDigiNoise.C({simParams.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, {simParams.random_seed + start_evt})'"""
        )
    else:
        os.system(
            f"""root -l -b -q 'runLumiPixel1Digi.C({simParams.num_events_per_sample}, {start_evt}, "{workpathname}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
        )


# always copy mc data and params from node to permanent storage (params are needed for all subsequent steps. Also: Params are UPDATED every step, so only the final Params file holds all needed data)
if not debug:
    os.system(f"cp {workpathname}/Lumi_MC_{start_evt}.root {path_mc_data}/Lumi_MC_{start_evt}.root")
    os.system(f"cp {workpathname}/Lumi_Params_{start_evt}.root {path_mc_data}/Lumi_Params_{start_evt}.root")
    # copy the Lumi_Digi data to permanent storage, it's needed for IP cut for the LumiFit
    # MC path is better for this since digi data is "almost real data"
    os.system(f"cp {workpathname}/Lumi_digi_{start_evt}.root {path_mc_data}/Lumi_digi_{start_evt}.root")

os.chdir(LMDscriptpath)
os.system("./runLmdReco.py")
