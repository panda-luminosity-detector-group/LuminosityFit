#!/bin/sh

#include some helper functions
. ./bashFunctions.sh

cd ${VMCWORKDIR}/macro/detectors/lmd

filename_index=1
if [ ${SLURM_ARRAY_TASK_ID} ]; then
  filename_index=${SLURM_ARRAY_TASK_ID}
fi

gen_input_filename="${gen_input_file_stripped}_${filename_index}.root"

if [ ! -d $pathname ]; then
  mkdir -p $pathname
fi

verbositylvl=0
start_evt=$((${num_evts}*${filename_index})) #number of events * filename index is startevt

#switch on "missing plane" search algorithm
misspl=true
#use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trkcut=true
#merge hits on sensors from different sides. true=yes
mergedHits=true
#Skip kinematic filter (before back-propagation)
#SkipFilt=true
## if SkipFilt=false (XThetaCut or YPhiCut) or BoxCut should be true:
## X-Theta kinematic cut before back-propagation
#XThetaCut=true
#YPhiCut=true
## BOX cut before back-propagation
BoxCut=false
## Clean after back-propagation (momentum or MVA cut)
#CleanSig=false

radLength=0.32

echo "xthetacut: $XThetaCut"
echo "yphicut: $YPhiCut"
echo "mcut: $CleanSig"

prefilter="false"
if [ "$XThetaCut" == "true" ] || [ "$YPhiCut" == "true" ]; then
prefilter="true"
fi

## Write all MC info in TrkQA array
WrAllMC=true

#number of trks per event (should not be changed)
numTrks=1

#ok we want to simulate only on the node so also the output files of the simulation so change the pathname to /local/scratch/dirname
dirname=`echo $dirname | sed -e 's/\//_/g'`

workpathname="/localscratch/${SLURM_JOB_ID}/${dirname}"
if [ ! -d $workpathname ]; then
  mkdir -p $workpathname
fi

echo "force level: ${force_level}"
#simulation
check_stage_success "${path_mc_data}/Lumi_MC_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 2 -eq "${force_level}" ]; then
  echo "starting up a pandaroot simulation..."
  if [ ${simulate_noise} ]; then
    #simulation with Box generator cheating with neutrons, since "no tracks" gave problems
    root -l -b -q 'runLumiPixel0SimBox.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',2112,'${mom}','${numTrks}','$RANDOM')' > /dev/null 2>&1
  else
    root -l -b -q 'runLumiPixel0SimDPM.C('${num_evts}','${start_evt}','${mom}',"'${gen_input_filename}'", "'${workpathname}'",'$beamX0', '$beamY0', '${targetZ0}', '${beam_widthX}', '${beam_widthY}', '${target_widthZ}', '${beam_gradX}', '${beam_gradY}', '${beam_grad_sigmaX}', '${beam_grad_sigmaY}', "'${lmd_geometry_filename}'", "'${misalignment_matrices_path}'", '$verbositylvl')'
  fi
  cp $workpathname/Lumi_MC_${start_evt}.root ${path_mc_data}/Lumi_MC_${start_evt}.root
  cp $workpathname/Lumi_Params_${start_evt}.root ${path_mc_data}/Lumi_Params_${start_evt}.root
else
  cp ${path_mc_data}/Lumi_MC_${start_evt}.root $workpathname/Lumi_MC_${start_evt}.root
  cp ${path_mc_data}/Lumi_Params_${start_evt}.root $workpathname/Lumi_Params_${start_evt}.root
fi

#digitization
check_stage_success "$workpathname/Lumi_digi_${start_evt}.root"
if [ 0 -eq "$?" ]; then
  if [ ${simulate_noise} ]; then
    root -l -b -q 'runLumiPixel1bDigiNoise.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl', '$RANDOM')'
  else 
    root -l -b -q 'runLumiPixel1Digi.C('${num_evts}','${start_evt}',"'${workpathname}'", "'${misalignment_matrices_path}'", '$verbositylvl')'
  fi
fi

check_stage_success "$workpathname/Lumi_reco_${start_evt}.root"
if [ 0 -eq "$?" ]; then
  root -l -b -q 'runLumiPixel2Reco.C('${num_evts}','${start_evt}',"'${workpathname}'", "'${alignment_matrices_path}'", '$verbositylvl')'
