#!/usr/bin/env python3

import os
from pathlib import Path

from attrs import evolve
from lumifit.config import load_params_from_file
from lumifit.general import (
    envPath,
    isFilePresentAndValid,
    matrixMacroFileName,
    toCbool,
)
from lumifit.paths import generateAbsoluteROOTDataPath
from lumifit.types import (
    DataMode,
    ExperimentParameters,
    SimulationGeneratorType,
)

# * ------------------- Experiment Parameters -------------------
experimentDir = envPath("ExperimentDir")
thisMode = os.environ["DataMode"]
force_level = int(os.environ["force_level"])

experiment: ExperimentParameters = load_params_from_file(experimentDir / "experiment.config", ExperimentParameters)

if thisMode == DataMode.DATA.value:
    configPackage = experiment.dataPackage
elif thisMode == DataMode.VERTEXDATA.value:
    configPackage = experiment.dataPackage
    if configPackage.recoParams.use_xy_cut or configPackage.recoParams.use_m_cut:
        print("----------------------------------------------------------------------------------------")
        print("Attention! This experiment configs specifies to use XY and m cuts during reconstruction.")
        print("That's reasonable for the luminosity determination, but the initial data sample must    ")
        print("still be generated without cuts first.                                                  ")
        print("                                                                                        ")
        print("                            Disabling all cuts for this run!                            ")
        print("----------------------------------------------------------------------------------------")

        # remember, all params are immutable, so we need to copy them
        tempRecoParams = evolve(configPackage.recoParams, use_xy_cut=False, use_m_cut=False)
        configPackage = evolve(configPackage, recoParams=tempRecoParams)

elif thisMode == DataMode.RESACC.value:
    configPackage = experiment.resAccPackage
else:
    raise NotImplementedError("DataMode not implemented")

assert configPackage.simParams is not None
assert configPackage.MCDataDir is not None

MCDataDir = configPackage.MCDataDir
simParams = configPackage.simParams
alignParams = configPackage.alignParams
pathToTrkQAFiles = generateAbsoluteROOTDataPath(configPackage=configPackage)
lmd_build_path = experiment.softwarePaths.LmdFitBinaries
PNDmacropath = experiment.softwarePaths.PandaRootMacroPath
LMDscriptpath = experiment.softwarePaths.LmdFitScripts
relativeDirToTrksQAFilesOnComputeNode = "LMD-TempRootFiles"


# prepare directories
MCDataDir.mkdir(parents=True, exist_ok=True)
pathToTrkQAFiles.mkdir(parents=True, exist_ok=True)

filename_index = 1
debug = True
if "SLURM_ARRAY_TASK_ID" in os.environ:
    filename_index = int(os.environ["SLURM_ARRAY_TASK_ID"])
    debug = False

start_evt = simParams.num_events_per_sample * filename_index

# workingDirOnComputeNode is the temporary path on the compute node where Lumi_{MC,Digi,Reco...} files are stored.
# it is deleted after the job is finished
if debug:
    workingDirOnComputeNode = pathToTrkQAFiles
    MCDataDir = workingDirOnComputeNode
else:
    workingDirOnComputeNode = Path(f"/localscratch/{os.environ['SLURM_JOB_ID']}/{relativeDirToTrksQAFilesOnComputeNode}")

workingDirOnComputeNode.mkdir(parents=True, exist_ok=True)

gen_filepath = workingDirOnComputeNode / "gen_mc.root"

verbositylvl = 0
numTrks = 1  # should not be changed


