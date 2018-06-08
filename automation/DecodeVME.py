import os, sys, argparse, subprocess, glob

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('runs', type=int, nargs='+')
    p.add_argument('--no_pixel', action='store_true')
    p.add_argument('-f','--force', action='store_true')

    p.add_argument('--daq_dir', default='/data/TestBeam/2018_06_June_CMSTiming')
    p.add_argument('--NimPlus_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_06/data/NimPlus')
    p.add_argument('--raw_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_06/data/VME/RAW')
    p.add_argument('--out_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_06/data/VME/RECO/v1')
    p.add_argument('--code_dir', default=os.environ['PWD'])


    p.add_argument('--out_name', default='DataCMSVMETiming')

    return p.parse_args()

if __name__ == '__main__':
    args = GetCommandLineArgs()

    print args
    print args.runs

    runs_list = []
    if len(runs) == 2:
        runs_list = range(runs[0], runs[1]+1)
    else:
        runs_list = runs

    for run in runs_list:
        print '============================= Processing Run {} ============================='.format(run)

        #Get NimPlus triggers
        cmd = 'rsync -artvh otsdaq@ftbf-daq-08.fnal.gov:{}/NimPlus/TriggerCountNimPlus_{}.cnt {}/'.format(args.daq_dir, run, args.NimPlus_dir)
        out = subprocess.call(cmd, shell=True)
        if out:
            print 'NimPlus cnt file copy failed'

        NimPlus_file = args.NimPlus_dir + 'TriggerCountNimPlus_{}.cnt'.format(run)
        N_evt_expected_evts = -1
        if os.path.exists(NimPlus_file):
            cmd = 'more {}} | grep sig_norm | awk \'{print $3}\''.format(NimPlus_file)
            print cmd
            proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
            (out, err) = proc.communicate()
            N_evt_expected_evts = out
        else:
            print '[WARNING] NO NimPlus file present in ' + args.NimPlus_dir


        #Copy the VME data
        raw_filename = args.raw_dir + 'Raw' + args.out_name + 'Run{}.dat'.format(run)
        if not os.path.exists(raw_filename) or args.force:
            cmd = 'rsync -artvh otsdaq@ftbf-daq-08.fnal.gov:{}/CMSTiming/RawDataSaver0CMSVMETiming_Run{}_*_Raw.dat {}/'.format(args.daq_dir, run, args.raw_dir)
            print cmd
            subprocess.call(cmd, shell=True)

            copied_files = glob.glob('{}/RawDataSaver0CMSVMETiming_Run{}_*_Raw.dat'.format(args.raw_dir, run))

            if len(copied_files) > 1:
                cmd = 'cat ' + ' '.join(copied_files) + ' > ' +raw_filename
                print cmd
                subprocess.call(cmd, shell=True)
                for f in copied_files:
                    os.remove(f)
            else:
                cmd = 'mv '+ copied_files[0] + ' ' + raw_filename
                subprocess.call(cmd, shell=True)



