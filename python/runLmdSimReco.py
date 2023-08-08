#!/usr/bin/env python3

import os

from lumifit.config import load_params_from_file
from lumifit.general import (
    check_stage_success,
    envPath,
    matrixMacroFileName,
    toCbool,
)
from lumifit.types import (
    AlignmentParameters,
    ExperimentParameters,
    SimulationGeneratorType,
    SimulationParameters,
)

# TODO: get all this from the experiment config
# lmd_build_path = os.environ["LMDFIT_BUILD_PATH"]
# PNDmacropath = os.environ["LMDFIT_MACROPATH"]
# LMDscriptpath = os.environ["LMDFIT_SCRIPTPATH"]
# pathToTrkQAFiles = os.environ["pathname"]
# relativeDirToTrksQAFiles = os.environ["dirname"]
# path_mc_data = os.environ["path_mc_data"]


force_level = int(os.environ["force_level"])

# this is all that is needed because the experiment config is there.
BaseDir = envPath("BaseDir")
experiment = load_params_from_file(BaseDir / "experiment.config", ExperimentParameters)
simParams = experiment.simParams
alignParams = experiment.alignParams

# are we in the "normal" simulation mode or in the "res/acc" mode?
# TODO: get from the paths module

resAccMode = simParams.simGeneratorType == SimulationGeneratorType.RESACCBOX or simParams.simGeneratorType == SimulationGeneratorType.RESACCPBARP_ELASTIC
if resAccMode:
    path_mc_data = experiment.softwarePaths.ResAccMCDataDir
    pathToTrkQAFiles = experiment.softwarePaths.simData
else:
    path_mc_data = experiment.softwarePaths.SimulationMCDataDir
    pathToTrkQAFiles = experiment.softwarePaths.resAccData

# prepare directories
path_mc_data.mkdir(parents=True, exist_ok=True)

filename_index = 1
debug = True
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

# workpathname is the temporary path on the compute node where Lumi_{MC,Digi,Reco...} files are stored.
# it is deleted after the job is finished
if debug:
    workpathname = pathToTrkQAFiles
    path_mc_data = workpathname
else:
    workpathname = f"/localscratch/{os.environ['SLURM_JOB_ID']}/{relativeDirToTrksQAFiles}"

if not os.path.isdir(workpathname):
    os.makedirs(workpathname)

gen_filepath = workpathname + "/gen_mc.root"

verbositylvl = 0
numTrks = 1  # should not be changed
start_evt: int = simParams.num_events_per_sample * filename_index


print(f"\n\nINFO:\ndirname is {relativeDirToTrksQAFiles}\npathname is {pathToTrkQAFiles}\nworkpathname is {workpathname}\n\n")

