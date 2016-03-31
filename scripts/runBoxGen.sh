#! /bin/bash

cd $PBS_O_WORKDIR
random_num=$RANDOM.$RANDOM

if [ $PBS_ARRAYID ]; then
  index=$PBS_ARRAYID
fi

if [ $# -eq '8' ]; then
  lab_momentum=$1
  num_events=$2
  theta_min=$3
  theta_max=$4
  dirname=$5
  dirname_cleaned=$6
  basedir=$7
  index=$8
fi

filename=${dirname_cleaned}_$index.root

if [ ! -d $basedir/${dirname} ]; then
  mkdir $basedir/${dirname}
fi

echo root -l -b -q 'standaloneBoxGen.C('${lab_momentum}', '${num_events}', '${theta_min}', '${theta_max}', "'$basedir/$dirname/$filename'", '${random_num}', '${use_recoil_mom}')'

root -l -b -q 'standaloneBoxGen.C('${lab_momentum}', '${num_events}', '${theta_min}', '${theta_max}', "'$basedir/$dirname/$filename'", '${random_num}', '${use_recoil_mom}')'

sleep 10;
exit 0;
