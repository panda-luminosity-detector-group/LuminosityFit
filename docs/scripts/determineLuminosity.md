# The `determineLuminosity.py` Script

Can only be run if `LumiTrkQA_` files for PandaRoot or `Koala_Track_` files for KoalaSoft are already present.

Minimum run example works without arguments, but searches a LOT of directories. It's generally better to at least limit the search paths:

```bash
./determineLuminosity.py -e /path/to/experiment.config
```

# TL;DR

There are two main large functions:

- runSimOnHimster
- determineLuminosity

They pass a scenario object between them back and forth while updating it's internal parameters. So the lumi determination process itself is rather straight forward. But the first function has to submit jobs to SLURM, and so it can't block but the process can also not proceed until the jobs are done.

So the call is something like:

1: run determineLuminosity once
2: that one calls runSimOnHimster sometimes
3: wait and go to 1

The scenario object has an internal state variable that's changed by the function. It describes what should currently be done with the scenario object. The fact that this variable changes at runtime by two functions in an endless loop makes this script rather difficult to understand.

At least the states have descriptive names 🤷

# Run Sequence

Luminosity Determination involves a lot of steps. Each of them is detailled in this section:

- Dertermine interaction point from reconstructed tracks
- Do Data Reconstruction again (with IP position cut)
- Perform box sim for acceptance and resolution data
- Create multiple LMD data
- Merge multiple LMD data
- Perform Luminosity Fit on Merged Data

## Diagram

The run sequence is as follows:

```mermaid
flowchart TD
determineLuminosity.py --> searchjDirs
id1([scenarioConfig.json]) -.-> determineLuminosity.py
```

# Functions

- `lumiDetermination()`
- `simulateDataOnHimster(scen: scenario)`

## Call Diagram

The function diagram is pretty complicated, and has lot's of internal states. One very important class of objects is the `scenario` class, which holds a `state` variable (and a list with more states and working directories). This class is in `scenario.. These variables controls the program flow. There can be multiple `scenario`s, each with their own `state`. A scenario object ist also passed to the `simulateDataOnHimster(scen: scenario)` function.

```mermaid
flowchart TD
start([lumiDetermination]) --> append["append to wait stack\n(changed internal state\)"]
append --> wstack
wstack["while(waiting stack > 0)"] ---> lumiDetermination --> finished["finished?"]
finished --no --> append
finished --yes--> done(["Done!"])
```

## `lumiDetermination()`

The `lumiDetermination` function is called once at the beginning, and then sometimes again if the waiting stack is not empty.

```mermaid
flowchart TD
start([lumiDetermination]) --> setup[Parse arguments, check cluster,\n prepare jobManager, create Scenario stack]
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

## `simulateDataOnHimster(scen: scenario)`

It also has an internal `state` variable that determines its behavior.

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
