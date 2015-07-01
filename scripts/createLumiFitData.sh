#!/bin/bash 
###############################################################################
## script for creating LMD data used for the fitting procedure etc.
## for batch use run submit script version of create LumiFit data instead
###############################################################################

batchjob=0
if [ $PBS_O_WORKDIR ]; then
  batchjob=1
fi

# lab momentum of the beam antiprotons
#pbeam=$var1
#number of events used for the fitting procedure
#numEv=$var2
# directory of the data generated with dpm
if [[ ${PBS_ARRAYID} ]]; then
  filelist_url=${filelist_path}/filelist_${PBS_ARRAYID}.txt
else
  filelist_url=${filelist_path}
fi
echo "data path: ${input_path}!"
#type=$var4
echo "data type: ${type}"
#elastic_cross_section=$var5
echo "elastic cross secion: ${elastic_cross_section}"
if [ ! $numEv ]; then
  numEv=0
fi

if [[ $type == *a* ]]; then
  if [ -z "${elastic_cross_section}" ]; then
    dpm_logfile=${GEN_DATA}/`echo ${data_path} | sed -rn 's/^.*\/(.*)_pixel.*$/\1/p'`/*_1.log
    elastic_cross_section=$(cat ${dpm_logfile} | sed -rn 's/elastic cross section[ ]*([0-9]*.[0-9]*).*/\1/p')
    echo cross section is ${elastic_cross_section}
  fi
fi
  
echo "using elastic cross section of ${elastic_cross_section}"
if [ -z ${filelist_url} ]; then
  echo $VMCWORKDIR/build/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -n ${numEv} -e ${elastic_cross_section}
  if [ $batchjob -eq "0" ]; then
    $VMCWORKDIR/build/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -n ${numEv} -e ${elastic_cross_section} 2>&1 >> ${data_path}/createLumiFitData.log
  else
    $VMCWORKDIR/build/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -n ${numEv} -e ${elastic_cross_section}
  fi
else
  echo $VMCWORKDIR/build/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -f ${filelist_url} -o ${output_path} -n ${numEv} -e ${elastic_cross_section}
  if [ $batchjob -eq "0" ]; then
    $VMCWORKDIR/build/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -f ${filelist_url} -o ${output_path} -n ${numEv} -e ${elastic_cross_section} 2>&1 >> ${data_path}/createLumiFitData.log
  else
    $VMCWORKDIR/build/bin/createLmdFitData -m $pbeam -t $type -c ${config_path} -d ${input_path} -f ${filelist_url} -o ${output_path} -n ${numEv} -e ${elastic_cross_section}
  fi
fi
  
sleep 10;
exit 0;
