import argparse
import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file
from lumifit.simulation import SimulationParameters, SimulationType

# parser = argparse.ArgumentParser(
#     description="Call for important variables" " Detector.",
#     formatter_class=argparse.RawTextHelpFormatter,
# )

# parser.add_argument(
#     "--force_level",
#     metavar="force_level",
#     type=int,
#     default=0,
#     help="force level 0: if directories exist with data\n"
#     + "files no new simulation is started\n"
#     + "force level 1: will do full reconstruction even if "
#     + "this data already exists, but not geant simulation\n"
#     + "force level 2: resimulation of everything!",
# )
# parser.add_argument(
#      "dirname",
#      metavar="dirname",
#      type=str,
#      nargs=1,
#      help="This directory for the outputfiles.",
#  )
# parser.add_argument(
#     "path_mc_data",
#     metavar="path_mc_data",
#     type=str,
#     nargs=1,
#     help="Path to MC files.",
# )
# parser.add_argument(
#     "pathname",
#     metavar="pathname",
#     type=str,
#     nargs=1,
#     help="This the path to the outputdirectory",
# )
# parser.add_argument('macropath', metavar='macropath', type=str, nargs=1,
#                     help='This the path to the macros')

# args = parser.parse_args()


# force_level = args.force_level
# dirname = args.dirname[0]
# path_mc_data = os.path.abspath(args.path_mc_data[0])
# pathname = os.path.abspath(args.pathname[0])
# macropath = os.path.abspath(args.macropath[0])

dirname = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
pathname = os.environ["pathname"]
macropath = f"{os.environ["VMCWORKDIR"]}/macro/detector/lmd"

filename_index = 1
debug = True
if os.environ["SLURM_ARRAY_TASK_ID"]:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

sim_params = SimulationParameters(
    **load_params_from_file(path_mc_data + "/simparams.config")
)

ali_params = AlignmentParameters()

workpathname="/localscratch/{SLURM_JOB_ID}/{dirname}"
if debug:
    workpathname=pathname
    path_mc_data=workpathname

if not os.path.exists(workpathname):
    os.makedirs(workpathname)

gen_filepath = workpathname + "/gen_mc.root"
scriptpath = os.getcwd()
lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]

verbositylvl = 0
start_evt: int = sim_params.num_events_per_sample * filename_index
numTracks: int = 1  # do not change

# simulation
def check_stage_sucess() -> bool:
    return False


# check_stage_success "args.path_mc_data + "/Koala_MC_${start_evt}.root""
if not check_stage_sucess() or force_level == 2:
    os.chdir(scriptpath)
    if sim_params.sim_type == SimulationType.BOX:
        os.system(
            f"root -l -b -q 'standaloneBoxGen.C({sim_params.lab_momentum}, "
            + f"{sim_params.num_events_per_sample}, "
            + f"{sim_params.theta_min_in_mrad}, "
            + f"{sim_params.theta_max_in_mrad}, "
            + f"\"{gen_filepath}\", {sim_params.random_seed}, "
            + f"{sim_params.neglect_recoil_momentum})'"
        )
    elif sim_params.sim_type == SimulationType.PBARP_ELASTIC:
        os.system(
            f"{lmd_build_path}/bin/generatePbarPElasticScattering"
            + f" {sim_params.lab_momentum} {sim_params.num_events_per_sample}"
            + f" -l {sim_params.theta_min_in_mrad}"
            + f" -u {sim_params.theta_max_in_mrad} -s {sim_params.random_seed}"
            + f" -o {gen_filepath}"
        )
    os.chdir(macropath)
    print("starting up a koalasoft simulation...")
    os.system(
        f"root -l -b -q 'KoaPixel0SimExtern.C({sim_params.num_events_per_sample}, {start_evt}, {sim_params.lab_momentum},"
        + f"\"{gen_filepath}\", \"{workpathname}\" ,"
        + f"{sim_params.ip_offset_x}, {sim_params.ip_offset_y}, {sim_params.ip_offset_z}, "
        + f"{sim_params.ip_spread_x}, {sim_params.ip_spread_y}, {sim_params.ip_spread_z}, "
        + f"{sim_params.beam_tilt_x}, {sim_params.beam_tilt_y}, {sim_params.beam_divergence_x}, {sim_params.beam_divergence_y}, "
        # + f"\"{sim_params.lmd_geometry_filename}\", "
        # + f"\"{ali_params.misalignment_matrices_path}\", "
        # + f"{1 if ali_params.use_point_transform_misalignment else 0}, "
        + f"{verbositylvl})'"
    )
    if not debug:
        os.system(
            f"cp {workpathname}/Koala_MC_{start_evt}.root {path_mc_data}/Koala_MC_{start_evt}.root"
        )
        os.system(
            f"cp {workpathname}/Koala_Params_{start_evt}.root {path_mc_data}/Koala_Params_{start_evt}.root"
        )
else:
    if not debug:
        os.system(
            f"cp {path_mc_data}/Koala_MC_{start_evt}.root {workpathname}/Koala_MC_{start_evt}.root"
        )
        os.system(
            f"cp {path_mc_data}/Koala_Params_{start_evt}.root {workpathname}/Koala_Params_{start_evt}.root"
        )

# check_stage_success "workpathname + "/Koala_digi_{start_evt}.root""
if not check_stage_sucess() or force_level == 2:
    os.system(
        f"root -l -b -q 'KoaPixel1Digi.C({sim_params.num_events_per_sample}, {start_evt},"
        + f"\"{workpathname}\", "
        # + f"\"{ali_params.misalignment_matrices_path}\", "
        # + f"{1 if ali_params.use_point_transform_misalignment else 0}, "
        + f"{verbositylvl})'"
    )

os.chdir(scriptpath)
os.system(f"python runKoaReco.py")
