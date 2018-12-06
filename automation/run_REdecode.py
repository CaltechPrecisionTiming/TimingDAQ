from glob import glob
import shutil, os, argparse
import subprocess
import time, re
import numpy as np

VMERaw_file_template = 'VME/RAW/RawDataSaver0CMSVMETiming_RunRN*dat'
NimPlus_file_template = 'NimPlus/TriggerCountNimPlus_RN.cnt'
Tracks_file_template = 'Tracks/RunRN_CMSTiming_converted.root'

cmd_DQM_template = '../DataQualityMonitor/runDQM.sh {RN} {v} {dir}'# &> ~/tmp/DQM.log &'
# if not os.path.isdir(os.environ['HOME']+'/tmp'):
#     os.mkdir(os.environ['HOME']+'/tmp')

def GetCommandLineArgs():
    p = argparse.ArgumentParser()

    p.add_argument('-R', '--runs', type=int, nargs='+', help='List of runs to be processed. If two runs are given: if the order is increasing all the runs in the middle are processed as well, otherwise not.')

    p.add_argument('--v_fast', type=str, default=None, help='Version of the config to run inline. (e.g. vf1).\nIf None no inline decoding is run')
    p.add_argument('--v_full', type=str, default=None, help='If None not run')

    p.add_argument('--wait_for_NimPlus', action='store_true', default=False, help='Wait for binary before recostructing it')
    p.add_argument('--wait_for_tracks', action='store_true', default=False, help='Wait for track before recostructing it')
    p.add_argument('--run_DQM', action='store_true', default=False, help='Run DQM')

    p.add_argument('--data_dir', default='../data')

    p.add_argument('--opt_DecodeData', type=str, default='', help='options to pass to DecodeData.py. Use xx instead of -- to declare options.')

    return p.parse_args()

if __name__ == '__main__':
    args = GetCommandLineArgs()

    if args.v_fast==None and args.v_full==None:
        print 'At least v_fast or v_full needs to be given'
        print 'Run with -h for help'
        exit(0)

    data_dir = args.data_dir
    if not data_dir.endswith('/'):
        data_dir += '/'

    runs_list = []
    if len(args.runs) == 2 and (args.runs[0] < args.runs[1]):
        runs_list = range(args.runs[0], args.runs[1]+1)
    else:
        runs_list = args.runs

    for run in runs_list:
            tks_file = data_dir + Tracks_file_template.replace('RN', str(run))
            tracks_check = os.path.exists(tks_file)
            nimplus_check = os.path.exists(data_dir + NimPlus_file_template.replace('RN', str(run)))

            if ((not args.wait_for_tracks) or tracks_check) and ((not args.wait_for_NimPlus) or nimplus_check):
                if not args.v_fast == None:
                    cmd = 'python automation/DecodeData.py --vVME {0} -R {1}'.format(args.v_fast, run)
                    if args.opt_DecodeData:
                        cmd += ' ' + args.opt_DecodeData.replace('xx', '--')
                    print cmd
                    subprocess.call(cmd, shell=True)
                    if args.run_DQM:

                        cmd = cmd_DQM_template.format(RN=run, v=args.v_fast, dir=os.environ['PWD']+'/..')
                        print cmd
                        subprocess.call(cmd, shell=True)

                if not args.v_full == None:
                    cmd = 'python automation/DecodeData.py --vVME {0} -R {1}'.format(args.v_full, run)
                    if args.opt_DecodeData:
                        cmd += ' ' + args.opt_DecodeData
                    cmd += ' &> ~/tmp/{}.log &'.format(run)
                    print cmd
                    subprocess.call(cmd, shell=True)

            elif args.wait_for_tracks and not tracks_check:
                print 'Missing tracks for run', run
            elif args.wait_for_NimPlus and not NimPlus_check:
                print 'Missing NimPlus for run', run
