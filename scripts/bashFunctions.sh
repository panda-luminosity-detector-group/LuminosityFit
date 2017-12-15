#!/bin/bash

#return which of the two numbers is smaller
min () {
  if [ $1 -lt $2 ]; then
    echo $1
  else
    echo $2
  fi
}

user_agree () {
  while [ 1 ]
  do
    read -p "Are these settings correct? Note: Choosing no [n] will terminate the program and has to be entirely reexecuted! [y/n]" answer
    if [ "$answer" = "y" ]; then
      return 1
    elif [ "$answer" = "n" ]; then
      return 0
    fi
  done
}

check_stage_success () {
  file_url=$1
  echo "checking wether ${file_url} exists and is larger than 3kB... "
  result=""
  if [ -e ${file_url} ] ; then
    filesize=$(stat -c%s -L "${file_url}")
    if [ "$filesize" -gt "3000" ]; then
      echo "found file and is larger than corrupted file size"
      return 1
    fi
    #if less ${logfile_url} | grep succes ; then
    #  echo "found file"
    #  return 1
    #fi
  fi
  echo "nope..."
  return 0
}

#check if $GEN_DATA env is set and points to a valid directory!
check_output_dir () {
  if [ ! ${GEN_DATA} ]; then
    exec >&2; echo "Error: Please set the environment variable GEN_DATA to the directory path where you want the files to be stored!";
    exit 1
  else
    if [ ! -d ${GEN_DATA} ]; then
      exec >&2; echo "Error: Please make sure the directory the variable GEN_DATA=${GEN_DATA} points to actually exists!";
      exit 1
    else 
      echo Using ${GEN_DATA} as saving path!
    fi
  fi
}

# checker functions
check_for_float () {
  echo "checking wether $1 is a floating point number!"
  result=""
  #`echo $1 | grep -Eq '^[-+]?[0-9]+\.?[0-9]*$' && echo 1`
  result=`echo $1 | grep -Eq '^[0-9]+\.?[0-9]*$' && echo 1`
  if [ $result ]; then
    echo "is ok!"
    #everything is ok
    return 1
  fi
  echo "nope.."
  return 0
}

check_for_int () {
  echo "checking wether $1 is an unsigned integer!"
  result=""
  if ! [[ "$1" =~ ^[0-9]+$ ]] ; then
    echo "nope.."
    return 0 
  fi
  echo "is ok"
  return 1
}

check_for_dir () {
  echo "checking wether $1 is an existing directory!"
  result=""
  dir_path=$1
  if [ ! -d $1 ]; then
    mkdir $1
    echo "directory was created!"
  else
    read -p "WARNING: Directory already exists. Are you sure you want to use this directory for the data output (data may get overwritten!)? [y/n]" answer
    if [ "$answer" = "y" ]; then
      return 1
    else
      return 0
    fi
  fi
  return 1
}


# ok now we have all options the user specified... now get the remaining from input
check_or_get_float_option () {
  ok=0
  first=1
  value=$2
  string=$1
  while [ "$ok" -eq 0 ]
  do
    if [ "$first" -eq 0 ] || [ ! $value ]; then
      read -p "You have not specified the $string! Please enter your desired $string now (no +- signs!): " value
    else
      first=0
    fi
    check_for_float $value
    ok=$?
  done
}

check_or_get_int_option () {
  ok=0
  first=1
  string=$1
  value=$2
  moreinfo=""
  if [ $namesarray ]; then
    moreinfo=" ("
    counter=0;
    for t in "${namesarray[@]}"; do
      moreinfo="$moreinfo$counter: $t; "
      counter=$(($counter+1))
    done;
    moreinfo="$moreinfo)"
  fi
  while [ "$ok" -eq 0 ]
  do
    if [ "$first" -eq 0 ] || [ ! $value ]; then
      read -p "You have not specified the $string! Please enter your desired $string$moreinfo now: " value
    else
      first=0
    fi
    check_for_int $value
    ok=$?
  done
}

check_or_get_dir_option () {
  ok=0
  first=1
  value=$2
  string=$1
  while [ "$ok" -eq 0 ]
  do
    if [ "$first" -eq 0 ] || [ ! $value ]; then
      read -p "You have not specified the $string! Please enter your desired $string now: " value
    else
      first=0
    fi
    check_for_dir ${GEN_DATA}/$value
    ok=$?
  done
}
