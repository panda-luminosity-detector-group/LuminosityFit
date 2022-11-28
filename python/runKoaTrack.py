#!/usr/bin/env python3

import os
import subprocess

from lumifit.alignment import AlignmentParameters
from lumifit.general import check_stage_success, load_params_from_file
from lumifit.reconstruction import ReconstructionParameters

debug = True
filename_index = 1
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

dirname = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
pathname = os.environ["pathname"]
macropath = os.environ["LMDFIT_MACROPATH"]
force_level = int(os.environ["force_level"])

reco_param: ReconstructionParameters = load_params_from_file(
    pathname + "/reco_params.config", ReconstructionParameters
)
ali_params = AlignmentParameters()      # TODO Alignment with KOALA isn't implemented yet.

verbositylvl: int = 0
start_evt: int = reco_param.num_events_per_sample * filename_index

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

# if ali_params.alignment_matrices_path = "":
#   trkcut = False

# merge hits on sensors from different sides. true=yes
mergedHits = False
# BOX cut before back-propagation
BoxCut = False
# Write all MC info in TrkQA array
WrAllMC = True

missalign = False
SkipFilt = False
XthFilt = False
YthFilt = False
BoxFilt = False
dX = 0
dY = 0

radLength = 0.32# merge hits on sensors from different sides. true=yes# merge hits on sensors from different sides. true=yes

prefilter = False
if reco_param.use_xy_cut:
    prefilter = True


os.chdir(macropath)
# check_stage_success "workpathname + "/Koala_TCand_${start_evt}.root""
if (
    not check_stage_success(pathname + f"/Koala_Track_{start_evt}.root")
    or force_level == 1
):
    os.chdir(macropath)
    
    os.system(
        f"root -l -b -q 'KoaPixel3Finder.C({reco_param.num_events_per_sample},"
        + f"{start_evt},"
        + f'"{workpathname}",'
        + f"{verbositylvl},"
        + f'"{reco_param.track_search_algo}",'
        + f"{1 if misspl else 0},"
        + f"{1 if mergedHits else 0},"
        + f"{1 if trkcut else 0},"
        + f"{reco_param.lab_momentum})'"
    )
    os.chdir(macropath)
    os.system(
        f"root -l -b -q 'KoaPixel4Fitter.C({reco_param.num_events_per_sample},{start_evt},"
        + f'"{workpathname}",{verbositylvl},'
        + f'"Minuit", {1 if mergedHits else 0})\''
    )
    # copy Lumi_recoMerged_ for module aligner
    os.system(
        f"cp {workpathname}/Koala_Track_{start_evt}.root {pathname}/Koala_Track_{start_evt}.root"
    )
else:
    if not debug:            
        os.system(
            f"cp {pathname}/Koala_Track_{start_evt}.root {workpathname}/Koala_Track_{start_evt}.root"
        )

if (
    not check_stage_success(pathname + f"/Koala_comp_{start_evt}.root")
    or force_level == 1
):
    os.chdir(macropath)
    
    returnvalue = subprocess.run(
        f"root -l -b -q 'KoaPixel5BackProp.C({reco_param.num_events_per_sample},{start_evt},"
        + f'"{workpathname}",{verbositylvl},'
        + f"{1 if mergedHits else 0},"
        + f"{1 if SkipFilt else 0},"
        + f"{1 if XthFilt else 0},"
        + f"{1 if YthFilt else 0},"
        + f"{1 if BoxFilt else 0},"
        + f"{dX},"
        + f"{dY})'",
        shell=True,
    )

    os.chdir(macropath)
    
    os.system(
        f"root -l -b -q 'KoaPixel6Compare.C({reco_param.num_events_per_sample},{start_evt},"
        + f'"{workpathname}",{verbositylvl},'
        + f"{1 if missalign else 0})'"
    )
    # copy Koala_comp
    os.system(
        f"cp {workpathname}/Koala_comp_{start_evt}.root {pathname}/Koala_comp_{start_evt}.root"
    )

    os.chdir(macropath)
    os.system(
        f"root -l -b -q 'KoaPixel7Unify.C({reco_param.num_events_per_sample},{start_evt},"
        + f'"{workpathname}",{verbositylvl},'
        + f"{1 if missalign else 0})'"
    )
    # copy Koala_IP for determineLuminosity
    os.system(
        f"cp {workpathname}/Koala_IP_{start_evt}.root {pathname}/Koala_IP_{start_evt}.root"
    )


    os.chdir(macropath)
    os.system(
        f"root -l -b -q 'KoaPixel7Unify.C({reco_param.num_events_per_sample},{start_evt},"
        + f'"{workpathname}",{verbositylvl},'
        + f"{1 if missalign else 0})'"
    )
    # copy Koala_IP for determineLuminosity
    os.system(
        f"cp {workpathname}/Koala_IP_{start_evt}.root {pathname}/Koala_IP_{start_evt}.root"
    )

