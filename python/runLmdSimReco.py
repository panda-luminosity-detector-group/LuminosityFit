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
    description='Script for full reconstruction of the PANDA Luminosity'
    ' Detector.',
    formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('output_dir', metavar='output_dir', type=str, nargs=1,
                    help='This directory is used for the output.')
parser.add_argument('output_dir', metavar='output_dir', type=str, nargs=1,
                    help='This directory is used for the output.')

args = parser.parse_args()

# "force_level": force_level,
# "dirname": dirname_full,
# "path_mc_data": path_mc_data,
# "pathname": pathname_full,

# python runLmdSimReco --pathname asdfasdfsafdsa
#class InputParams():

 #   def _init_(  macropath = ${VMCWORKDIR}"/macro/detector/lmd", pathname, scriptpath = $pwd, dirname, workpathname = "lokalscratch/" + ${SLURM_JOB_ID} + "/" + dirname, gen_filepath = workpathname + "/gen_mc.root")

if not os.path.isdir(pathname):
 os.system("mkdir -p pathname")

sim_params = SimulationParameters(**load_params_from_file(path_mc_data + "/.."))
ali_params = AlignmentParameters(**load_params_from_file(path_mc_data + "/.."))

verbosityLevel: int = 0
start_evt: int = $((${num_evts}*${filename_index}))
numTracks: int = 1 #do not change

 #simulation

check_stage_success "path_mc_data + "/Lumi_MC_${start_evt}.root""
if 0 == "$?" or  2 == "sim_params.force_level":
   os.system(f"cd scriptpath")
   if sim_params.reaction_type -eq -1:
     os.system(f"root -l -b -q 'standaloneBoxGen.C({sim_params.lab_momentum}, '{sim_params.num_events_per_sample}', '{sim_params.theta_min_in_mrad}', '{sim_params.theta_max_in_mrad}', "'${gen_filepath}'", '{sim_params.random_seed}', '{sim_params.neglect_recoil_momentum}')'")
   else: 
     os.system(f"${LMDFIT_BUILD_PATH}/bin/generatePbarPElasticScattering {sim_params.lab_momentum} {sim_params.num_events_per_sample} -l {sim_params.theta_min_in_mrad} -u {sim_params.theta_max_in_mrad} -s {sim_params.random_seed} -o ${gen_filepath}")
   os.system("cd macropath")
   print('starting up a pandaroot simulation...')
   if simulate_noise:
     os.system(f"root -l -b -q 'runLumiPixel0SimBox.C('{sim_params.num_events_per_sample}','${start_evt}',"'${workpathname}'",'$verbositylvl',2112,'{sim_params.lab_momentum}','{numTrks}','{sim_params.random_seed}')' > /dev/null 2>&1")
   else: 
     os.system(f"root -l -b -q 'runLumiPixel0SimDPM.C('{sim_params.num_events_per_sample}','${start_evt}','{sim_params.lab_momentum}',"'${gen_filepath}'", "'${workpathname}'",'$beamX0', '$beamY0', '${targetZ0}', '${beam_widthX}', '${beam_widthY}', '${target_widthZ}', '${beam_gradX}', '${beam_gradY}', '${beam_grad_sigmaX}', '${beam_grad_sigmaY}', "'${lmd_geometry_filename}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'")
   if debug:
     os.system(f"cp $workpathname/Lumi_MC_${start_evt}.root ${path_mc_data}/Lumi_MC_${start_evt}.root")
     os.system(f"cp $workpathname/Lumi_Params_${start_evt}.root ${path_mc_data}/Lumi_Params_${start_evt}.root")
else:
  if debug:
    os.system(f"cp ${path_mc_data}/Lumi_MC_${start_evt}.root $workpathname/Lumi_MC_${start_evt}.root")
    os.system(f"cp ${path_mc_data}/Lumi_Params_${start_evt}.root $workpathname/Lumi_Params_${start_evt}.root")

check_stage_success "workpathname + "/Lumi_digi_${start_evt}.root""
if 0 == "$?" or  1 == "{sim_params.force_level}":
  if simulate_noise:
    os.system(f"root -l -b -q 'runLumiPixel1bDigiNoise.C('{sim_params.num_events_per_sample}','${start_evt}',"'${workpathname}'",'$verbositylvl', '{sim_params.random_seed}')'")
  else:
    os.system(f"root -l -b -q 'runLumiPixel1Digi.C('{sim_params.num_events_per_sample}','${start_evt}',"'${workpathname}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'")

os.system(f"cd scriptpath")
os.system(f"python runLmdReco.py")
