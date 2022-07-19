# Data Locations and Paths

#TODO:

- dpm vs box
- $LMDFIT_DATA_DIR
- bunches, binning and merge data


There are two main sub folder per simulated beam momentum:

- `plab_1.5GeV/box_theta_[...]`
- `plab_1.5GeV/dpm_elastic_theta[...]`

It is absolutely critical that these two are not confused.

The `box_theta` folder contains the acceptance and resolution simulation data. Basically it is MC data that was generated with the box generator and then reconstructed with the original reconstruction chain. This way, the detector acceptance and resolution can be modeled realistically. Because of this, it looks very similar to the `dpm_elastic` folder, but serves a different purpose.