#!/bin/bash

# Check if the "singularity" binary is available in the $PATH for the current user
if ! command -v singularity &> /dev/null; then
    echo "Singularity is not available. Loading module..."
    module load tools/Singularity
fi

# After ensuring that Singularity is available, run the singularity image
singularity run $HOME/lmdfitNov22p1.sif
