#!/usr/bin/env python3

"""
This script is only run by a user, not any other script!
"""

import argparse
import json
import shutil
from pathlib import Path

from lumifit.cluster import ClusterJobManager
from concurrent.futures import ThreadPoolExecutor
from lumifit.config import load_params_from_file, write_params_to_file
from lumifit.general import addDebugArgumentsToParser
from lumifit.gsi_virgo import create_virgo_job_handler
from lumifit.paths import generateAbsoluteROOTDataPath
from lumifit.himster import create_himster_job_handler
from lumifit.types import ClusterEnvironment, DataMode, ExperimentParameters
from typing import List
from wrappers.createSimRecoJob import create_simulation_and_reconstruction_job


def run_simulation_and_reconstruction(thisExperiment: ExperimentParameters) -> bool:
    assert thisExperiment.dataPackage.simParams is not None
    assert thisExperiment.dataPackage.MCDataDir is not None

    thisExperiment.experimentDir.mkdir(parents=True, exist_ok=True)
    thisExperiment.dataPackage.MCDataDir.mkdir(parents=True, exist_ok=True)
    # thisExperiment.dataPackage.baseDataDir.mkdir(parents=True, exist_ok=True)

    # before anything else, check if config is internally consistent
    if not thisExperiment.isConsistent():
        print("Error! Experiment config is inconsistent!")
        return

    # overwrite existing config file, otherwise old settings from the last run could remain
    write_params_to_file(thisExperiment, thisExperiment.experimentDir, "experiment.config", overwrite=True)

    # if no alignment matrices are found, create an empty json file so that ROOT doesn't crash
    # the file must be overwritten by the user once alignment matrices are available
    # but even when it is empty, we should still get a working (albeit wrong) lumifit
    if thisExperiment.dataPackage.alignParams.alignment_matrices_path is not None:
        alignmentMatrixPath = thisExperiment.dataPackage.alignParams.alignment_matrices_path

        # make parent path if it doesn't exist
        alignmentMatrixPath.parent.mkdir(parents=True, exist_ok=True)

        if not alignmentMatrixPath.exists():
            print(f"INFO: Alignment matrix file {alignmentMatrixPath} does not exist! Creating empty file...")
            with open(alignmentMatrixPath, "w") as f:
                json.dump({}, f, ensure_ascii=True)

    # lastly, if this scipt was called specifically for alignment, delete the reco_uncut dir
    # and disable cuts
    if args.alignment:
        print("INFO: Running sim and reco for alignment, deleting reco_uncut dir and disabling cuts...")
        recoDirUncut = generateAbsoluteROOTDataPath(thisExperiment.dataPackage, DataMode.VERTEXDATA)
        if recoDirUncut.exists():
            shutil.rmtree(recoDirUncut)
        # cuts are disabled by runLmdSimReco.py when the data mode is set to VERTEXDATA, so nothing to do here


    job = create_simulation_and_reconstruction_job(
        thisExperiment,
        thisMode=DataMode.VERTEXDATA,
        force_level=args.force_level,
        debug=args.debug,
        use_devel_queue=args.use_devel_queue,
    )

    jobID = job_manager.enqueue(job)
    print (f"INFO: Enqueued job with ID {jobID}.")
    return True

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Script to run a simulation and reconstruction of the PANDA Luminosity Detector from parameter config files.",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    configGroup = parser.add_mutually_exclusive_group(required=True)

    configGroup.add_argument(
        "-e",
        "--experiment_config",
        dest="ExperimentConfigFile",
        type=Path,
        help="The Experiment.config file that holds all info.",
        default=None,
    )

    configGroup.add_argument(
        "-E",
        "--experiment_dir",
        dest="ExperimentDir",
        type=Path,
        help="Process all configs in this directory. If not set, at least one config must be specified with -e.",
        default=None,
    )

    parser.add_argument(
        "-a",
        "--alignment",
        dest="alignment",
        action="store_true",
        help="Run the simulation and reconstruction so that alignment can be performed. This disables cuts and deletes any existing reco_uncut dirs.",
    )

    parser = addDebugArgumentsToParser(parser)
    args = parser.parse_args()

    experiments: List[ExperimentParameters] = []

    # either we have one experiment config file or a directory with many
    if args.ExperimentDir is not None:
        if args.ExperimentDir.is_file():
            raise ValueError("ERROR! ExperimentDir must be a directory!")

        # process all experiment configs in the given dir
        for experimentConfig in args.ExperimentDir.glob("*.config"):
            experiment: ExperimentParameters = load_params_from_file(experimentConfig, ExperimentParameters)
            experiments.append(experiment)

            # this is a hack to make the code work with the old config files
    elif args.ExperimentConfigFile is not None:
        if args.ExperimentConfigFile.is_dir():
            raise ValueError("ERROR! ExperimentConfigFile must be a file!")

        # load experiment config
        experiment = load_params_from_file(args.ExperimentConfigFile, ExperimentParameters)
        experiments.append(experiment)

    # ask confirmation
    print("The following experiments will be processed:")
    for experiment in experiments:
        print(f"  {experiment.experimentDir} ({experiment.dataPackage.recoParams.lab_momentum} GeV)")
    print("")
    print("Continue? [y/n]")
    user_input = input()
    if user_input != "y":
        print("Aborting!")
        exit(0)

    # check which cluster we're on and create job handler
    # we know there is at least one config, and we just assume all use the same cluster
    if experiments[0].cluster == ClusterEnvironment.VIRGO:
        job_handler = create_virgo_job_handler("long")
    elif experiments[0].cluster == ClusterEnvironment.HIMSTER:
        if args.use_devel_queue:
            job_handler = create_himster_job_handler("devel")
        else:
            job_handler = create_himster_job_handler("himster2_exp")
    else:
        raise NotImplementedError("This cluster type is not implemented!")

    # job threshold of this type (too many jobs could generate to much io load
    # as quite a lot of data is read in from the storage...)
    job_manager = ClusterJobManager(job_handler, 2000, 3600)

    if args.debug:
        print("DEBUG: running experiment sequentially without thread pool.")
        for experiment in experiments:
            run_simulation_and_reconstruction(experiment)

    else:
        with ThreadPoolExecutor() as recipeExecutor:
            futures = [recipeExecutor.submit(run_simulation_and_reconstruction, experiment) for experiment in experiments]

            # wait for all tasks to finish
            print("Enqueued all experiments, waiting...")
            recipeExecutor.shutdown(wait=True)

            for future in futures:
                if future.result() is False:
                    raise RuntimeError("ERROR! A recipe crashed!")
