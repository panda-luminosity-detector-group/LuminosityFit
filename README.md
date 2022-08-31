# LuminosityFit

Note: For future development I would recommend to port the necessary code python and perform all needed operations in tensorflow, numpy and pandas. This will reduce the repo size and improve the UX and DX dramatically. Also when using tensorflow many features like running on gpus ship out of the box. Performance improvements can also be expected.

# Table of Contents

- [LuminosityFit](#luminosityfit)
- [Table of Contents](#table-of-contents)
  - [Installation](#installation)
    - [Prerequisites](#prerequisites)
    - [Generate Container](#generate-container)
    - [Compilation](#compilation)
    - [First Run](#first-run)
  - [Using](#using)
  - [Using in Container with the Slurm Agent](#using-in-container-with-the-slurm-agent)
- [Mode of Operation](#mode-of-operation)
- [Code Layout](#code-layout)
  - [Singularity Wrapper](#singularity-wrapper)
- [Scripts](#scripts)
- [Apps in `/bin/`](#apps-in-bin)
  - [createLmdFitData](#createlmdfitdata)
  - [createKoaFitData](#createkoafitdata)
  - [ExtractLuminosityValues](#extractluminosityvalues)

## Installation

### Prerequisites

Make sure your Pandaroot environment is set up correctly, more precisely that these environment variables are set:

- SIMPATH
- VMCWORKDIR
- FAIRROOTPATH
- ROOTSYS
- LMD_DATA_DIR

Boost and the gsl library are two requirements, which are automatically included with fairsoft, so you already have them installed for sure. It is recommended to use the same boost, which was used to build the pandaroot enviroment. Use the _BOOST_ROOT_ environment variable to hint cmake to correct boost location.

```bash
export BOOST_ROOT=$SIMPATH
```

(No need to put that export in your bashrc, just run it in your shell before cmake call)

### Generate Container

You have to generate the Luminosity Fit singularity container before the software can be compiled and run.

Please see [the docker/singularity documentation](docker/README.md).

Run the following steps inside the container.

### Compilation

Simply create a build directory, change into that build directory, and run `cmake {PATH_TO_YOUR_LUMINOSITY_FIT_SOURCE}`.

**IMPORTANT!** This software works with either PandaRoot or KoalaSoft _only_. Either of these packages must be loaded prior to compilation, otherwise project-specific binaries will not be built! Use this example for PandaRoot:

```bash
. ~/PandaRoot/build/config.sh
cd ~/LuminositySoft
mkdir build && cd build
cmake ../
make -j16
```

### First Run

Simulation jobs are executed inside a Singularity container. To keep the (runtime) environment clean, all environment variables are unset in the container and have to be explicitly set. This is done via a `lmdEndFile.env`, which depends on the cluster, LmdFit installation details and used Software (PandaRoot or KoalaSoft). It must be generated only once _inside_ the Lmdfit singularity container with:

```bash
python3 genEnvVarFile.py
```

It's best if all env variables are already set

## Using

The binaries in the `./bin` subdirectory of the build path can be used directly. For more convenient use, especially for larger datasamples sizes it is recommended to use the python scripts in the [./scripts](https://github.com/spflueger/LuminosityFit/tree/master/scripts) subdirectory. However, to use these scripts several environment variables have to be exported.

```bash
export LMDFIT_BUILD_PATH="path-to-your-luminosityfit-build-directory"   # e.g. $HOME/LuminosityFit/build
export LMDFIT_SCRIPTPATH="path-to-your-luminosityfit-script-directory"  # e.g. $HOME/LuminosityFit/python
export DATA_HOME="path-to-himspecf-data-storage"`
export LMDFIT_DATA_DIR=$DATA_HOME/paluma/"directory-name-of-your-choice"
```

In order to have the full ROOT cling support, export the build library directory location to the LD_LIBRARY_PATH.

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LMDFIT_BUILD_PATH/lib
```

## Using in Container with the Slurm Agent

Note: the agent currently needs Python 3.10 or higher. Start the agent with:

```bash
module load lang/Python/3.10.4-GCCcore-11.2.0
python/lumifit/agent.py
```

It will run in the background and wait for json-formatted `SlurmOrder`s in the named pipe `/tmp/lmdfitpipe`. After it receives an order and executes it, it returns a json-formatted `SlurmOrder` to the same pipe (and blocks if no-one receives it!).

To exit the agent, pipe the exit meta-command to the pipe:

```bash
echo '{"orderType": -1}' > $HOME/tmp/lmdfit
```

Start container (pipe in `/tmp` is automatically available in Singularity):

```bash
module load tools/Singularity
singularity run lmdfit-mini.sif
```

In there, run the test simulation:

```bash
python python runSimulationReconstruction.py simparams.conf recoparams.conf
```

# Mode of Operation

The luminosity extraction can only happen if one or multiple Lumi_TrksQA files are available (For now, a Lumi_Params file must also be present, I don't know yet what's in this file. It's created during MC generation step, but we don't have MC data in the actual experiment).

The LumiFit software then performs multiple individual steps, many of them on a cluster via slurm job.

1. Lumi_TrkQA files are already there
   - Determine IP Position from them
   - determine (or read?) total cross section and save to file
2. Now, two things happen at the same time (they are independent of each other):
   - With that IP position, apply xy and momentum cuts on tracks. That is a complete reco chain! This is done with DPM, just like the MC data gen.
     - copy Lumi_Params and Lumi_Digi from previous simulation
     - run reco, erecomerge, track find, track fit macros again
     - then, run trackFilter macro (this is the actual cut)
     - then, back propagation and Lumi_TrkQA again
   - The second thing is resolution and acceptance simulation, so have a realistic detector model
     - this is done with the BOX generator, not the DPM
     - but basically the same, complete MC data generation
     - and reconstruction WITH cuts (it's gotta be realistic)
     - which results in Lumi_TrksQA (but stored elsewhere, only relevant for resolution and acceptance!)
3. the script waits if all jobs are finished and merges the Lumi_TrkQA files from the Box gen sample, to merge the acc and res files. these are important for the LumiFit.

# Code Layout

This is reduced overview. For simplicity, directly user-callable scripts are teal, scripts (or binaries) that _can_ be run by a user but should not are reddish. Usually, the user-runnable scripts call these other scripts themselves:

```mermaid
graph LR
    id1[can be called by user, few parameters] --> id2[should not be called directly, lots of parameters]
    style id1 fill:#3caea3
    style id2 fill:#ed553b
```

There are only few convenience scripts and lots of "under the hood" scripts (and binaries, yellow). Rectangular nodes are scripts or programs, round nodes are data containers (like root files, json config files etc)

```mermaid
flowchart TD
    determineLuminosity.py:::teal --> runLmdReco.py:::red
    determineLuminosity.py --> runLmdSimReco.py:::red
    runSimulationReconstruction.py:::teal --> runLmdSimReco.py
    determineLuminosity.py --> LmdFitBinaries:::yellow
    runLmdSimReco.py --> runLmdReco.py

    runLmdSimReco.py --> simMacros[ROOT MC simulation macros]
    runLmdReco.py --> recoMacros[ROOT reconstruction macros]

    simMacros --> lumiMC{{Lumi MC, Lumi Digi}}
    recoMacros --> lumiQA{{Lumi_TrsQA}}
    lumiMC -.-> recoMacros
    lumiQA -.-> LmdFitBinaries
    LmdFitBinaries --> lumiValue([extrated luminosity]):::green

    %% --- color some nodes
    %% user callable

    classDef red fill:#ed553b
    classDef yellow fill:#f0a500
    classDef green fill:#125b50
    classDef teal fill:#3caea3

```

## Singularity Wrapper

Some script parts need to be submitted to SLURM. Because the entire application only works inside the Singularity container from now on, it must be called with a special wrapper script:

```mermaid
flowchart LR
id1[squeue COMMAND] --> id2[squeue ./singularityJob.sh COMMAND]
```

The script calls the container and sources the PandaRoot/KoalaSoft config.sh scripts necessary to set the needed env variables.

# Scripts

- [runSimulationReconstruction.py](docs/scripts/runSimulationRecoinstruction.md)
- [determineLuminosity.py](docs/scripts/determineLuminosity.md)
- [doMultipleLuminosityFits.py](docs/scripts/doMultipleLuminosityFits.md)

# Apps in `/bin/`

## createLmdFitData

## createKoaFitData

Same but for the Koala Experiment

## ExtractLuminosityValues

Is compiled to `extractLuminosity`. Needs only a path to the merge data, and extracts the fitted lumi from the `lmd_fitted_data.root`.

```bash
./extractLuminosity /lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_1.5GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/merge_data
```

Note: currently this doesn't seem to work because the `EstimatorOptions` aren't saved to the root file, this is a new problem.
