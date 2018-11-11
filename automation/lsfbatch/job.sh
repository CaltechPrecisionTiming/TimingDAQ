#!/bin/bash

run_number=$1
config_version=v2ct
echo "============= Processing run $1 ===================="
cd /eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/CMSSW_9_0_2/src
echo $PWD
eval `scramv1 runtime -sh`
cd /eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/TimingDAQ
echo $PWD
# source setup_lxplus.sh
echo "Setup ok"
python automation/DecodeData.py --vVME $config_version --NO_save_meas -R $run_number
