#!/bin/bash

force=false
delete=false

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -f|--force)
    force=true
    shift # past argument
    shift # past value
    ;;
    -D|--delete)
    delete=true
    shift # past argument
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters


numberlo=$1
numberhi=$2

data_dir=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/RAW/VMEbin

for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
{
  echo "Run ${runNum}"

  nfiles=$(ls ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat | wc -l)
  echo "    Number of raw files: ${nfiles}"
  if [ $nfiles -gt 1 ]
  then
    if [ -e $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}.dat ]
    then
    	echo "    Combined .dat file already exists."
      if [ "${force}" == true ]
      then
        echo "    Creating $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}.dat"
        rm ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}.dat
        cat $(ls -v $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat) > $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}.dat
      fi
    else
      echo "    Creating $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}.dat"
    	cat $(ls -v $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat) > $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}.dat
    fi
    
    if [ "${delete}" == true ]
    then
      rm ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat
    fi
  else
    if [ $nfiles -gt 0 ]
    then
      mv $(ls -v $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat) $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}.dat
    fi
  fi

}
