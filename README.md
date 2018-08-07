# LuminosityFit

## Installation

### Prerequisites
Make sure your Pandaroot enviroment is set up correctly, more precisely that these environment variables are set:
- VMCWORKDIR
- FAIRROOTPATH
- ROOTSYS

A further requirement is the gsl library, which can be loaded on the himster2 via `module load numlib/GSL/2.4-foss-2017a`.
Additionally, make sure you are using a boost version which was build using a gcc version 5 or greater. You can set the location of the boost via the *BOOST_ROOT* environment variable.

### Compilation
Simply create a build directory, change into that build directory, and run `cmake {PATH_TO_YOUR_LUMINOSITY_FIT_SOURCE}`

## Using
The binaries in the ./bin subdirectory of the build path can be used directly. For more convenient use, especially for larger datasamples sizes it is recommended to use the python scripts in the ./scripts subdirectory. However, to use these scripts several environment variables have to be exported.

```bash
export LMDFIT_BUILD_PATH="path-to-your-luminosityfit-build-directory"
export DATA_HOME="path-to-himspecf-data-storage"`
export LMDFIT_GEN_DATA=$DATA_HOME/paluma/lmdfit_sim_gen_samples
export LMDFIT_DATA_DIR=$DATA_HOME/paluma/"directory-name-of-your-choice"
```