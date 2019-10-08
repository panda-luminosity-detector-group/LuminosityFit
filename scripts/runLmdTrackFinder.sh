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
  start_evt=$((${num_evts}*${filename_index})) #number of events * filename index is startevt


  #ok we want to simulate only on the node so also the output files of the simulation so change the pathname to /local/scratch/dirname
  dirname=`echo $dirname | sed -e 's/\//_/g'`

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
## Write all MC info in TrkQA array
WrAllMC=true

radLength=0.32

echo "Kinematics cut (@LMD): $KinematicsCut"
echo "Momentum cut (after backpropagation): $CleanSig"

prefilter="false"
if [ "$KinematicsCut" = "true" ]; then
prefilter="true"
fi

### change "CA" --> "Follow" if you want to use Trk-Following as trk-search algorithm
### NB: CA can use merged or single(not merged) hits, Trk-Following can't
check_stage_success "$workpathname/Lumi_TCand_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  root -l -b -q 'runLumiPixel3Finder.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',"'${track_search_algorithm}'",'${misspl}','${mergedHits}','${trkcut}','${mom}')'
fi

#track fit:
### Possible options: "Minuit", "KalmanGeane", "KalmanRK"
check_stage_success "$workpathname/Lumi_TrackNotFiltered_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
	check_stage_success "$workpathname/Lumi_Track_${start_evt}.root"
	if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
		root -l -b -q 'runLumiPixel4Fitter.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',"Minuit",'${mergedHits}')'
    #this script outputs a Lumi_Track_... file. Rename that to the NotFiltered..
    
    # copy track file for module alignment
    cp $workpathname/Lumi_Track_${start_evt}.root $pathname/Lumi_Track_${start_evt}.root
		
    if [ "$prefilter" = "true" ]; then
			mv ${workpathname}/Lumi_Track_${start_evt}.root ${workpathname}/Lumi_TrackNotFiltered_${start_evt}.root
		fi
	fi
fi

#track filter (on number of hits and chi2 and optionally on track kinematics)

if [ "$prefilter" = "true" ]; then
check_stage_success "$workpathname/Lumi_Track_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  check_stage_success "$workpathname/Lumi_TrackFiltered_${start_evt}.root"
  if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
    #this macro needs Lumi_Track_... file as input so we need to link the unfiltered file
    ln -sf ${workpathname}/Lumi_TrackNotFiltered_${start_evt}.root ${workpathname}/Lumi_Track_${start_evt}.root

    root -l -b -q 'runLumiPixel4aFilter.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', '${mergedHits}', '${mom}', '${KinematicsCut}', '${rec_ipx}', '${rec_ipy}')'
   
    #now overwrite the Lumi_Track_ sym link with the filtered version
    ln -sf ${workpathname}/Lumi_TrackFiltered_${start_evt}.root ${workpathname}/Lumi_Track_${start_evt}.root
  fi
fi
fi


# back-propgation GEANE
### Possible options: "Geane", "RK"
check_stage_success "$workpathname/Lumi_Geane_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
root -l -b -q 'runLumiPixel5BackProp.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', "Geane", '${mergedHits}', '${mom}', '${rec_ipx}', '${rec_ipy}', '${rec_ipz}', '$prefilter')'
fi


# filter back-propagated tracks (momentum cut)
if [ $CleanSig = "true" ]; then 
  root -l -b -q 'runLumiPixel5bCleanSig.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', '${mom}', '${rec_ipx}', '${rec_ipy}')'
fi

# # Quality assurance task(s)
# combine MC and reco information
# the last parameter is mc all write flag and needs to be true
# so that all mc events are written even if geometrically missing the sensors
# this is required for the acceptance calculation
check_stage_success "$workpathname/Lumi_TrksQA_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  root -l -b -q 'runLumiPixel7TrksQA.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl','${mom}', '$WrAllMC', '${KinematicsCut}', '${CleanSig}')'
  if [ "${debug}" -eq 0 ]; then
    cp $workpathname/Lumi_TrksQA_${start_evt}.root $pathname/Lumi_TrksQA_${start_evt}.root
  fi
fi

#remove everything in the local path
if [ "${debug}" -eq 0 ]; then
  rm -f $workpathname/Lumi_*_${start_evt}.root
  if [ ! "$(ls -A $workpathname)" ]; then
    rm -rf $workpathname
  fi
fi
