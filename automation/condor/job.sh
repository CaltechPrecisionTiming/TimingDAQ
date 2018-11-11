#!/bin/bash

run_number=$1
config_version=v2ct

cd /eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/TimingDAQ
source setup_lxplus.sh

python automation/DecodeData.py --vVME $config_version --NO_save_meas -R run_number
