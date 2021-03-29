import argparse
import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file
from lumifit.reconstruction import ReconstructionParameters

parser = argparse.ArgumentParser(
    description="Call for important variables" " Detector.",
    formatter_class=argparse.RawTextHelpFormatter,
)

parser.add_argument('--force_level', metavar='force_level', type=int, default=0,
                    help="force level 0: if directories exist with data\n"
                          + "files no new simulation is started\n"
                          + "force level 1: will do full reconstruction even if "
                          + "this data already exists, but not geant simulation\n"
                          + "force level 2: resimulation of everything!")
parser.add_argument('dirname', metavar='dirname', type=str, nargs=1,
                    help='This directory for the outputfiles.')
parser.add_argument(           
    "path_mc_data",
    metavar="path_mc_data",
    type=str,
    nargs=1,
    help="Path to MC files.",
)
parser.add_argument('pathname', metavar='pathname', type=str, nargs=1,
                    help='This the path to the outputdirectory')
parser.add_argument('macropath', metavar='macropath', type=str, nargs=1,
                    help='This the path to the macros')

args = parser.parse_args()

debug = 1
filename_index = 1
dirname = os.path.abspath(args.dirname[0])
path_mc_data = os.path.abspath(args.path_mc_data[0])
pathname = os.path.abspath(args.pathname[0])
macropath = os.path.abspath(args.macropath[0])

reco_params = ReconstructionParameters(**load_params_from_file(path_mc_data + "/recoparams.config"))
ali_params = AlignmentParameters()

verbositylvl: int = 0
start_evt: int = reco_params.num_events_per_sample * filename_index 
#workpathname = "lokalscratch/" + ${SLURM_JOB_ID} + "/" + dirname
workpathname = f"{os.getcwd()}/tmpOutput"
if debug:
    workpathname = path_mc_data
gen_filepath = workpathname + "/gen_mc.root"
scriptpath = os.getcwd()


#switch on "missing plane" search algorithm
misspl = True

#use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trkcut = True

track_search_algorithm = "CA"
if not trkcut:
    track_search_algorithm = "Follow"

#if ali_params.alignment_matrices_path = "":
#   trkcut = False

#merge hits on sensors from different sides. true=yes
mergedHits = False
## BOX cut before back-propagation
BoxCut = False
## Write all MC info in TrkQA array
WrAllMC = True

missalign = False
SkipFilt = False
XthFilt = False
YthFilt = False
BoxFilt = False
dX = 0
dY = 0

radLength = 0.32

prefilter = False
if reco_params.use_xy_cut :
  prefilter = True

def check_stage_sucess() -> bool:
        return False

os.chdir(macropath)
#check_stage_success "workpathname + "/Koala_TCand_${start_evt}.root""
if not check_stage_sucess() or force_level ==  1:
  os.system(f"root -l -b -q 'KoaPixel3Finder.C({reco_params.num_events_per_sample},"
          + f"{start_evt},"
          + f"\"{workpathname}\","
          + f"{verbositylvl},"
          + f"\"{track_search_algorithm}\","
          + f"{1 if misspl else 0},"
          + f"{1 if mergedHits else 0},"
          + f"{1 if trkcut else 0},"
          + f"{reco_params.lab_momentum})'"
          )


#check_stage_success "workpathname + "/Koala_reco_${start_evt}.root""
if not check_stage_sucess() or force_level ==  1:
  os.system(f"root -l -b -q 'KoaPixel4Fitter.C({reco_params.num_events_per_sample},{start_evt},"
           + f"\"{workpathname}\",{verbositylvl},"
           + f"\"Minuit\", {1 if mergedHits else 0})'")
  # copy Lumi_recoMerged_ for module aligner
  os.system(f"cp {workpathname}/Koala_Track_{start_evt}.root {pathname}/Koala_Track_{start_evt}.root")

#check_stage_success "workpathname + "/Koala_reco_${start_evt}.root""
if not check_stage_sucess() or force_level ==  1:
  os.system(f"root -l -b -q 'KoaPixel5BackProp.C({reco_params.num_events_per_sample},{start_evt},"
           + f"\"{workpathname}\",{verbositylvl},"
           + f"{1 if mergedHits else 0},"
           + f"{1 if SkipFilt else 0},"
           + f"{1 if XthFilt else 0},"
           + f"{1 if YthFilt else 0},"
           + f"{1 if BoxFilt else 0},"
           + f"{dX},"
           + f"{dY})'"
           )

  #check_stage_success "workpathname + "/Koala_reco_${start_evt}.root""
if not check_stage_sucess() or force_level ==  1:
  os.system(f"root -l -b -q 'KoaPixel6Compare.C({reco_params.num_events_per_sample},{start_evt},"
           + f"\"{workpathname}\",{verbositylvl},"
           + f"{1 if missalign else 0})'")
        # copy Lumi_recoMerged_ for module aligner
  os.system(f"cp {workpathname}/Koala_comp_{start_evt}.root {pathname}Koala_comp_{start_evt}.root")

os.chdir(scriptpath)


