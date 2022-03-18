import argparse
import json
import os

import alignment
import general
import reconstruction
from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file
from lumifit.simulation import SimulationParameters

parser = argparse.ArgumentParser(
    description='Call for important variables'
    ' Detector.',
    formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('force_level', metavar='force_level', type=int, default=0,
                    help='"force level 0: if directories exist with data\n"
                          "files no new simulation is started\n"
                          "force level 1: will do full reconstruction even if "
                          "this data already exists, but not geant simulation\n"
                          "force level 2: resimulation of everything!"')
parser.add_argument('dirname', metavar='dirname', type=str, nargs=1,
                    help='This directory for the outputfiles.')
parser.add_argument('path_mc_data', metavar='path_mc_data', type=str, nargs=1,
                    help='Path to MC files.')
parser.add_argument('pathname', metavar='pathname', type=str, nargs=1,
                    help='This the path to the outputdirectory')
parser.add_argument('macropath', metavar='macropath', type=str, nargs=1,
                    help='This the path to the macros')

args = parser.parse_args()

# "force_level": force_level,
# "dirname": dirname_full,
# "path_mc_data": path_mc_data,
# "pathname": pathname_full,

# python runLmdSimReco --pathname asdfasdfsafdsa
#class InputParams():

 #   def _init_(  macropath = ${VMCWORKDIR}"/macro/detector/lmd", pathname, scriptpath = $pwd, dirname, workpathname = "lokalscratch/" + ${SLURM_JOB_ID} + "/" + dirname, gen_filepath = workpathname + "/gen_mc.root")

if not os.path.isdir(args.pathname):
 os.system("mkdir -p args.pathname")

sim_params = SimulationParameters(**load_params_from_file(path_mc_data + "/.."))
ali_params = AlignmentParameters(**load_params_from_file(path_mc_data + "/.."))

verbosityLevel: int = 0
start_evt: int = $((${num_evts}*${filename_index}))
numTracks: int = 1 #do not change
workpathname = "lokalscratch/" + ${SLURM_JOB_ID} + "/" + dirname
gen_filepath = workpathname + "/gen_mc.root"
scriptpath = os.getcwd() 

 #simulation

check_stage_success "args.path_mc_data + "/Lumi_MC_${start_evt}.root""
if 0 == "$?" or  2 == "args.force_level":
   os.system(f"cd scriptpath")
   if sim_params.reaction_type -eq -1:
     os.system(f"root -l -b -q 'standaloneBoxGen.C({sim_params.lab_momentum}, '{sim_params.num_events_per_sample}', '{sim_params.theta_min_in_mrad}', '{sim_params.theta_max_in_mrad}', "'gen_filepath'", '{sim_params.random_seed}', '{sim_params.neglect_recoil_momentum}')'")
   else: 
     os.system(f"${LMDFIT_BUILD_PATH}/bin/generatePbarPElasticScattering {sim_params.lab_momentum} {sim_params.num_events_per_sample} -l {sim_params.theta_min_in_mrad} -u {sim_params.theta_max_in_mrad} -s {sim_params.random_seed} -o gen_filepath")
   os.system("cd args.macropath")
   print('starting up a pandaroot simulation...')
   if simulate_noise:
     os.system(f"root -l -b -q 'runLumiPixel0SimBox.C('{sim_params.num_events_per_sample}','${start_evt}',"'workpathname'",'verbositylvl',2112,'{sim_params.lab_momentum}','{numTrks}','{sim_params.random_seed}')' > /dev/null 2>&1")
   else: 
     os.system(f"root -l -b -q 'runLumiPixel0SimDPM.C('{sim_params.num_events_per_sample}','${start_evt}','{sim_params.lab_momentum}',"'gen_filepath'", "'workpathname'",'{sim_params.ip_offset_x}', '{sim_params.ip_offset_y}', '{sim_params.ip_offset_z}', '{sim_params.ip_spread_x}', '{sim_params.ip_spread_y}', '{sim_params.ip_spread_z}', '{sim_params.beam_tilt_x}', '{sim_params.beam_tilt_y}', '{sim_params.beam_divergence_x}', '{sim_params.beam_divergence_y}', "'{sim_params.lmd_geometry_filename}'", "'${ali_params.misalignment_matrices_path}'", '${ali_params.use_point_transform_misalignment}', '$verbositylvl')'")
   if debug:
     os.system(f"cp {args.workpathname}/Lumi_MC_${start_evt}.root {args.path_mc_data}/Lumi_MC_${start_evt}.root")
     os.system(f"cp {args.workpathname}/Lumi_Params_${start_evt}.root {args.path_mc_data}/Lumi_Params_${start_evt}.root")
else:
  if debug:
    os.system(f"cp {args.path_mc_data}/Lumi_MC_${start_evt}.root workpathname/Lumi_MC_${start_evt}.root")
    os.system(f"cp {args.path_mc_data}/Lumi_Params_${start_evt}.root workpathname/Lumi_Params_${start_evt}.root")

check_stage_success "workpathname + "/Lumi_digi_${start_evt}.root""
if 0 == "$?" or  1 == "{args.force_level}":
  if simulate_noise:
    os.system(f"root -l -b -q 'runLumiPixel1bDigiNoise.C('{sim_params.num_events_per_sample}','${start_evt}',"'workpathname'",'verbositylvl', '{sim_params.random_seed}')'")
  else:
    os.system(f"root -l -b -q 'runLumiPixel1Digi.C('{sim_params.num_events_per_sample}','${start_evt}',"'workpathname'", "'{ali_params.misalignment_matrices_path}'", '{ali_params.use_point_transform_misalignment}', 'verbositylvl')'")

os.system(f"cd scriptpath")
os.system(f"python runLmdReco.py")
