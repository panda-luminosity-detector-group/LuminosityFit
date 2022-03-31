#!/bin/bash

#Load the Singularity module
module load tools/Singularity

#if image is >250MB, change the TMP dir to prevent a overfull /tmp directory on node 
SINGULARITY_TMPDIR=/localscratch/${SLURM_JOB_ID}/singularity_tmp/
export SINGULARITY_TMPDIR
mkdir -p $SINGULARITY_TMPDIR
LMDPATH=${LMDFIT_BUILD_DIR}/../

# TODO: don't hardcode the path ffs, use environment variables
# TODO add bash -c and source config.sh here!
singularity exec --env-file ${LMDPATH}/lmdEnvFile.env /home/roklasen/lmdfit-mini.sif bash -c ". \$VMCWORKDIR/build/config.sh -a ; python /home/roklasen/LuminosityFit/python/runLmdSimReco.py"