#!/bin/bash 
###############################################################################
## script for running luminosity fitting/extraction procedure for LMD
###############################################################################

echo ${VMCWORKDIR}/build/bin/runLmdFit -c ${config_url} -d ${data_path} -a ${acceptance_resolution_path} -m ${number_of_threads}
${VMCWORKDIR}/build/bin/runLmdFit -c ${config_url} -d ${data_path} -a ${acceptance_resolution_path} -m ${number_of_threads}

sleep 10;
exit 0;
