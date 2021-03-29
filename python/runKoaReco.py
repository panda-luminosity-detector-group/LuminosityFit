import argparse
import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file
from lumifit.reconstruction import ReconstructionParameters

#parser = argparse.ArgumentParser(
#    description="Call for important variables" " Detector.",
#    formatter_class=argparse.RawTextHelpFormatter,
#)

#parser.add_argument('--force_level', metavar='force_level', type=int, default=0,
#                    help="force level 0: if directories exist with data\n"
#                          + "files no new simulation is started\n"
#                          + "force level 1: will do full reconstruction even if "
#                          + "this data already exists, but not geant simulation\n"
#                          + "force level 2: resimulation of everything!")
#parser.add_argument('dirname', metavar='dirname', type=str, nargs=1,
#                    help='This directory for the outputfiles.')
#parser.add_argument(           
#    "path_mc_data",
#    metavar="path_mc_data",
#    type=str,
#    nargs=1,
#    help="Path to MC files.",
#)
#parser.add_argument('pathname', metavar='pathname', type=str, nargs=1,
#                    help='This the path to the outputdirectory')
#parser.add_argument('macropath', metavar='macropath', type=str, nargs=1,
#                    help='This the path to the macros')

#args = parser.parse_args()
dirname = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
pathname = os.environ["pathname"]
macropath = f"{os.environ["VMCWORKDIR"]}/macro/detector/lmd"

debug = True
if os.environ["SLURM_ARRAY_TASK_ID"]:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False
filename_index = 1
#dirname = os.path.abspath(args.dirname[0])
#path_mc_data = os.path.abspath(args.path_mc_data[0])
#pathname = os.path.abspath(args.pathname[0])
#macropath = os.path.abspath(args.macropath[0])

reco_params = ReconstructionParameters(**load_params_from_file(path_mc_data + "/recoparams.config"))
ali_params = AlignmentParameters()

verbositylvl: int = 0
start_evt: int = reco_params.num_events_per_sample * filename_index 
workpathname="/localscratch/{SLURM_JOB_ID}/{dirname}"
if debug:
    workpathname=pathname
    path_mc_data=workpathname

gen_filepath = workpathname + "/gen_mc.root"
scriptpath = os.getcwd()


#switch on "missing plane" search algorithm
misspl = True

#use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trkcut = True
#if ali_params.alignment_matrices_path = "":
#   trkcut = False

#merge hits on sensors from different sides. true=yes
mergedHits = True
## BOX cut before back-propagation
BoxCut = False
## Write all MC info in TrkQA array
WrAllMC = True

radLength = 0.32

prefilter = False
if reco_params.use_xy_cut :
  prefilter = True

def check_stage_sucess() -> bool:
        return False
print(workpathname)
os.chdir(macropath)
#check_stage_success "workpathname + "/Koala_MC_${start_evt}.root""
if not check_stage_sucess() or force_level ==  1:
  os.system(f"root -l -b -q 'KoaPixel2Reco.C({reco_params.num_events_per_sample},"
          + f"{start_evt},"
          + f"\"{workpathname}\","
         #+ f"\"{ali_params.alignment_matrices_path}\", "
         #+ f"\"{ali_params.misalignment_matrices_path}\", {ali_params.use_point_transform_misalignment},"
          + f"{verbositylvl})'"
          )


#check_stage_success "workpathname + "/Koala_reco_${start_evt}.root""
#if not check_stage_sucess() or force_level ==  1:
#  os.system(f"root -l -b -q 'LumiPixel2Reco.C({reco_params.num_events_per_sample},{start_evt},"
#          + f"\"workpathname\",verbositylvl)'")
  # copy Lumi_recoMerged_ for module aligner
#  os.system(f"cp workpathname/Koala_reco_{start_evt}.root pathname/Koala_reco_{start_evt}.root")

os.chdir(scriptpath)
#os.system("./runLmdPairFinder.sh") #only for Lmd
os.system(f"python runKoaTrack.py")

