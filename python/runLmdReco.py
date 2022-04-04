#!/usr/bin/env python3

import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file, check_stage_success
from lumifit.reconstruction import ReconstructionParameters

lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]
scriptpath = lmd_build_path + "/../python"
dirname = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
pathname = os.environ["pathname"]
macropath = os.environ["macropath"]
force_level = int(os.environ["force_level"])

debug = True
filename_index = 1
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

if not os.path.isdir(pathname):
    os.makedirs(pathname)

# TODO: check if params are loaded correctly, shouldn't be the specified file name be used?
reco_params = ReconstructionParameters(
    **load_params_from_file(pathname + "/reco_params.config")
)

# TODO: read alignment parameters correctly
ali_params = AlignmentParameters()

verbositylvl: int = 0
start_evt: int = reco_params.num_events_per_sample * filename_index

if debug:
    workpathname = pathname
    path_mc_data = workpathname
else:
    workpathname = f"/localscratch/{os.environ['SLURM_JOB_ID']}/{dirname}"

gen_filepath = workpathname + "/gen_mc.root"

# switch on "missing plane" search algorithm
misspl = True

# use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trkcut = True
if ali_params.alignment_matrices_path == "":
    trkcut = False

# merge hits on sensors from different sides. true=yes
mergedHits = True
# BOX cut before back-propagation
BoxCut = False
# Write all MC info in TrkQA array
WrAllMC = True

radLength = 0.32

prefilter = False
if reco_params.use_xy_cut:
    prefilter = True

os.chdir(macropath)
# * ------------------- Reco Step -------------------
if (
    not check_stage_success(pathname + f"/Lumi_MC_{start_evt}.root")
    or force_level == 1
):
    os.system(
        f"""root -l -b -q 'runLumiPixel2Reco.C({reco_params.num_events_per_sample},{start_evt}, "{workpathname}", "{ali_params.alignment_matrices_path}", "{ali_params.misalignment_matrices_path}", {1 if ali_params.use_point_transform_misalignment else 0}, {verbositylvl})'"""
    )

# * ------------------- Hit Merge Step -------------------
if (
    not check_stage_success(
        workpathname + f"/Lumi_recoMerged_{start_evt}.root"
    )
    or force_level == 1
):
    os.system(
        f"""root -l -b -q 'runLumiPixel2bHitMerge.C({reco_params.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl})'"""
    )

    # copy Lumi_recoMerged_ for module aligner
    os.system(
        f"cp {workpathname}/Lumi_recoMerged_{start_evt}.root {pathname}/Lumi_recoMerged_{start_evt}.root"
    )

# TODO: do this right here, not in some other scripts
# os.chdir(scriptpath)
# os.system("./runLmdPairFinder.sh")
# os.system("./runLmdTrackFinder.sh")

# * ------------------- Pair Finder Step -------------------

# TODO: store this in configuration, we don't always need hit pairs
if True:

    os.system(
        f"""root -l -b -q 'runLumiPixel2ePairFinder.C({reco_params.num_events_per_sample},{start_evt},"{workpathname}", $verbositylvl)'"""
    )

    os.system(
        f"""cp $workpathname/Lumi_Pairs_{start_evt}.root {pathname}/Lumi_Pairs_{start_evt}.root"""
    )


# * ------------------- Pixel Finder Step -------------------

echo "Kinematics cut (@LMD): $KinematicsCut"
echo "Momentum cut (after backpropagation): $CleanSig"

# TODO check if the prefilter is off at any time, add to recoparams in that case

prefilter = 1
# prefilter="false"
# if [ "$KinematicsCut" = "true" ]; then
#   prefilter="true"
# fi

# change "CA" --> "Follow" if you want to use Trk-Following as trk-search algorithm
# NB: CA can use merged or single(not merged) hits, Trk-Following can't
# check_stage_success "$workpathname/Lumi_TCand_${start_evt}.root"
# if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
#   root -l -b -q 'runLumiPixel3Finder.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',"'${track_search_algorithm}'",'${misspl}','${mergedHits}','${trkcut}','${mom}')'

#   # copy track candidates, although maybe we don't actually need them
#   # cp ${workpathname}/Lumi_TCand_${start_evt}.root ${pathname}/Lumi_TCand_${start_evt}.root
# fi

if(not check_stage_success(f"{workpathname}/Lumi_TCand_{start_evt}.root") or force_level == 1):
  os.system(f"""root -l -b -q 'runLumiPixel3Finder.C({reco_params.num_events_per_sample_evts},{start_evt},"{workpathname}",{verbositylvl},"{reco_params.track_search_algo}",{int(misspl)},{int(mergedHits)}, {int(trkcut)}, {reco_params.lab_momentum})'""")

#! ------------------- Everything below is still bash, this needs to be ported!

# * ------------------- Pixel Fitter Step -------------------

#track fit:
### Possible options: "Minuit", "KalmanGeane", "KalmanRK"
check_stage_success "$workpathname/Lumi_TrackNotFiltered_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  check_stage_success "$workpathname/Lumi_Track_${start_evt}.root"
  if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
    root -l -b -q 'runLumiPixel4Fitter.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl',"Minuit",'${mergedHits}')'
    #this script outputs a Lumi_Track_... file. Rename that to the NotFiltered..

    # copy track file for module alignment
    cp ${workpathname}/Lumi_Track_${start_evt}.root ${pathname}/Lumi_Track_${start_evt}.root

    if [ "$prefilter" = "true" ]; then
      mv ${workpathname}/Lumi_Track_${start_evt}.root ${workpathname}/Lumi_TrackNotFiltered_${start_evt}.root
    fi
  fi
fi

# * ------------------- Pixel Filter Step -------------------

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

      # don't copy to permanent storage, but don't delete this comment just yet
      cp ${workpathname}/Lumi_Track_${start_evt}.root ${pathname}/Lumi_TrackFiltered_${start_evt}.root
    fi
  fi
fi

# * ------------------- Pixel BackProp Step -------------------

# back-propgation GEANE
### Possible options: "Geane", "RK"
check_stage_success "$workpathname/Lumi_Geane_${start_evt}.root"
if [ 0 -eq "$?" ] || [ 1 -eq "${force_level}" ]; then
  root -l -b -q 'runLumiPixel5BackProp.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', "Geane", '${mergedHits}', '${mom}', '${rec_ipx}', '${rec_ipy}', '${rec_ipz}', '$prefilter')'
fi

# * ------------------- Pixel CleanSig Step -------------------

# filter back-propagated tracks (momentum cut)
if [ $CleanSig = "true" ]; then
  root -l -b -q 'runLumiPixel5bCleanSig.C('${num_evts}', '${start_evt}', "'${workpathname}'", '$verbositylvl', '${mom}', '${rec_ipx}', '${rec_ipy}')'
fi

# * ------------------- Pixel Track QA Step -------------------

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

# * ------------------- Cleanup Step -------------------

#remove everything in the local path
if [ "${debug}" -eq 0 ]; then
  rm -f $workpathname/Lumi_*_${start_evt}.root
  if [ ! "$(ls -A $workpathname)" ]; then
    rm -rf $workpathname
  fi
fi
