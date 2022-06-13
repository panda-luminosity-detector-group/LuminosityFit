# LuminosityFit

Note: For future development I would recommend to port the necessary code python and perform all needed operations in tensorflow, numpy and pandas. This will reduce the repo size and improve the UX and DX dramatically. Also when using tensorflow many features like running on gpus ship out of the box. Performance improvements can also be expected.

## Installation

### Prerequisites

Make sure your Pandaroot enviroment is set up correctly, more precisely that these environment variables are set:

- SIMPATH
- VMCWORKDIR
- FAIRROOTPATH
- ROOTSYS

Boost and the gsl library are two requirements, which are automatically included with fairsoft, so you already have them installed for sure. It is recommended to use the same boost, which was used to build the pandaroot enviroment. Use the _BOOST_ROOT_ environment variable to hint cmake to correct boost location.

```bash
export BOOST_ROOT=$SIMPATH
```

(No need to put that export in your bashrc, just run it in your shell before cmake call)

### Compilation

Simply create a build directory, change into that build directory, and run `cmake {PATH_TO_YOUR_LUMINOSITY_FIT_SOURCE}`

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

This is reduced overview. For simplicity, directly user-callable scripts are greenish, scripts (or binaries) that _can_ be run by a user but should not are reddish. Usually, the user-runnable scripts call these other scripts themselves:

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

## runSimulationReconstruction.py

Generates MC simulation data and performs the entire reconstruction chain. It needs a simulation parameters config file (simparams.conf) and a reconstruction parameters config file (recoparams.conf) to work. Examples of these files can be created with the script `create_sim_reco_pars.py`. Both are human-readable json files. The simulation is then:

```bash
./runSimulationReconstruction.py simparams.conf recoparams.conf
```

## determineLuminosity.py

Can only be run if `LumiTrkQA_` files for PandaRoot or `Koala_Track_` files for KoalaSoft are already present.

Minimum run example works without arguments, but searches a LOT of directories. It's generally better to at least limit the search paths:

```bash
./determineLuminosity.py --base_output_data_dir /path/to/TrksQAFiles.root
```

### Details

For details, please see the detailed readme in the `/python` folder.

The run sequence is as follows:

```mermaid
flowchart TD
determineLuminosity.py --> searchjDirs
id1([scenarioConfig.json]) -.-> determineLuminosity.py
```

### Function Call Diagram

The function diagram is pretty complicated, and has lot's of internal states. One very important class of objects is the `scenario` class, which holds a `state` variable. This variable controls the program flow. There can be multiple `scenario`s, each with their own `state`. A scenario object ist also passed to the `simulateDataOnHimster(scen: scenario)` function.

```mermaid
flowchart TD
start([Start]) --> setup[Parse arguments, check cluster,\n prepare jobManager, create Scenario stack]
setup --> callLumi["lumiDetermination(scenario)"]
callLumi --> state{Which State?}
state --1--> simOnHim["simulateDataOnHimster(scen: scenario)"]
simOnHim --> incrementState[increment state]
state --2--> recoIP["reconstruct IP position\nor use (0,0,0)"]
recoIP --> incrementState
incrementState ==> state
state --3--> filterDPM["Filter DPM data\nwith IP position"]
filterDPM --> simOnHim
```

---

The function `simulateDataOnHimster(scen: scenario)` needs it's own flowchart. It also has an internal `state` variable that determines its behavior.

```mermaid
flowchart TD
start([Start]) --> simOnHim["some call to simulateDataOnHimster"]
simOnHim --> prepare["output some debug data"]
prepare --> state{Which State?}
state --1--> simType{"Which\nsim_type?"}
simType --a--> simRecoDPM["start MC simulation\nand reconstruction\n(with IP position cut)"]
simType --er--> simRecoBOX["box gen for\nacc and res"]
simRecoDPM --> incrementState["increment state"]
simRecoBOX --> incrementState
incrementState ==> state
state --2--> makeMulti["makeMultipleFileListBunches.py"]
makeMulti --> simType2{"Which\nsim_type?"}
simType2 --a--> useCross["use cross section"]
useCross --> createMulti["createMultipleLmdData.py"]
simType2 --else--> createMulti
createMulti --> incrementState
state --3--> mergeData["mergeMultipleLmdData.py"]
mergeData --> incrementState
state ----4----> doMultiFit["doMultipleLuminosityFits.py"]
doMultiFit ==> done["All done, lumi data\nis saved in lmd_fitted_data.root"]
```

So basically, it does:

- simulation
- reconstruction (with IP position cut)
- box sim for acc and res data
- create multiple LMD data
- merge multiple LMD data

## doMultipleLuminosityFits.py

Needs a `--forced_box_gen_data` path and a path to the firconfig (has no parameter, just the path). So it's `python doMultipleLuminosityFits.py --forced_box_gen_data path fitconfig.json`. A full working example is here:

```bash
python doMultipleLuminosityFits.py --forced_box_gen_data /lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_1.5GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction /lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_1.5GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction xy_m_cut_real /home/roklasen/LuminosityFit/fitconfig-fast.json
```

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