# * ------------------- MC Data Step -------------------
if not check_stage_success(f"{path_mc_data}/Lumi_MC_{start_evt}.root") or force_level == 2:
    # * prepare box or dpm tracks
    if simParams.simGeneratorType == SimulationGeneratorType.BOX or simParams.simGeneratorType == SimulationGeneratorType.RESACCBOX:
        os.chdir(LMDscriptpath)

        cmd = f"""root -l -b -q 'standaloneBoxGen.C({simParams.lab_momentum}, {simParams.num_events_per_sample}, {simParams.theta_min_in_mrad}, {simParams.theta_max_in_mrad}, {simParams.phi_min_in_rad}, {simParams.phi_max_in_rad},"{gen_filepath}", {simParams.random_seed + start_evt}, {toCbool(not simParams.neglect_recoil_momentum)})'"""

        print(f"\ncalling stand alone box gen with:\n")
        print(cmd)
        os.system(cmd)

    elif simParams.simGeneratorType == SimulationGeneratorType.PBARP_ELASTIC or simParams.simGeneratorType == SimulationGeneratorType.RESACCPBARP_ELASTIC:
        # cmd = f"{lmd_build_path}/bin/generatePbarPElasticScattering {simParams.lab_momentum} {simParams.num_events_per_sample} -l {simParams.theta_min_in_mrad} -u {simParams.theta_max_in_mrad} -s {simParams.random_seed + start_evt} -o {gen_filepath}"

        # print(f"\ncalling elastic P Pbar generator with:\n")
        # print(cmd)

        # os.system(cmd)

        os.system(
            f"{lmd_build_path}/bin/generatePbarPElasticScattering"
            + f" {simParams.lab_momentum} {simParams.num_events_per_sample}"
            + f" -l {simParams.theta_min_in_mrad}"
            + f" -u {simParams.theta_max_in_mrad}"
            + f" -n {simParams.phi_min_in_rad}"
            + f" -g {simParams.phi_max_in_rad}"
            + f" -s {simParams.random_seed + start_evt}"
            + f" -o {gen_filepath}"
        )

    # * run simBox or simDPM
    print("starting up a pandaroot simulation...")
    os.chdir(PNDmacropath)
    # sim noise is the same with and without rest gas
    if simParams.simGeneratorType == SimulationGeneratorType.NOISE:
        os.system(
            f"""root -l -b -q 'runLumiPixel0SimBox.C({simParams.num_events_per_sample}, {start_evt}, "{workpathname}",{verbositylvl},-2212,{simParams.lab_momentum},{numTrks},{simParams.random_seed + start_evt}, 0, , "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)})' > /dev/null 2>&1"""
        )
    # later steps differ if we have rest gas
    else:
        if simParams.useRestGas:
            if simParams.simGeneratorType == SimulationGeneratorType.PBARP_ELASTIC or simParams.simGeneratorType == SimulationGeneratorType.RESACCPBARP_ELASTIC:
                os.system(
                    f"""root -l -b -q 'runLumiPixel0SimDPMRestGas.C({simParams.num_events_per_sample}, {start_evt}, {simParams.lab_momentum}, "{gen_filepath}", "{workpathname}", {simParams.ip_offset_x}, {simParams.ip_offset_y}, {simParams.ip_offset_z}, {simParams.ip_spread_x}, {simParams.ip_spread_y}, {simParams.ip_spread_z}, {simParams.beam_tilt_x}, {simParams.beam_tilt_y}, {simParams.beam_divergence_x}, {simParams.beam_divergence_y}, "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
                )
            elif simParams.simGeneratorType == SimulationGeneratorType.BOX or simParams.simGeneratorType == SimulationGeneratorType.RESACCBOX:
                os.system(
                    f"""root -l -b -q 'runLumiPixel0SimBoxRestGas.C({simParams.num_events_per_sample}, {start_evt}, "{workpathname}",{verbositylvl},-2212,{simParams.lab_momentum},{simParams.ip_offset_x}, {simParams.ip_offset_y}, {simParams.ip_offset_z}, {simParams.ip_spread_x}, {simParams.ip_spread_y}, {simParams.ip_spread_z}, {simParams.beam_tilt_x}, {simParams.beam_tilt_y}, {simParams.beam_divergence_x}, {simParams.beam_divergence_y}, {numTrks},{simParams.random_seed + start_evt}, 0, "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)})'"""
                )
        else:
            os.system(
                f"""root -l -b -q 'runLumiPixel0SimDPM.C({simParams.num_events_per_sample}, {start_evt}, {simParams.lab_momentum}, "{gen_filepath}", "{workpathname}", {simParams.ip_offset_x}, {simParams.ip_offset_y}, {simParams.ip_offset_z}, {simParams.ip_spread_x}, {simParams.ip_spread_y}, {simParams.ip_spread_z}, {simParams.beam_tilt_x}, {simParams.beam_tilt_y}, {simParams.beam_divergence_x}, {simParams.beam_divergence_y}, "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
            )


# if first stage was successful, copy MC data directly to compute node and don't generate new
else:
    if not debug:
        os.system(f"cp {path_mc_data}/Lumi_MC_{start_evt}.root {workpathname}/Lumi_MC_{start_evt}.root")
        os.system(f"cp {path_mc_data}/Lumi_Params_{start_evt}.root {workpathname}/Lumi_Params_{start_evt}.root")

# * ------------------- Digi Step -------------------
if not check_stage_success(workpathname + f"/Lumi_digi_{start_evt}.root") or force_level == 2:
    os.chdir(PNDmacropath)
    if simParams.simGeneratorType == SimulationGeneratorType.NOISE:
        os.system(
            f"""root -l -b -q 'runLumiPixel1bDigiNoise.C({simParams.num_events_per_sample}, {start_evt}, "{workpathname}", {verbositylvl}, {simParams.random_seed + start_evt})'"""
        )
    else:
        os.system(
            f"""root -l -b -q 'runLumiPixel1Digi.C({simParams.num_events_per_sample}, {start_evt}, "{workpathname}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
        )


# always copy mc data and params from node to permanent storage (params are needed for all subsequent steps. Also: Params are UPDATED every step, so only the final Params file holds all needed data)
if not debug:
    os.system(f"cp {workpathname}/Lumi_MC_{start_evt}.root {path_mc_data}/Lumi_MC_{start_evt}.root")
    os.system(f"cp {workpathname}/Lumi_Params_{start_evt}.root {path_mc_data}/Lumi_Params_{start_evt}.root")
    # copy the Lumi_Digi data to permanent storage, it's needed for IP cut for the LumiFit
    # MC path is better for this since digi data is "almost real data"
    os.system(f"cp {workpathname}/Lumi_digi_{start_evt}.root {path_mc_data}/Lumi_digi_{start_evt}.root")

os.chdir(LMDscriptpath)
os.system("./runLmdReco.py")