#!/bin/bash
#
# pixel=true
# Force=false
# freedom=false
#
# POSITIONAL=()
# while [[ $# -gt 0 ]]
# do
# key="$1"
#
# case $key in
#     -np|--no_pixel)
#     pixel=false
#     shift # past argument
#     shift # past value
#     ;;
#     -f|--force)
#     Force=true
#     shift # past argument
#     shift # past value
#     ;;
#     -F|--FREEDOM)
#     freedom=true
#     shift # past argument
#     shift # past value
#     ;;
#     --default)
#     DEFAULT=YES
#     shift # past argument
#     ;;
#     *)    # unknown option
#     POSITIONAL+=("$1") # save it in an array for later
#     shift # past argument
#     ;;
# esac
# done
# set -- "${POSITIONAL[@]}" # restore positional parameters
#
# if [ "${pixel}" == true ]
# then
#     echo "[INFO] Running reconstruction WITH pixels"
# else
#     echo "[INFO] Running reconstruction WITHOUT pixels"
# fi
#
# if [ "${freedom}" == true ]
# then
#   echo
#   echo
#   echo "   * * * * * * ########################"
#   echo "    * * * * *                          "
#   echo "   * * * * * * ########################"
#   echo "    * * * * *                          "
#   echo "   * * * * * * ########################"
#   echo "                                       "
#   echo "   ####################################"
#   echo "                                       "
#   echo "   ####################################"
#   echo "                                       "
#   echo "   ####################################"
#   echo
#   echo
# fi
#
# numberlo=$1
# numberhi=$2
#
# #data_dir=../data/ETL_march_testbeam/RAW/
# data_dir=/eos/uscms/store/user/cmstestbeam/ETL/2018_04/OTSDAQ/CMSTiming/VME/RAW
# #data_dir=/uscms_data/d1/sxie/data/CMSTiming/
# #data_dir=/tmp/zhicai/
# output_name=DataCMSVMETiming
# #output_dir=../data/ETL_march_testbeam/RECO_v1
# output_dir=$data_dir/../RECO/v2
# code_dir=.
# config_file=$code_dir/config/VME_FNALTestbeam_180409_LGAD_v1.config
#
# for((runNum=${numberlo}; runNum<=${numberhi}; runNum++))
# {
#   echo "============================= Processing Run ${runNum} ============================="
#   rsync -artvh --progress otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_03_March_CMSTiming/CMSTiming/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat $data_dir/
#   nfiles=$(ls ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}_*_Raw.dat | wc -l)
#   echo "number of raw DRS files: ${nfiles}"
#   echo
#   if [ $nfiles -gt 1 ]
#   then
#     echo "Combining .dat files"
#     if [ -e $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat ]
#     then
#     	echo -e "Combined .dat file already exists."
#     	echo "Filename: $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
#     else
#     	cat $(ls -v $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat) > $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat
#     	echo "Created $data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_combined.dat"
#     fi
#     flag=combined
#   else
#     flag=$(ls ${data_dir}/RawDataSaver0CMSVMETiming_Run${runNum}_*.dat | tail -c 10 | head -c 5)
#   fi
#
#   echo "Recostructing VME and merging pixel data"
#
#   output_file=$output_dir/${output_name}_Run${runNum}.root
#   #if [ -e $output_file ]
#   if [ -e $output_file ] && [ "${Force}" == false ]
#   then
#     echo "Run$runNum already present in output directory"
#   else
#     pixel_file=$data_dir/Run${runNum}_CMSTiming_converted.root
#     input_file=$data_dir/RawDataSaver0CMSVMETiming_Run${runNum}_${flag}.dat
#
#     if [ -e $pixel_file ]
#     then
# 	if [ "${pixel}" == true ]
# 	then
# 	    echo "[INFO] Pixel file found. Running WITH pixels"
# 	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --pixel_input_file=${pixel_file} --output_file=${output_file} --config=${config_file} --save_meas"
# 	    $code_dir/VMEDat2Root --input_file=$input_file --pixel_input_file=$pixel_file --output_file=$output_file --config=$config_file --save_meas
# 	else
# 	    echo "[INFO] Pixel file found. Running WITHOUT pixels"
# 	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --output_file=${output_file} --config=${config_file} --save_meas"
# 	    $code_dir/VMEDat2Root --input_file=$input_file --output_file=$output_file --config=$config_file --save_meas
# 	fi
#     else
# 	if [ "${pixel}" == false ]
# 	then
# 	    echo "[INFO] Pixel file NOT found. Running without pixels"
# 	    echo "${code_dir}/VMEDat2Root --input_file=${input_file} --output_file=${output_file} --config=${config_file} --save_meas"
# 	    $code_dir/VMEDat2Root --input_file=$input_file --output_file=$output_file --config=$config_file --save_meas
# 	else
# 	    echo "[ERROR] Pixel file NOT found. NOT running the Recostructing. If you want to run  without tracks use <--no_pixel>."
# 	fi
#     fi
#   fi
#
#   echo "Finished processing run " ${runNum}
# }
