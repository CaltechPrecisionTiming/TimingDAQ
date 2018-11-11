#!/bin/bash

run_number=$1
config_version=$2
data_dir=/eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/data

echo "============= Job for run $1 ===================="
cd /eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/CMSSW_9_0_2/src
echo $PWD
eval `scramv1 runtime -sh`
cd TimingDAQ
echo $PWD

logfile=automation/lsfbatch/log/$run_number.log

python automation/DecodeData.py --vVME $config_version --NO_save_meas -R $run_number --data_dir=$data_dir &> $logfile
