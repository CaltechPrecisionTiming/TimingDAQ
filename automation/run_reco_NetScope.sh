#!/bin/bash

Force=false
freedom=false

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -f|--force)
    Force=true
    shift # past argument
    shift # past value
    ;;
    -F|--FREEDOM)
    freedom=true
    shift # past argument
    shift # past value
    ;;
    --default)
    DEFAULT=YES
    shift # past argument
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

if [ "${freedom}" == true ]
then
  echo
  echo
  echo "   * * * * * * ########################"
  echo "    * * * * *                          "
  echo "   * * * * * * ########################"
  echo "    * * * * *                          "
  echo "   * * * * * * ########################"
  echo "                                       "
  echo "   ####################################"
  echo "                                       "
  echo "   ####################################"
  echo "                                       "
  echo "   ####################################"
  echo
  echo
fi

numberlo=$1
numberhi=$2

# data_dir=../data/ETL_march_testbeam/RAW/
data_dir=/eos/uscms/store/user/cmstestbeam/ETL/2018_04/OTSDAQ/CMSTiming/NetScope/RAW
#data_dir=/uscms_data/d1/sxie/data/CMSTiming/
#data_dir=/tmp/zhicai/
output_name=DataNetScope
# output_dir=../data/ETL_march_testbeam/RECO_v1
output_dir=$data_dir/../RECO/v1
code_dir=.
config_file=$code_dir/config/NetScope_FNALTestbeam_180408_v1.config

for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
{
  echo "============================= Processing Run ${runNum} ============================="
  rsync -artvh --progress otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_03_March_CMSTiming/NetScopeTiming/* $data_dir/
  nfiles=$(ls ${data_dir}/RawDataSaver0NetScope_Run${runNum}_*_Raw.dat | wc -l)
  echo "number of raw DRS files: ${nfiles}"
  echo
  if [ $nfiles -gt 1 ]
  then
    echo "Combining .dat files"
    if [ -e $data_dir/RawDataSaver0NetScope_Run${runNum}_combined.dat ]
    then
    	echo -e "Combined .dat file already exists."
    	echo "Filename: $data_dir/RawDataSaver0NetScope_Run${runNum}_combined.dat"
    else
    	cat $(ls -v $data_dir/RawDataSaver0NetScope_Run${runNum}_*.dat) > $data_dir/RawDataSaver0NetScope_Run${runNum}_combined.dat
    	echo "Created $data_dir/RawDataSaver0NetScope_Run${runNum}_combined.dat"
    fi
    flag=combined
  else
    flag=$(ls ${data_dir}/RawDataSaver0NetScope_Run${runNum}_*.dat | tail -c 10 | head -c 5)
  fi

  output_file=$output_dir/${output_name}_Run${runNum}.root

  if [ -e $output_file ] && [ "${Force}" == false ]
  then
    echo "Run$runNum already present in output directory"
  else
    input_file=$data_dir/RawDataSaver0NetScope_Run${runNum}_${flag}.dat
    echo "${code_dir}/NetScopeDat2Root --input_file=${input_file} --output_file=${output_file} --config=${config_file} --save_meas"
    $code_dir/NetScopeDat2Root --input_file=$input_file --output_file=$output_file --config=$config_file --save_meas
  fi

  echo "Finished processing run " ${runNum}
}
