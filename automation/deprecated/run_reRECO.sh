#!/bin/bash

pixel=true
Force=false
freedom=false
save_meas=false

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -np|--no_pixel)
    pixel=false
    shift # past argument
    ;;
    -f|--force)
    Force=true
    shift # past argument
    ;;
    -F|--FREEDOM)
    freedom=true
    shift # past argument
    ;;
    -SM|--save_meas)
    save_meas=true
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

bin_dir=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/RAW/VMEbin
track_dir=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/RAW/Tracks
nim_plus_dir=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/RAW/NimPlus


output_dir=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/reRECO_180419

code_dir=.
config_file=$code_dir/config/FNALTestbeam_1803/VME_BTLreRECO_180419.config

for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
{
  echo "============================= Processing Run ${runNum} ============================="

  input_file=$bin_dir/RawDataSaver0CMSVMETiming_Run${runNum}.dat
  pixel_file=$track_dir/Run${runNum}_CMSTiming_converted.root
  echo $(ls $pixel_file)
  nimplus_file=$nim_plus_dir/TriggerCountNimPlus_${runNum}.cnt

  output_file=$output_dir/DataCMSVMETiming_Run${runNum}.root

  N_exp=-1
  if [ -e $nimplus_file ]
  then
    N_exp=$(more $nimplus_file | grep sig_norm | awk '{print $3}')
  fi

  if [ -e $output_file ] && [ "${Force}" == false ]
  then
    echo "Run$runNum already present in output directory"
  else
    if [ -e $pixel_file ]
    then
    	if [ "${pixel}" == true ]
    	then
    	    echo "[INFO] Pixel file found. Running WITH pixels"
    	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --pixel_input_file=${pixel_file} --output_file=${output_file} --config=${config_file} --N_evt_expected=$N_exp"
    	    $code_dir/VMEDat2Root --input_file=$input_file --pixel_input_file=$pixel_file --output_file=$output_file --config=$config_file --N_evt_expected=$N_exp
    	else
    	    echo "[INFO] Pixel file found. Running WITHOUT pixels"
    	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --output_file=${output_file} --config=${config_file} --N_evt_expected=$N_exp"
    	    $code_dir/VMEDat2Root --input_file=$input_file --output_file=$output_file --config=$config_file --N_evt_expected=$N_exp
    	fi
    else
    	if [ "${pixel}" == false ]
    	then
    	    echo "[INFO] Pixel file NOT found. Running without pixels"
    	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --output_file=${output_file} --config=${config_file} --N_evt_expected=$N_exp"
    	    $code_dir/VMEDat2Root --input_file=$input_file --output_file=$output_file --config=$config_file --N_evt_expected=$N_exp
    	else
    	    echo "[ERROR] Pixel file NOT found. NOT running the Recostructing. If you want to run  without tracks use <--no_pixel>."
    	fi
    fi
  fi
}
