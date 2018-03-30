#!/bin/bash
numberlo=$1
numberhi=$2

data_dir=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming
code_dir=/uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ
config_file=$code_dir/config/VME_FNALTestbeam_180329_v1.config

for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
{
  echo "Processing run number " ${runNum}
  rsync -artvh --progress otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_03_March_CMSTiming/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat $data_dir/
  nfiles=$(ls ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat | wc -l)
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

  pixel_file=$data_dir/Run${runNum}_CMSTiming_converted.root
  output_file=$data_dir/RECO/V2/RawDataSaver0CMSVMETiming_Run${runNum}.root
  input_file=$data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_${flag}.dat

  $code_dir/VMEDat2Root --input_file=$input_file --pixel_input_file=$pixel_file --output_file=$output_file --config=$config_file --save_meas

  echo "Finished processing run " ${runNum}
}
