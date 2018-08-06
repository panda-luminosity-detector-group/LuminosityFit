# LuminosityFit

## Installation

### Prerequisites
Make sure your Pandaroot enviroment is set up correctly, more precisely that these environment variables are set:
- VMCWORKDIR
- FAIRROOTPATH
- ROOTSYS

Additionally, make sure you are using a boost version which was build using a gcc version 5 or greater. You can set the location of the boost via the *BOOST_ROOT* environment variable.

### Compilation
Simply create a build directory, change into that build directory, and run `cmake {PATH_TO_YOUR_LUMINOSITY_FIT_SOURCE}`

## Using
Export the *build/bin* and *build/lib* directory to *$PATH* and *$LD_LIBRARY_PATH* respectively. Then you can run the python scripts in the ./scripts subdirectory.