# * ------------------- MC Data Step -------------------
if not isFilePresentAndValid(Path(f"{MCDataDir}/Lumi_MC_{start_evt}.root")) or force_level == 2:
    # * prepare box or dpm tracks
    if simParams.simGeneratorType == SimulationGeneratorType.BOX:
        os.chdir(LMDscriptpath)

        cmd = f"""root -l -b -q 'standaloneBoxGen.C({simParams.lab_momentum}, {simParams.num_events_per_sample}, {simParams.theta_min_in_mrad}, {simParams.theta_max_in_mrad}, {simParams.phi_min_in_rad}, {simParams.phi_max_in_rad},"{gen_filepath}", {simParams.random_seed + start_evt}, {toCbool(not simParams.neglect_recoil_momentum)})'"""

        print(f"\ncalling stand alone box gen with:\n")
        print(cmd)
        os.system(cmd)

    elif simParams.simGeneratorType == SimulationGeneratorType.PBARP_ELASTIC:
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
            f"""root -l -b -q 'runLumiPixel0SimBox.C({simParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}",{verbositylvl},-2212,{simParams.lab_momentum},{numTrks},{simParams.random_seed + start_evt}, 0, , "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)})' > /dev/null 2>&1"""
        )
    # later steps differ if we have rest gas
    else:
        if simParams.useRestGas:
            if simParams.simGeneratorType == SimulationGeneratorType.PBARP_ELASTIC:
                os.system(
                    f"""root -l -b -q 'runLumiPixel0SimDPMRestGas.C({simParams.num_events_per_sample}, {start_evt}, {simParams.lab_momentum}, "{gen_filepath}", "{workingDirOnComputeNode}", {simParams.ip_offset_x}, {simParams.ip_offset_y}, {simParams.ip_offset_z}, {simParams.ip_spread_x}, {simParams.ip_spread_y}, {simParams.ip_spread_z}, {simParams.beam_tilt_x}, {simParams.beam_tilt_y}, {simParams.beam_divergence_x}, {simParams.beam_divergence_y}, "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
                )
            elif simParams.simGeneratorType == SimulationGeneratorType.BOX:
                os.system(
                    f"""root -l -b -q 'runLumiPixel0SimBoxRestGas.C({simParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}",{verbositylvl},-2212,{simParams.lab_momentum},{simParams.ip_offset_x}, {simParams.ip_offset_y}, {simParams.ip_offset_z}, {simParams.ip_spread_x}, {simParams.ip_spread_y}, {simParams.ip_spread_z}, {simParams.beam_tilt_x}, {simParams.beam_tilt_y}, {simParams.beam_divergence_x}, {simParams.beam_divergence_y}, {numTrks},{simParams.random_seed + start_evt}, 0, "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)})'"""
                )
        else:
            os.system(
                f"""root -l -b -q 'runLumiPixel0SimDPM.C({simParams.num_events_per_sample}, {start_evt}, {simParams.lab_momentum}, "{gen_filepath}", "{workingDirOnComputeNode}", {simParams.ip_offset_x}, {simParams.ip_offset_y}, {simParams.ip_offset_z}, {simParams.ip_spread_x}, {simParams.ip_spread_y}, {simParams.ip_spread_z}, {simParams.beam_tilt_x}, {simParams.beam_tilt_y}, {simParams.beam_divergence_x}, {simParams.beam_divergence_y}, "{simParams.lmd_geometry_filename}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
            )


# if first stage was successful, copy MC data directly to compute node and don't generate new
else:
    if not debug:
        os.system(f"cp {MCDataDir}/Lumi_MC_{start_evt}.root {workingDirOnComputeNode}/Lumi_MC_{start_evt}.root")
        os.system(f"cp {MCDataDir}/Lumi_Params_{start_evt}.root {workingDirOnComputeNode}/Lumi_Params_{start_evt}.root")

# * ------------------- Digi Step -------------------
if not isFilePresentAndValid(workingDirOnComputeNode / f"Lumi_digi_{start_evt}.root") or force_level == 2:
    os.chdir(PNDmacropath)
    if simParams.simGeneratorType == SimulationGeneratorType.NOISE:
        os.system(
            f"""root -l -b -q 'runLumiPixel1bDigiNoise.C({simParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", {verbositylvl}, {simParams.random_seed + start_evt})'"""
        )
    else:
        os.system(
            f"""root -l -b -q 'runLumiPixel1Digi.C({simParams.num_events_per_sample}, {start_evt}, "{workingDirOnComputeNode}", "{matrixMacroFileName(alignParams.misalignment_matrices_path)}", {toCbool(alignParams.use_point_transform_misalignment)}, {verbositylvl})'"""
        )


# always copy mc data and params from node to permanent storage (params are needed for all subsequent steps. Also: Params are UPDATED every step, so only the final Params file holds all needed data)
if not debug:
    os.system(f"cp {workingDirOnComputeNode}/Lumi_MC_{start_evt}.root {MCDataDir}/Lumi_MC_{start_evt}.root")
    os.system(f"cp {workingDirOnComputeNode}/Lumi_Params_{start_evt}.root {MCDataDir}/Lumi_Params_{start_evt}.root")
    # copy the Lumi_Digi data to permanent storage, it's needed for IP cut for the LumiFit
    # MC path is better for this since digi data is "almost real data"
    os.system(f"cp {workingDirOnComputeNode}/Lumi_digi_{start_evt}.root {MCDataDir}/Lumi_digi_{start_evt}.root")

os.chdir(LMDscriptpath)
os.system("./runLmdReco.py")
