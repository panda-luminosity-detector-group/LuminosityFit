A collection of (mostly) Python3 scripts to facilitate the use of the LuminosityFit software.

Note that the whole workflow is far from trivial, and there are many details that are not mentioned in this Readme.

The LuminosityFit software requires a reconstructed elastic scattering events from the Panda Luminosity Detector (LMD).

As long as the [PANDA experiment](https://panda.gsi.de/) is not running these events have to be simulated using the PandaRoot framework.

Simulations and reconstruction can be easily performed using the python scripts `doSimulationReconstruction.py` or `doReconstruction.py`. Use the `--help` flag to get more detailed information on how to use these scripts. These scripts create so call configuration files `sim_params.config` and `reco_params.config`, that contain all of the information to perform a simulation or reconstruction (as you specified them).

The are useful for bookkeeping and can also be used to redo the simulation or reconstruction using the scripts `runSimulationReconstruction.py` and `runReconstruction.py`.

Note that the simulation step also can create an `elastic_cross_section.txt` file when the dpm mode is selected. It contains the cross section of the elastic scattering process, corresponding to the generated events. From this the reference luminosity can be calculated, which is useful for calculating the accuracy of the luminosity determination procedure.

Once you have an unfiltered (do **NOT** use any cut flags in the reconstruction, such as `--use_xy_cut` or `--use_m_cut`) dataset of reconstructed elastic scattering events, the luminosity determination can be started.

Use `determineLuminosity.py`, which is quite complicated and *fragile*, and only runs on the Himster2 cluster in Mainz. The reason for this is that the whole luminosity determination process is very difficult. The excellent accuracy in the below percent level can only be reached with big efforts. Currently the efficiency and resolution corrections are performed via additional simulations and reconstruction.

The overall steps in the luminosity determination process are:

1. create interaction point (IP) distribution from the unfiltered reconstructed elastic scattering tracks
2. determine the means (X,Y,Z) of the IP distribution
3. filter the elastic scattering dataset 
   **AND** independently do a simulation and reconstruction using the determined IP info and a box generator
4. create a 2D angular track distribution of the filtered elastic scattering dataset 
   **AND** independently create an efficiency and a resolution object from the box generated dataset
5. perform the actual luminosity fit using the 2D angular track distribution and the efficiency and resolution objects

These step exclude a possible alignment process which should run before all of this. 