fi

#merge hits
check_stage_success "$workpathname/Lumi_recoMerged_${start_evt}.root"
if [ 0 -eq "$?" ]; then
  root -l -b -q 'runLumiPixel2bHitMerge.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl')'
fi

### change "CA" --> "Follow" if you want to use Trk-Following as trk-search algorithm
### NB: CA can use merged or single(not merged) hits, Trk-Following can't
check_stage_success "$workpathname/Lumi_TCand_${start_evt}.root"
if [ 0 -eq "$?" ]; then
  root -l -b -q 'runLumiPixel3Finder.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',"'${track_search_algorithm}'",'${misspl}','${mergedHits}','${trkcut}','${mom}')'
fi

#track fit:
### Possible options: "Minuit", "KalmanGeane", "KalmanRK"
check_stage_success "$workpathname/Lumi_TrackNotFiltered_${start_evt}.root"
if [ 0 -eq "$?" ]; then
	check_stage_success "$workpathname/Lumi_Track_${start_evt}.root"
	if [ 0 -eq "$?" ]; then
		root -l -b -q 'runLumiPixel4Fitter.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',"Minuit",'${mergedHits}')'
  		#this script output a Lumi_Track_... file. Rename that to the NotFiltered..

		if [ "$prefilter" == "true" ]; then
			mv ${workpathname}/Lumi_Track_${start_evt}.root ${workpathname}/Lumi_TrackNotFiltered_${start_evt}.root
		fi
	fi
fi

#track filter (on number of hits and chi2)

if [ "$prefilter" == "true" ]; then
check_stage_success "$workpathname/Lumi_Track_${start_evt}.root"
if [ 0 -eq "$?" ]; then
  check_stage_success "$workpathname/Lumi_TrackFiltered_${start_evt}.root"
  if [ 0 -eq "$?" ]; then
    #this macro needs Lumi_Track_... file as input so we need to link the unfiltered file
    ln -sf ${workpathname}/Lumi_TrackNotFiltered_${start_evt}.root ${workpathname}/Lumi_Track_${start_evt}.root

    root -l -b -q 'runLumiPixel4aFilter.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', '${mergedHits}', '${SkipFilt}', '${XThetaCut}', '${YPhiCut}', '${BoxCut}', '${rec_ipx}', '${rec_ipy}')'
   
    #now overwrite the Lumi_Track_ sym link with the filtered version
    ln -sf ${workpathname}/Lumi_TrackFiltered_${start_evt}.root ${workpathname}/Lumi_Track_${start_evt}.root
  fi
fi
fi


# back-propgation GEANE
### Possible options: "Geane", "RK"
check_stage_success "$workpathname/Lumi_Geane_${start_evt}.root"
if [ 0 -eq "$?" ]; then
root -l -b -q 'runLumiPixel5BackProp.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', "Geane", '${mergedHits}', '${mom}', '${rec_ipx}', '${rec_ipy}', '${rec_ipz}', '$prefilter')'
fi


# filter back-propagated tracks (momentum cut)
if [ $CleanSig == "true" ]; then 
  root -l -b -q 'runLumiPixel5bCleanSig.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', '${mom}', '${rec_ipx}', '${rec_ipy}')'
fi

# # Quality assurance task(s)
# combine MC and reco information
# the last parameter is mc all write flag and needs to be true
# so that all mc events are written even if geometrically missing the sensors
# this is required for the acceptance calculation
check_stage_success "$workpathname/Lumi_TrksQA_${start_evt}.root"
if [ 0 -eq "$?" ]; then
	root -l -b -q 'runLumiPixel7TrksQA.C('${num_evts}','${start_evt}',"'${workpathname}'",'0','${mom}', '$WrAllMC', '${CleanSig}')'
	cp $workpathname/Lumi_TrksQA_${start_evt}.root $pathname/Lumi_TrksQA_${start_evt}.root
fi

#remove everything in the local path
rm -f $workpathname/Lumi_*_${start_evt}.root
if [ ! "$(ls -A $workpathname)" ]; then
  rm -rf $workpathname
fi
