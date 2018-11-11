#!/bin/bash
run_number=$1
config_version=$2

dir=/eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/TimingDAQ/automation/lsfbatch

cd $dir/jobs
bsub -q 8nm -J decode_$run_number $dir/job.sh $run_number $config_version
cd -

echo
echo 'Done'
