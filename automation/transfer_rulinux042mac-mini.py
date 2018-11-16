from glob import glob
import shutil, os, argparse
import subprocess
import time, re
import numpy as np

#RN is a wildcard for run run number
track_file_template = 'RunRN_CMSTiming_converted.root'

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('-R', '--runs', type=int, nargs='+', default=None)

    p.add_argument('--dir_rulinux04', type=str, default='/data/TestBeam/2018_11_November_CMSTiming/CMSTimingConverted/')
    p.add_argument('--dir_data', type=str, default='../data/')

    p.add_argument('--max_void', type=int, default=-1)
    p.add_argument('--sleep', type=float, default=60)
    p.add_argument('--min_file_age', type=float, default=90)

    p.add_argument('-v', '--verbose', action='store_true', default=False)

    return p.parse_args()

def transfer(args, rn):
    print '============= Processing run ' + str(rn) + ' ============='

    cmd = 'rsync -artu --progress otsdaq@rulinux04.dhcp.fnal.gov:'
    cmd += args.dir_rulinux04
    cmd += track_file_template.replace('RN', str(rn))
    cmd += ' ' + args.dir_data + 'Tracks/'

    subprocess.call(cmd, shell=True)

def get_last_remote_file(args):
    cmd = "ssh otsdaq@rulinux04.dhcp.fnal.gov \'cd " + args.dir_rulinux04 + "; "
    cmd += "ls -1ltr " + track_file_template.replace('RN', '*') + "\'"

    output = subprocess.check_output(cmd, shell=True)
    last_file = output.split('\n')[-2]
    last_file = last_file.split(' ')
    fn = last_file[-1]

    size = int(last_file[-5])
    run_number = re.search('Run[0-9]+_', fn).group(0)[3:-1]
    return int(run_number), size

if __name__ == '__main__':
    args = GetCommandLineArgs()

    if not args.dir_rulinux04.endswith('/'):
        args.dir_rulinux04 += '/'
    if not args.dir_data.endswith('/'):
        args.dir_data += '/'

    if args.runs==None:
        last_run_number = 0
        nothing_changed = 0

        while(args.max_void < 0 or nothing_changed < args.max_void):
            print "Loop start"

            r_pre, s_pre = get_last_remote_file(args)
            time.sleep(10)
            r_post, s_post = get_last_remote_file(args)

            while (r_pre == r_post and not s_pre == s_post) or (r_pre < r_post):
                print "File from run", r_post, "still changing"
                r_pre, s_pre = get_last_remote_file(args)
                time.sleep(10)
                r_post, s_post = get_last_remote_file(args)

            transfer(args, '*')

            files = args.dir_data + 'Tracks/' + track_file_template.replace('RN', '*')
            files = glob(files)
            if len(files) == 0:
                print 'No run presents.... move one manually'
                exit(0)

            rlist = map(lambda x: int(re.search('Run[0-9]+', x).group(0)[3:]), files)
            run_number = np.max(rlist)


            print 'Lastest run present', run_number

            if last_run_number != run_number:
                last_run_number = run_number
            else:
                nothing_changed += 1

            print '...Going to sleep for {0:.0f} s\n'.format(args.sleep)
            time.sleep(args.sleep)

        print '\n\nStopped because nothing changed for at least {0:.1f} min'.format(args.sleep*nothing_changed/60.0)

    else:
        if len(args.runs) == 2 and (args.runs[0] < args.runs[1]):
            runs_list = range(args.runs[0], args.runs[1]+1)

        for rn in args.runs:
            transfer(args, rn)
