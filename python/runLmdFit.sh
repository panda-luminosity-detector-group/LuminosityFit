#!/bin/bash 
###############################################################################
## script for running luminosity fitting/extraction procedure for LMD
###############################################################################

echo ${LMDFIT_BUILD_PATH}/bin/runLmdFit -c ${config_url} -d ${data_path} -a ${acceptance_resolution_path} -m ${number_of_threads}
${LMDFIT_BUILD_PATH}/bin/runLmdFit -c ${config_url} -d ${data_path} -a ${acceptance_resolution_path} -m ${number_of_threads}

sleep 10;
exit 0;
