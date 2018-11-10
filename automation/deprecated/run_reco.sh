#!/bin/bash

pixel=true
Force=false
freedom=false

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -np|--no_pixel)
    pixel=false
    shift # past argument
    shift # past value
    ;;
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

if [ "${pixel}" == true ]
then
    echo "[INFO] Running reconstruction WITH pixels"
else
    echo "[INFO] Running reconstruction WITHOUT pixels"
fi

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

#data_dir=../data/ETL_march_testbeam/RAW/
data_dir=/eos/uscms/store/user/cmstestbeam/ETL/2018_04/OTSDAQ/CMSTiming/VME/RAW
#data_dir=/uscms_data/d1/sxie/data/CMSTiming/
#data_dir=/tmp/zhicai/
output_name=DataCMSVMETiming
#output_dir=../data/ETL_march_testbeam/RECO_v1
output_dir=$data_dir/../RECO/v2
code_dir=.
config_file=$code_dir/config/VME_FNALTestbeam_180409_LGAD_v1.config

for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
{
  echo "============================= Processing Run ${runNum} ============================="
  rsync -artvh --progress otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_03_March_CMSTiming/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat $data_dir/
  nfiles=$(ls ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat | wc -l)
  echo "number of raw DRS files: ${nfiles}"
  echo
  if [ $nfiles -gt 1 ]
  then
    echo "Combining .dat files"
    if [ -e $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat ]
    then
    	echo -e "Combined .dat file already exists."
    	echo "Filename: $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
    else
    	cat $(ls -v $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat) > $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat
    	echo "Created $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
    fi
    flag=combined
  else
    flag=$(ls ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat | tail -c 10 | head -c 5)
  fi

  echo "Recostructing VME and merging pixel data"

  output_file=$output_dir/${output_name}_Run${runNum}.root
  #if [ -e $output_file ]
  if [ -e $output_file ] && [ "${Force}" == false ]
  then
    echo "Run$runNum already present in output directory"
  else
    pixel_file=$data_dir/Run${runNum}_CMSTiming_converted.root
    input_file=$data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_${flag}.dat

    if [ -e $pixel_file ]
    then
	if [ "${pixel}" == true ]
	then
	    echo "[INFO] Pixel file found. Running WITH pixels"
	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --pixel_input_file=${pixel_file} --output_file=${output_file} --config=${config_file} --save_meas"
	    $code_dir/VMEDat2Root --input_file=$input_file --pixel_input_file=$pixel_file --output_file=$output_file --config=$config_file --save_meas
	else
	    echo "[INFO] Pixel file found. Running WITHOUT pixels"
	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --output_file=${output_file} --config=${config_file} --save_meas"
	    $code_dir/VMEDat2Root --input_file=$input_file --output_file=$output_file --config=$config_file --save_meas
	fi
    else
	if [ "${pixel}" == false ]
	then
	    echo "[INFO] Pixel file NOT found. Running without pixels"
	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --output_file=${output_file} --config=${config_file} --save_meas"
	    $code_dir/VMEDat2Root --input_file=$input_file --output_file=$output_file --config=$config_file --save_meas
	else
	    echo "[ERROR] Pixel file NOT found. NOT running the Recostructing. If you want to run  without tracks use <--no_pixel>."
	fi
    fi
  fi

  echo "Finished processing run " ${runNum}
}
