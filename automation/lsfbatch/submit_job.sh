#!/bin/bash
run_number=$1
config_version=v2ct

dir=/eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/TimingDAQ/automation/lsfbatch

bsub -q 1nh -J decode_$run_number $dir/job.sh $run_number $config_version

echo
echo 'Done'
