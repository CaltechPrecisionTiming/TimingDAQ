from glob import glob
import shutil, os, argparse
import subprocess
import time, re

VMERaw_file_template = 'VME/RAW/RawDataSaver0CMSVMETiming_RunRN_*_Raw.dat'
Tracks_file_template = 'Tracks/RunRN_CMSTiming_converted.root'

def GetCommandLineArgs():
    p = argparse.ArgumentParser()

    p.add_argument('--v_fast', type=str, default=None, help='Version of the config to run inline. (e.g. vf1).\nIf None no inline decoding is run')
    p.add_argument('--v_full', type=str, default=None, help='If None not run')

    p.add_argument('--wait_for_tracks', action='store_true', default=False, help='Wait for track before recostructing it')

    p.add_argument('--data_dir', default='../data')

    p.add_argument('--ignore_before', type=int, default=0)

    p.add_argument('--max_void', type=int, default=-1)
    p.add_argument('--sleep', type=float, default=30)
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
        latest_file = glob(data_dir + VMERaw_file_template.replace('RN', '*'))[-1]
        run_number = int(re.search('_Run[0-9]+_', latest_file).group(0)[4:-1])

        has_run =  False
        while run_number > last_run_number:
            age_check = time.time() - os.path.getmtime(latest_file) > args.min_file_age
            tracks_check = True
            if args.wait_for_tracks:
                tracks_check = os.path.exists(data_dir + Tracks_file_template.replace('RN', str(run_number)))

            if age_check and tracks_check:
                if not args.v_fast == None:
                    cmd = 'python automation/DecodeData.py -f --vVME {0} -R {1}'.format(args.v_fast, run_number)
                    print cmd
                    subprocess.call(cmd, shell=True)
                if not args.v_full == None:
                    cmd = './automation/lsfbatch/submit_job.sh {0} {1}'.format(run_number, args.v_full)
                    subprocess.call(cmd, shell=True)
                    
                last_run_number = run_number
                has_run = True
                nothing_changed = 0
            else:
                run_number -= 1

        if not has_run:
            nothing_changed += 1

        print '\n...Going to sleep for {0:.0f} s\n\n'.format(args.sleep)
        time.sleep(args.sleep)

    print '\n\nStopped because nothing changed for at least {0:.1f} min'.format(args.sleep*nothing_changed/60.0)
