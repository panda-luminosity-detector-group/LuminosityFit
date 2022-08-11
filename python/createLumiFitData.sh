#!/bin/bash 
###############################################################################
## script for creating LMD data used for the fitting procedure etc.
## for batch use run submit script version of create LumiFit data instead
###############################################################################

batchjob=0
if [ $SLURM_ARRAY_TASK_ID ]; then
  batchjob=1
fi

# directory of the data generated with dpm
if [[ ${SLURM_ARRAY_TASK_ID} ]]; then
  filelist_url=${filelist_path}/filelist_${SLURM_ARRAY_TASK_ID}.txt
else
  filelist_url=${filelist_path}
fi
echo "data path: ${input_path}!"
echo "data type: ${type}"
echo "config: ${config_path}"
echo "elastic cross secion: ${elastic_cross_section}"
if [ ! $numEv ]; then
  numEv=0
fi

# if {filelist_url} is empty, the createLmdFitData binary must be run
if [ -z ${filelist_url} ]; then
  echo ${LMDFIT_BUILD_PATH}/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -n ${numEv} -e ${elastic_cross_section}
  if [ $batchjob -eq "0" ]; then
    ${LMDFIT_BUILD_PATH}/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -n ${numEv} -e ${elastic_cross_section} 2>&1 >> ${data_path}/createLumiFitData.log
  else
    ${LMDFIT_BUILD_PATH}/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -n ${numEv} -e ${elastic_cross_section}
  fi
else
  echo $/build/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -f ${filelist_url} -o ${output_path} -n ${numEv} -e ${elastic_cross_section}
  if [ $batchjob -eq "0" ]; then
    ${LMDFIT_BUILD_PATH}/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -f ${filelist_url} -o ${output_path} -n ${numEv} -e ${elastic_cross_section} 2>&1 >> ${data_path}/createLumiFitData.log
  else
    ${LMDFIT_BUILD_PATH}/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -f ${filelist_url} -o ${output_path} -n ${numEv} -e ${elastic_cross_section}
  fi
fi
  
sleep 10;
exit 0;
