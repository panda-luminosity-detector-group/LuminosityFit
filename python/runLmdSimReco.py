import json
import os

#read json file
with open('../simconfig.json','r') as simconfig:
    data = simconfig.read()

#parse json file
obj = json.loads(data)


class SimulationParamteres():

    def _init_(self, macropath = ${VMCWORKDIR}"/macro/detector/lmd", pathname, scriptpath = $pwd, random_seed, dirname, workpathname = "lokalscratch/" + ${SLURM_JOB_ID} + "/" + dirname, gen_filepath = workpathname + "/gen_mc.root")
        self.macropath = ""
        self.pathname = ""
        self.scriptpath = ""
        self.random_seed: int = $((${random_seed}+${filename_index}))
        self.dirname = ""
        self.workpathname = ""
        self.gen_filepath = ""

if ! -d pathname
 os.system("mkdir -p pathname")

verbosityLevel: int = 0
start_evt: int = $((${num_evts}*${filename_index}))
numTracks: int = 1 #do not change

#simulation

check_stage_success "path_mc_data + "/Lumi_MC_${start_evt}.root""
if 0 -eq "$?" or  2 -eq "force_level"
   os.system("cd scriptpath")
   if ${reaction_type} -eq -1
     os.system("root -l -b -q 'standaloneBoxGen.C('${mom}', '${num_evts}', '${theta_min_in_mrad}', '${theta_max_in_mrad}', "'${gen_filepath}'", '${random_seed}', '${use_recoil_momentum}')'")
   else 
     os.system("${LMDFIT_BUILD_PATH}/bin/generatePbarPElasticScattering ${mom} ${num_evts} -l ${theta_min_in_mrad} -u ${theta_max_in_mrad} -s ${random_seed} -o ${gen_filepath}")
   os.system("cd macropath")
   print('starting up a pandaroot simulation...')
   if simulate_noise
     os.system("root -l -b -q 'runLumiPixel0SimBox.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',2112,'${mom}','${numTrks}','${random_seed}')' > /dev/null 2>&1")
   else 
     os.system("root -l -b -q 'runLumiPixel0SimDPM.C('${num_evts}','${start_evt}','${mom}',"'${gen_filepath}'", "'${workpathname}'",'$beamX0', '$beamY0', '${targetZ0}', '${beam_widthX}', '${beam_widthY}', '${target_widthZ}', '${beam_gradX}', '${beam_gradY}', '${beam_grad_sigmaX}', '${beam_grad_sigmaY}', "'${lmd_geometry_filename}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'")
   if debug
     os.system("cp $workpathname/Lumi_MC_${start_evt}.root ${path_mc_data}/Lumi_MC_${start_evt}.root")
     os.system("cp $workpathname/Lumi_Params_${start_evt}.root ${path_mc_data}/Lumi_Params_${start_evt}.root")
else
  if debug
    os.system("cp ${path_mc_data}/Lumi_MC_${start_evt}.root $workpathname/Lumi_MC_${start_evt}.root")
    os.system("cp ${path_mc_data}/Lumi_Params_${start_evt}.root $workpathname/Lumi_Params_${start_evt}.root")

check_stage_success "workpathname + "/Lumi_digi_${start_evt}.root""
if 0 -eq "$?" or  1 -eq "${force_level}"
  if simulate_noise
    os.system("root -l -b -q 'runLumiPixel1bDigiNoise.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl', '${random_seed}')'")
  else
    os.system("root -l -b -q 'runLumiPixel1Digi.C('${num_evts}','${start_evt}',"'${workpathname}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'")
     
os.system("cd scriptpath")
os.system("python runLmdReco.py")
