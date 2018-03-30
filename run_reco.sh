#!/bin/bash
numberlo=$1
numberhi=$2

data_dir=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming
code_dir=/uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ

echo "range from run number  ${numberlo} to run number ${numberhi}"

for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
{
  echo "processing run number " ${runNum}
  echo "copying Raw VME data to lpc eos"
  rsync -artv --progress otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_03_March_CMSTiming/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat $data_dir/
  nfiles=$(ls /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat | wc -l)
  echo "number of raw DRS files: ${nfiles}"
  if [ $nfiles -gt 1 ]
  then
    echo "Combining .dat files"
    if [ -e $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat ]
    then
    	echo -e "${RED}Combined .dat file already exists.${NC} Delete it first if you would like to recombine."
    	echo "Filename: $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
    else
    	cat $(ls -v $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat) > $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat
    	echo "Created $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
    fi
    flag=combined
  else
    flag=$(ls /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run110_*.dat | tail -c 10 | head -c 1)
  fi

  echo $flag

  echo "Recostructing VME and merging pixel data:"
  echo "$code_dir/VMEDat2Root --input_file=$data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_${flag}_Raw.dat --pixel_input_file=$data_dir/Run${runNum}_CMSTiming_converted.root --output_file=$data_dir/RECO/V1/RawDataSaver0CMSVMETiming_Run${runNum}.root --config=$code_dir/config/VME_FNALTestbeam_180329_v1.config"

  $code_dir/VMEDat2Root --input_file=$data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_${flag}_Raw.dat --pixel_input_file=$data_dir/Run${runNum}_CMSTiming_converted.root --output_file=$data_dir/RECO/V1/RawDataSaver0CMSVMETiming_Run${runNum}.root --config=$code_dir/config/VME_FNALTestbeam_180329_v1.config

  echo "finish with processing run number " ${runNum}
}
