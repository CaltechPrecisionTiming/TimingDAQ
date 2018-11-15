from glob import glob
import shutil, os, argparse
import subprocess
import time, re
import numpy as np

VMERaw_file_template = 'VME/RAW/RawDataSaver0CMSVMETiming_RunRN*dat'
NimPlus_file_template = 'NimPlus/TriggerCountNimPlus_RN.cnt'
Tracks_file_template = 'Tracks/RunRN_CMSTiming_converted.root'

cp_cmd_template = 'rsync -uv --progress ../data/VME/RECO/{version}/DataVMETiming_Run{RN}.root otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_11_November_CMSTiming/RECO/{version}/ &> ~/tmp/transfer.log &'

cmd_DQM_template = '../DataQualityMonitor/runDQM.sh {RN} {v} &> ~/tmp/DQM.log &'

def GetCommandLineArgs():
    p = argparse.ArgumentParser()

    p.add_argument('--v_fast', type=str, default=None, help='Version of the config to run inline. (e.g. vf1).\nIf None no inline decoding is run')
    p.add_argument('--v_full', type=str, default=None, help='If None not run')

    p.add_argument('--wait_for_NimPlus', action='store_true', default=False, help='Wait for binary before recostructing it')
    p.add_argument('--wait_for_tracks', action='store_true', default=False, help='Wait for track before recostructing it')
    p.add_argument('--run_DQM', action='store_true', default=False, help='Run DQM')

    p.add_argument('--auto_copy', action='store_true', default=False, help='Run DQM')

    p.add_argument('--data_dir', default='../data')

    p.add_argument('--ignore_before', type=int, default=0)

    p.add_argument('--max_void', type=int, default=-1)
    p.add_argument('--sleep', type=float, default=60)
    p.add_argument('--min_file_age', type=float, default=15)

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

    last_run_number = args.ignore_before
    nothing_changed = 0

    while(args.max_void < 0 or nothing_changed < args.max_void):
        latest_file = glob(data_dir + Tracks_file_template.replace('RN', '*'))[-1]
        # latest_file = glob(data_dir + VMERaw_file_template.replace('RN', '*'))[-1]
        # run_number = int(re.search('Run[0-9]+_', latest_file).group(0)[3:-1])

        # flist = glob(data_dir + Tracks_file_template.replace('RN', '*'))
        flist = glob(data_dir + VMERaw_file_template.replace('RN', '*'))
        rlist = map(lambda x: int(re.search('Run[0-9]+', x).group(0)[3:]), flist)
        run_number = np.max(rlist)


        has_run =  False
        while run_number > last_run_number:
            age_check = time.time() - os.path.getmtime(latest_file) > args.min_file_age
            tks_file = data_dir + Tracks_file_template.replace('RN', str(run_number))
            tracks_check = os.path.exists(tks_file)
            if args.wait_for_tracks and tracks_check:
                while time.time() - os.path.getmtime(tks_file) < 5:
                    print "Waiting for traks to be transferred"
                    time.sleep(5)
            nimplus_check = os.path.exists(data_dir + NimPlus_file_template.replace('RN', str(run_number)))

            if age_check and ((not args.wait_for_tracks) or tracks_check) and ((not args.wait_for_NimPlus) or nimplus_check):
                if not args.v_fast == None:
                    cmd = 'python automation/DecodeData.py --vVME {0} -R {1}'.format(args.v_fast, run_number)
                    print cmd
                    subprocess.call(cmd, shell=True)
                    if args.run_DQM:
                        cmd = cmd_DQM_template.format(RN=run_number, v=args.v_fast)
                        print cmd
                        subprocess.call(cmd, shell=True)

                    if args.auto_copy:
                        print "Copying", args.v_fast
                        cmd = cp_cmd_template.format(version=args.v_fast, RN=run_number)
                        subprocess.call(cmd, shell=True)

                if not args.v_full == None:
                    cmd = 'python automation/DecodeData.py --vVME {0} -R {1}'.format(args.v_full, run_number)
                    cmd += ' &> ~/tmp/{}.log &'.format(run_number)
                    print cmd
                    subprocess.call(cmd, shell=True)

                last_run_number = run_number
                has_run = True
                nothing_changed = 0
            else:
                run_number -= 1

        if not has_run:
            nothing_changed += 1

        print 'Last run processed {0}\n...Going to sleep for {1:.0f} s\n\n'.format(last_run_number, args.sleep)
        time.sleep(args.sleep)

    print '\n\nStopped because nothing changed for at least {0:.1f} min'.format(args.sleep*nothing_changed/60.0)
