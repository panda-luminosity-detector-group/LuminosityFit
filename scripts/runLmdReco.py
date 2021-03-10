#!/bin/sh

# if scriptpath or workpathname is not set then this reco script was executed itself
if [ -z $scriptpath ] || [ -z $workpathname ]; then
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

#switch on "missing plane" search algorithm
misspl=true
#use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trkcut=true
if [ "${alignment_matrices_path}" = "" ]; then
  trkcut=false
fi
#merge hits on sensors from different sides. true=yes
mergedHits=true
## BOX cut before back-propagation
BoxCut=false
## Write all MC info in TrkQA array
WrAllMC=true

radLength=0.32

echo "xthetacut: $XThetaCut"
echo "yphicut: $YPhiCut"
echo "mcut: $CleanSig"

prefilter="false"
if [ "$XThetaCut" = "true" ] || [ "$YPhiCut" = "true" ]; then
  prefilter="true"
fi

#hit reco
check_stage_success "$workpathname/Lumi_reco_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  root -l -b -q 'runLumiPixel2Reco.C('${num_evts}','${start_evt}',"'${workpathname}'", "'${alignment_matrices_path}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'
  # cp $workpathname/Lumi_reco_${start_evt}.root $pathname/Lumi_reco_${start_evt}.root
fi

#merge hits
check_stage_success "$workpathname/Lumi_recoMerged_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  root -l -b -q 'runLumiPixel2bHitMerge.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl')'

  # copy Lumi_recoMerged_ for module aligner
  cp $workpathname/Lumi_recoMerged_${start_evt}.root $pathname/Lumi_recoMerged_${start_evt}.root
fi

cd $scriptpath
./runLmdPairFinder.sh
./runLmdTrackFinder.sh
