#!/bin/bash
numberlo=$1
numberhi=$2
echo "range from run number  ${numberlo} to run number ${numberhi}"
for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
{
  echo "processing run number " ${runNum}
  echo "copying Raw VME data to lpc eos"
  scp otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_03_March_CMSTiming/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/
  nfiles=$(ls /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat | wc -l)
  echo "number of raw DRS files: ${nfiles}"
  if [ $nfiles -gt 1 ]
  then
    echo "Combining .dat files"
    if [ -e /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat ]
    then
	echo -e "${RED}Combined .dat file already exists.${NC} Delete it first if you would like to recombine."
	echo "Filename: /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
    else
	cat $(ls -v /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat) > /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat
	echo "Created /eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
    fi
    echo "Merge VME and Pixel data"
    /uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ/VMEDat2Root --input_file=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat --pixel_input_file=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/Run${runNum}_CMSTiming_converted.root --output_file=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RECO/V1/RawDataSaver0CMSVMETiming_Run${runNum}.root --config=/uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ/config/VME_FNALTestbeam_180329_v1.config.config
  else
  echo "Merge VME and Pixel data:"
  /uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ/VMEDat2Root --input_file=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat --pixel_input_file=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/Run${runNum}_CMSTiming_converted.root --output_file=/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/RECO/V1/RawDataSaver0CMSVMETiming_Run${runNum}.root --config=/uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ/config/VME_FNALTestbeam_180329_v1.config.config
  fi
  echo "finish with processing run number " ${runNum}
}
