#!/bin/bash
run_number=$1

dir=/eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/TimingDAQ/automation/lsfbatch

bsub -q 8nm -J decode_$run_number $dir/job.sh $run_number

echo
echo 'Done'
