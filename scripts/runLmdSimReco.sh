#!/bin/sh

#include some helper functions
. ./bashFunctions.sh

scriptpath=$PWD
macropath="${VMCWORKDIR}/macro/detectors/lmd"
cd $macropath

filename_index=1
debug=1
if [ ${SLURM_ARRAY_TASK_ID} ]; then
  filename_index=${SLURM_ARRAY_TASK_ID}
  debug=0
fi

random_seed=$((${random_seed}+${filename_index}))
echo "using random seed: ${random_seed}"

if [ ! -d $pathname ]; then
  mkdir -p $pathname
fi

verbositylvl=0
start_evt=$((${num_evts}*${filename_index})) #number of events * filename index is startevt

#number of trks per event (should not be changed)
numTrks=1

#ok we want to simulate only on the node so also the output files of the simulation so change the pathname to /local/scratch/dirname
dirname=`echo $dirname | sed -e 's/\//_/g'`

workpathname="/localscratch/${SLURM_JOB_ID}/${dirname}"
if [ "${debug}" -eq 1 ]; then
  workpathname="${pathname}"
  path_mc_data=$workpathname
fi
if [ ! -d $workpathname ]; then
  mkdir -p $workpathname
fi

gen_filepath="$workpathname/gen_mc.root"
echo "force level: ${force_level}"
echo "debug: ${debug}"
echo "reaction type: ${reaction_type}"

#simulation
check_stage_success "${path_mc_data}/Lumi_MC_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 2 -eq "${force_level}" ]; then
  cd $scriptpath
  if [ ${reaction_type} -eq -1 ]; then
    echo "generating box MC sample"
    root -l -b -q 'standaloneBoxGen.C('${mom}', '${num_evts}', '${theta_min_in_mrad}', '${theta_max_in_mrad}', "'${gen_filepath}'", '${random_seed}', '${use_recoil_momentum}')'
  else
    echo "generating dpm MC sample"
    echo "Running generatePbarPElasticScattering ${mom} ${num_evts} -l ${theta_min_in_mrad} -u ${theta_max_in_mrad} -s ${random_seed} -o ${gen_filepath}"
    ${LMDFIT_BUILD_PATH}/bin/generatePbarPElasticScattering ${mom} ${num_evts} -l ${theta_min_in_mrad} -u ${theta_max_in_mrad} -s ${random_seed} -o ${gen_filepath}
  fi
  cd $macropath
  echo "starting up a pandaroot simulation..."
  if [ ${simulate_noise} ]; then
    #simulation with Box generator cheating with neutrons, since "no tracks" gave problems
    root -l -b -q 'runLumiPixel0SimBox.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',2112,'${mom}','${numTrks}','${random_seed}')' > /dev/null 2>&1
  else
    #root -l -b -q 'runLumiPixel0SimDPM.C('${num_evts}','${start_evt}','${mom}',"'${gen_filepath}'", "'${workpathname}'",'$beamX0', '$beamY0', '${targetZ0}', '${beam_widthX}', '${beam_widthY}', '${target_widthZ}', '${beam_gradX}', '${beam_gradY}', '${beam_grad_sigmaX}', '${beam_grad_sigmaY}', "'${lmd_geometry_filename}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'
    root -l -b -q 'runLumiPixel0SimDPM.C('${num_evts}','${start_evt}','${mom}',"'${gen_filepath}'", "'${workpathname}'",'$beamX0', '$beamY0', '${targetZ0}', '${beam_widthX}', '${beam_widthY}', '${target_widthZ}', '${beam_gradX}', '${beam_gradY}', '${beam_grad_sigmaX}', '${beam_grad_sigmaY}', "'${lmd_geometry_filename}'", '$verbositylvl')'
  fi
  if [ "${debug}" -eq 0 ]; then 
    cp $workpathname/Lumi_MC_${start_evt}.root ${path_mc_data}/Lumi_MC_${start_evt}.root
    cp $workpathname/Lumi_Params_${start_evt}.root ${path_mc_data}/Lumi_Params_${start_evt}.root
  fi
else
  if [ "${debug}" -eq 0 ]; then
    cp ${path_mc_data}/Lumi_MC_${start_evt}.root $workpathname/Lumi_MC_${start_evt}.root
    cp ${path_mc_data}/Lumi_Params_${start_evt}.root $workpathname/Lumi_Params_${start_evt}.root
  fi
fi

check_stage_success "$workpathname/Lumi_digi_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  if [ ${simulate_noise} ]; then
    root -l -b -q 'runLumiPixel1bDigiNoise.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl', '${random_seed}')'
  else 
    #root -l -b -q 'runLumiPixel1Digi.C('${num_evts}','${start_evt}',"'${workpathname}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'
    root -l -b -q 'runLumiPixel1Digi.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl')'
  fi
fi

cd $scriptpath
./runLmdReco.sh
