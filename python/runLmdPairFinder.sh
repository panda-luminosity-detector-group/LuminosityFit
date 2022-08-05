#!/bin/sh

echo =======================================================
echo ====   DEPRECATED. DO NOT USE ANYMORE              ====
echo ====   KEEP ONLY FOR REFERENCE, OR DELETE          ====
echo =======================================================

exit 1


# if scriptpath or workpathname is not set then this reco script was executed itself
if [ -z $scriptpath ] || [ -z $workpathname ]; then
  #include some helper functions
  . ./bashFunctions.sh

  macropath="${VMCWORKDIR}/macro/detectors/lmd"
  cd $macropath

  filename_index=1
  debug=1
  if [ ${SLURM_ARRAY_TASK_ID} ]; then
    filename_index=${SLURM_ARRAY_TASK_ID}
    debug=0
  fi

  if [ ! -d $pathname ]; then
    mkdir -p $pathname
  fi

  verbositylvl=0
  start_evt=$((${num_evts} * ${filename_index})) #number of events * filename index is startevt

  #ok we want to simulate only on the node so also the output files of the simulation so change the pathname to /local/scratch/dirname
  dirname=$(echo $dirname | sed -e 's/\//_/g')

  workpathname="/localscratch/${SLURM_JOB_ID}/${dirname}"
  if [ "${debug}" -eq 1 ]; then
    workpathname="${pathname}"
  fi
  if [ ! -d $workpathname ]; then
    mkdir -p $workpathname
  fi
  echo "force level: ${force_level}"
fi

#hit reco
check_stage_success "$workpathname/Lumi_reco_${start_evt}.root"
if [ 1 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  root -l -b -q 'runLumiPixel2ePairFinder.C('${num_evts}','${start_evt}',"'${workpathname}'", '$verbositylvl')'
  cp $workpathname/Lumi_Pairs_${start_evt}.root $pathname/Lumi_Pairs_${start_evt}.root
else
  echo "Warning! Cannot run pair finder, no reco files are found!\n"
fi
