from glob import glob
import shutil, os, argparse
import subprocess
import time, re

#RN is a wildcard for run run number
VME_file_template = 'CMSTiming/RawDataSaver0CMSVMETiming_RunRN_*_Raw.dat'
NimPlus_file_template = 'NimPlus/TriggerCountNimPlus*_RN.cnt'

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('-R', '--runs', type=int, nargs='+', default=None)

    p.add_argument('--dir_CERN', type=str, default='/lxplus/data')
    p.add_argument('--dir_CMSLPC', type=str, default='sxie@cmslpc31.fnal.gov:/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data')

    p.add_argument('--no_CERN', action='store_true', default=False)
    p.add_argument('--no_CMSLPC', action='store_true', default=False)

    p.add_argument('--max_void', type=int, default=-1)
    p.add_argument('--sleep', type=float, default=60)
    p.add_argument('--min_file_age', type=float, default=90)

    p.add_argument('-f', '--force', action='store_true', default=False)

    return p.parse_args()

def transfer(args, runs):
    for rn in runs:
        print '============= Processing run ' + str(rn) + ' ============='
        cmd_VME = 'rsync -artu --progress ' + VME_file_template.replace('RN', str(rn))
        cmd_NimPlus = 'rsync -artu --progress ' + NimPlus_file_template.replace('RN', str(rn))

        if not args.no_CERN:
            print '\nCERN....'
            if not os.path.exists(args.dir_CERN + 'VME/RAW/RawDataVMETiming_Run{0}.dat'.format(rn)) or args.force:
                subprocess.call(cmd_VME + ' ' + args.dir_CERN + 'VME/RAW/', shell=True)
                # print cmd_VME + ' ' + args.dir_CERN + 'VME/RAW/'
                subprocess.call(cmd_NimPlus + ' ' + args.dir_CERN + 'NimPlus/', shell=True)
                # print cmd_NimPlus + ' ' + args.dir_CERN + 'NimPlus/'
            else:
                print args.dir_CERN + 'VME/RAW/RawDataVMETiming_Run{0}.dat'.format(rn), 'already existing.'
        if not args.no_CMSLPC:
            print '\nCMSLPC....'
            subprocess.call(cmd_VME + ' ' + args.dir_CMSLPC + 'VME/RAW/', shell=True)
            # print cmd_VME + ' ' + args.dir_CMSLPC + 'VME/RAW/'
            subprocess.call(cmd_NimPlus + ' ' + args.dir_CMSLPC + 'NimPlus/', shell=True)
            # print cmd_NimPlus + ' ' + args.dir_CMSLPC + 'NimPlus/'

if __name__ == '__main__':
    args = GetCommandLineArgs()

    if not args.dir_CERN.endswith('/'):
        args.dir_CERN += '/'
    if not args.dir_CMSLPC.endswith('/'):
        args.dir_CMSLPC += '/'

    if args.runs==None:
        last_run_number = 0
        nothing_changed = 0

        while(args.max_void < 0 or nothing_changed < args.max_void):
            latest_file = glob(VME_file_template.replace('RN', '*'))[-1]
            run_number = re.search('_Run[0-9]+_', latest_file).group(0)[4:-1]

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

    else:
        if len(args.runs) == 2 and (args.runs[0] < args.runs[1]):
            runs_list = range(args.runs[0], args.runs[1]+1)

        transfer(args, args.runs)
