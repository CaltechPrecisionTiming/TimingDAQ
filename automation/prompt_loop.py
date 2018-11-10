from glob import glob
import shutil, os, argparse
import subprocess
import time, re

VMERaw_file_template = 'VME/RAW/RawDataSaver0CMSVMETiming_RunRN_*_Raw.dat'

def GetCommandLineArgs():
    p = argparse.ArgumentParser()

    p.add_argument('--v_fast', type=str, default=None)
    p.add_argument('--v_full', type=str, default=None)

    p.add_argument('--data_dir', default='../data')

    p.add_argument('--ignore_before', type=int, default=0)

    p.add_argument('--max_void', type=int, default=-1)
    p.add_argument('--sleep', type=float, default=60)
    p.add_argument('--min_file_age', type=float, default=10)

    return p.parse_args()

if __name__ == '__main__':
    args = GetCommandLineArgs()

    data_dir = args.data_dir
    if not data_dir.endswith('/'):
        data_dir += '/'

    last_run_number = args.ignore_before
    nothing_changed = 0

    while(args.max_void < 0 or nothing_changed < args.max_void):
        latest_file = glob(args.data_dir + VMERaw_file_template.replace('RN', '*'))[-1]
        run_number = int(re.search('_Run[0-9]+_', latest_file).group(0)[4:-1])

        if run_number > last_run_number and time.time() - os.path.getmtime(latest_file) > args.min_file_age:
            cmd = 'python automation/DecodeData.py -f --vVME {0} -R {1}'.format()


        run_list = [int(run_number)-1]

        if time.time() - os.path.getmtime(latest_file) > args.min_file_age:
            run_list.append(int(run_number))

        transfer(args, run_list)

        if last_run_number != run_list[-1]:
            last_run_number = run_list[-1]
        else:
            nothing_changed += 1

        print '\n\n...Going to sleep for {0:.0f} s'.format(args.sleep)
        time.sleep(args.sleep)

    print '\n\nStopped because nothing changed for at least {0:.1f} min'.format(args.sleep*nothing_changed/60.0)
