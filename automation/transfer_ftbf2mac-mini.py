from glob import glob
import shutil, os, argparse
import subprocess
import time, re
import numpy as np

#RN is a wildcard for run run number
VME_file_template = 'CMSTiming/RawDataSaver0CMSVMETiming_RunRN_*_Raw.dat'
#running master
# NimPlus_file_template = 'NimPlus/TriggerCountNimPlus*_RN.cnt' 
#Running as slave
NimPlus_file_template = '/../2018_11_November_RD53/NimPlus/TriggerCountNimPlus*_RN.cnt'

#Copy Desktop Digitizer Data
DT5742_file_template = 'DT5742/DT5742_RAW_RunRN.dat'

cmd_RunHyperScript_template = 'ssh otsdaq@rulinux04.dhcp.fnal.gov \''
cmd_RunHyperScript_template += 'cd CMSTiming; ./HyperScriptFastTrigger_NewGeo_18_12_11.sh RN'
cmd_RunHyperScript_template += '\''

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('-R', '--runs', type=int, nargs='+', default=None)

    p.add_argument('--dir_ftbf', type=str, default='/data/TestBeam/2018_11_November_CMSTiming/')
    p.add_argument('--dir_data', type=str, default='../data/')

    p.add_argument('-H', '--run_HyperScript', action='store_true', default=False)

    p.add_argument('--max_void', type=int, default=-1)
    p.add_argument('--sleep', type=float, default=60)
    p.add_argument('--min_file_age', type=float, default=90)

    p.add_argument('-v', '--verbose', action='store_true', default=False)

    return p.parse_args()

def transfer(args, runs):
    for rn in runs:
        print '============= Processing run ' + str(rn) + ' ============='
        cmd = 'rsync -artu --progress otsdaq@ftbf-daq-08:' + args.dir_ftbf

        cmd_VME = cmd + VME_file_template.replace('RN', str(rn))
        cmd_VME += ' ' + args.dir_data + 'VME/RAW/'
        # print cmd_VME
        subprocess.call(cmd_VME, shell=True)

        cmd_NimPlus = cmd + NimPlus_file_template.replace('RN', str(rn))
        cmd_NimPlus += ' ' + args.dir_data + 'NimPlus/'
        # print cmd_NimPlus
        subprocess.call(cmd_NimPlus, shell=True)
        
        cmd_DT5742 = cmd + DT5742_file_template.replace('RN', str(rn))
        cmd_DT5742 += ' ' + args.dir_data + 'DT5742/RAW/'
        # print cmd_VME
        subprocess.call(cmd_DT5742, shell=True)

        if args.run_HyperScript:
            print 'Running Hyperscript'
            cmd = cmd_RunHyperScript_template.replace('RN', str(rn))
            cmd += ' &> /tmp/Hyper_{}.log &'.format(rn)
            print cmd
            subprocess.call(cmd, shell=True)

def get_last_remote_file(args):
    cmd = "ssh otsdaq@ftbf-daq-08 \'cd " + args.dir_ftbf + "; "
    cmd += "ls -1ltr " + VME_file_template.replace('RN', '*') + "\'"

    output = subprocess.check_output(cmd, shell=True)
    last_file = output.split('\n')[-2]
    last_file = last_file.split(' ')
    fn = last_file[-1]

    size = int(last_file[-5])
    run_number = re.search('_Run[0-9]+_', fn).group(0)[4:-1]
    return int(run_number), size

if __name__ == '__main__':
    args = GetCommandLineArgs()

    if not args.dir_ftbf.endswith('/'):
        args.dir_ftbf += '/'
    if not args.dir_data.endswith('/'):
        args.dir_data += '/'

    if args.runs==None:
        last_high_run_number = 0
        last_size = 0
        nothing_changed = 0

        last_run_number, last_size = get_last_remote_file(args)
        print 'Current last remote run: {} ({:.1f} MB)'.format(last_run_number, last_size/float(8*1024*1024))
        print 'Starting the loop in 5 s'
        time.sleep(5)

        while(args.max_void < 0 or nothing_changed < args.max_void):
            high_run_number, size = get_last_remote_file(args)
            if last_high_run_number == high_run_number and size > last_size:
                last_size = size
                if args.verbose:
                    print 'Last remote file possibly increasing in size, considering previous one.'
                high_run_number -= 1
            elif last_high_run_number != high_run_number:
                last_high_run_number == high_run_number
                last_size = size
                if args.verbose:
                    print 'Last remote file possibly increasing in size, considering previous one.'
                high_run_number -= 1


            flist = glob(args.dir_data + 'VME/RAW/*')
            rlist = map(lambda x: int(re.search('_Run[0-9]+', x).group(0)[4:]), flist)
            run_number_local = np.max(rlist)

            run_list = range(run_number_local+1, high_run_number+1)

            if len(run_list) > 0:
                print 'Transfering runs', run_list
                transfer(args, run_list)
                nothing_changed = 0
            else:
                print 'Nothing to be done'
                nothing_changed += 1

            print '\n...Going to sleep for {0:.0f} s'.format(args.sleep)
            time.sleep(args.sleep)

        print '\n\nStopped because nothing changed for at least {0:.1f} min'.format(args.sleep*nothing_changed/60.0)

    else:
        if len(args.runs) == 2 and (args.runs[0] < args.runs[1]):
            runs_list = range(args.runs[0], args.runs[1]+1)
        else:
            run_list = args.runs

        transfer(args, run_list)
