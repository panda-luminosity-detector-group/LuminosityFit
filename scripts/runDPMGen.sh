#! /bin/bash

cd ${VMCWORKDIR}/macro/lmd/steve/myDpm
random_num=$RANDOM.$RANDOM

if [ $PBS_ARRAYID ]; then
  index=$PBS_ARRAYID
fi

if [ $# -eq '8' ]; then
  lab_momentum=$1
  num_events=$2
  reaction_type=$3
  minimal_theta_value=$4
  dirname=$5
  dirname_cleaned=$6
  basedir=$7
  index=$8
fi

filename=${dirname_cleaned}_$index.root

if [ ! -d $basedir/${dirname} ]; then
  mkdir $basedir/${dirname}
fi

echo Running ./DPMGen -s ${random_num} -m ${lab_momentum} -n ${num_events} -e ${reaction_type} -t ${minimal_theta_value} -f $basedir/${dirname}/$filename
./DPMGen -s ${random_num} -m ${lab_momentum} -n ${num_events} -e ${reaction_type} -t ${minimal_theta_value} -f $basedir/${dirname}/$filename

sleep 10;
exit 0;
