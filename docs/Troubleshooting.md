# Troubleshooting

In case the luminosity fit doesn't work or the results are unexpected, refer to this guide.

# Fit Won't Start / Software Issues

TBD. Most software issues must be solved with tedious debugging I'm afraid.

# Fit Result is unexpected / Luminosity is too large or too small

First, we should know what values are plausible for the fitted luminosity.

## Plausible Luminosity Values

The integrated luminosity depends on the cross section of the interaction and the total number of simulated events:

$L = \frac{1}{\sigma_{\text{tot}}} \cdot N_{\text{events}}$

If the data sample is generated with the Luminosity Fit Framework (`./runSimulationReconstruction.py`), it will generate a total cross section file `elastic_cross_section.txt` the base data dir (e.g. `${LMDFIT_DATA_DIR}/plab_1.5GeV/dpm_elastic_[...]/ip_offset_[...]/beam_grad_[...]/no_geo_misalignment/100000`). It will contain the number in plain text for ease of readability.

The elastic cross section depends on the model (DPM in our case) and the angular distribution. The DPM model diverges at small scattering angles because of the coulomb part of the interaction, so the integral limits must be given. The default values are 2.7 mrad to 13 mrad.

If these limits are used and 10.000.000 events are generated, the cross sections and luminosities are:

| Beam Momentum | Lower Angle | Upper Angle | Cross Section | Generated Luminosity @ 1e7 events |
| ------------- | ----------- | ----------- | ------------- | --------------------------------- |
| 1.5 GeV       | 2.7 mrad    | 13 mrad     | 20.8549       | 4.7950e5                          |
| 15 GeV        | 2.7 mrad    | 13 mrad     | 3.38184       | 2.9569e6                          |

Keep in mind that sometimes jobs crash on the compute cluster, and instead of 10.000.000 events you may end up with only 9.900.000 events. The integrated luminosity will then be smaller accordingly!

The fitted result is typically accurate to about 1%. If the fit results deviate significantly from these numbers, something went wrong.

## Resultant Files

Remember the two subdirectories for dpm data and res/acc test data. Refer to [DataLocations.md] if needed.

If the fit software seemed to work and the `bunches_10/binning_300/merge_data` paths in the DPM subdirectory have been created, look inside the `merge_data` folder.

There should be several files.

## `runLmdFit.log`

- The log file for the actual luminosity fit (for example from `./determineLuminosity.py`). If this doesn't exist, the lumi fit didn't even start. Look in the agent log instead to see if the Slurm Agent successfully submitted the jobs to SLURM.

- If the software crashed, you'll see it in this log. Compile the Luminosity Fit Framework with the DEBUG build target and see where the software crashes.

- If the fit didn't converge, look at the `lmd_data_1of1.root`.

- If the fit converged, you'll see some results (example for 1e7 events @ 1.5 GeV):

> ```
> dpm_angular_1d:luminosity         = 477959       +/-  311.482
> dpm_angular_2d:tilt_x     = 1.02419e-05  +/-  1.04452e-06
> dpm_angular_2d:tilt_y     = -7.26898e-06         +/-  1.10232e-06
> ```

## `lmd_data_1of1.root`

The `lmd_data_1of1.root` contains the merged resolution and acceptance calculations. That's why it's in the `merge_data` folder. It contains the data that is used by the fit and two histograms, which indicate if the resolution and acceptance calculations were successful.

