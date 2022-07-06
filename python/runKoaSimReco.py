#!/usr/bin/env python3

import os

from lumifit.alignment import AlignmentParameters
from lumifit.general import load_params_from_file, check_stage_success
from lumifit.simulation import SimulationParameters, SimulationType
from lumifit.general import toCbool

lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]
scriptpath = os.environ["LMDFIT_SCRIPTPATH"]
macropath = os.environ["LMDFIT_MACROPATH"]

dirname = os.environ["dirname"]
path_mc_data = os.environ["path_mc_data"]
pathname = os.environ["pathname"]
force_level = int(os.environ["force_level"])


filename_index = 1
debug = True
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

# TODO: check if params are loaded correctly, shouldn't be the specified file name be used?
sim_params = SimulationParameters(
    **load_params_from_file(path_mc_data + "/../sim_params.config")
)

ali_params = AlignmentParameters()

if debug:
    workpathname = pathname
    path_mc_data = workpathname
else:
    workpathname = f"/localscratch/{os.environ['SLURM_JOB_ID']}/{dirname}"

if not os.path.exists(workpathname):
    os.makedirs(workpathname)

gen_filepath = workpathname + "/gen_mc.root"

verbositylvl = 0
start_evt: int = sim_params.num_events_per_sample * filename_index

if (
    not check_stage_success(f"{path_mc_data} + /Koala_MC_{start_evt}.root")
    or force_level == 2
):
    os.chdir(scriptpath)
    if sim_params.sim_type == SimulationType.BOX:
        os.system(
            f"{lmd_build_path}/bin/generatePbarPElasticScattering"
            + f" {sim_params.lab_momentum} {sim_params.num_events_per_sample}"
            + f" -l {sim_params.theta_min_in_mrad}"
            + f" -u {sim_params.theta_max_in_mrad}"
            + f" -n {sim_params.phi_min_in_rad}"
            + f" -g {sim_params.phi_max_in_rad} -s {sim_params.random_seed + start_evt}"
            + f" -o {gen_filepath}"
        )

    elif sim_params.sim_type == SimulationType.PBARP_ELASTIC:
        os.system(
            f"{lmd_build_path}/bin/generatePbarPElasticScattering"
            + f" {sim_params.lab_momentum} {sim_params.num_events_per_sample}"
            + f" -l {sim_params.theta_min_in_mrad}"
            + f" -u {sim_params.theta_max_in_mrad}"
            + f" -n {sim_params.phi_min_in_rad}"
            + f" -g {sim_params.phi_max_in_rad} -s {sim_params.random_seed + start_evt}"
            + f" -o {gen_filepath}"
        )

    os.chdir(macropath)
    print("starting up a koalasoft simulation...")
    os.system(
        f"root -l -b -q 'KoaPixel0SimExtern.C({sim_params.num_events_per_sample}, {start_evt}, {sim_params.lab_momentum},"
        + f'"{gen_filepath}", "{workpathname}" ,'
        + f"{sim_params.ip_offset_x}, {sim_params.ip_offset_y}, {sim_params.ip_offset_z}, "
        + f"{sim_params.ip_spread_x}, {sim_params.ip_spread_y}, {sim_params.ip_spread_z}, "
        + f"{sim_params.beam_tilt_x}, {sim_params.beam_tilt_y}, {sim_params.beam_divergence_x}, {sim_params.beam_divergence_y}, "
        # + f"\"{sim_params.lmd_geometry_filename}\", "
        # + f"\"{ali_params.misalignment_matrices_path}\", "
        # + f"{1 if ali_params.use_point_transform_misalignment else 0}, "
        + f"{verbositylvl})'"
    )
else:
    if not debug:
        os.system(
            f"cp {path_mc_data}/Koala_MC_{start_evt}.root {workpathname}/Koala_MC_{start_evt}.root"
        )
        os.system(
            f"cp {path_mc_data}/Koala_Params_{start_evt}.root {workpathname}/Koala_Params_{start_evt}.root"
        )

if (
    not check_stage_success(workpathname + f"/Koala_digi_{start_evt}.root")
    or force_level == 2
):
    os.chdir(macropath)
    os.system(
        f"root -l -b -q 'KoaPixel1Digi.C({sim_params.num_events_per_sample}, {start_evt},"
        + f'"{workpathname}", '
        # + f"\"{ali_params.misalignment_matrices_path}\", "
        # + f"{1 if ali_params.use_point_transform_misalignment else 0}, "
        + f"{verbositylvl})'"
    )
if not debug:
    os.system(
        f"cp {workpathname}/Koala_MC_{start_evt}.root {path_mc_data}/Koala_MC_{start_evt}.root"
    )
    os.system(
        f"cp {workpathname}/Koala_digi_{start_evt}.root {path_mc_data}/Koala_digi_{start_evt}.root"
    )

    os.system(
        f"cp {workpathname}/Koala_Params_{start_evt}.root {path_mc_data}/Koala_Params_{start_evt}.root"
    )
else:
    if not debug:
        os.system(
            f"cp {path_mc_data}/Koala_digi_{start_evt}.root {workpathname}/Koala_digi_{start_evt}.root"
        )
        os.system(
            f"cp {path_mc_data}/Koala_Params_{start_evt}.root {workpathname}/Koala_Params_{start_evt}.root"
        )

os.chdir(scriptpath)
os.system("./runKoaReco.py")
