#! /usr/bin/env python
import os, sys, argparse, subprocess

class clrs:
    blue = '\033[94m'
    green = '\033[92m'
    yellow = '\033[93m'
    red = '\033[91m'
    end = '\033[0m'

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('run', type=int)
    p.add_argument('--no_vme', action='store_true')
    p.add_argument('--config_vme', default='')
    p.add_argument('--no_scope', action='store_true')
    p.add_argument('--config_scope', default='')
    p.add_argument('--daq_dir', default='/data/TestBeam/2018_11_November_CMSTiming/')
    p.add_argument('--data_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/')
    return p.parse_args()

if __name__ == '__main__':
    args = GetCommandLineArgs()

    nim_dir = args.data_dir+'/NimPlus/'
    vme_raw_dir = args.data_dir+'/VME/RAW/'
    scope_raw_dir = args.data_dir+'/NetScope/RAW/'
    track_dir = args.data_dir+'/Tracks/'
    dqm_dir = args.data_dir+'/DQM/'
    vme_root_dir = args.data_dir+'/VME/RECO/v0/'
    scope_root_dir = args.data_dir+'/NetScope/RECO/v0/'

    # run track reconstruction and get output to track_dir
    if (not args.no_vme): 
        print 'DQM::INFO: Running track reconstruction over SSH...'
        cmd = "ssh otsdaq@rulinux04.dhcp.fnal.gov 'cd CMSTiming && ./HyperScript.sh {0}'".format(args.run)
        print clrs.green+cmd+clrs.end
        sys.stdout.flush()
        out = subprocess.call(cmd, shell=True) #bbtl
        if out: 'DQM::ERROR: SSH command failed, exit code:', out
        sys.stdout.flush()

        print 'DQM::INFO: Copying reconstructing tracks from rulinux04'
        sys.stdout.flush()
        cmd = 'rsync -art otsdaq@rulinux04.dhcp.fnal.gov:{0}/CMSTimingConverted/Run{1}_CMSTiming_converted.root {2}'.format(args.daq_dir, args.run, track_dir)
        print clrs.green+cmd+clrs.end
        sys.stdout.flush()
        out = subprocess.call(cmd, shell=True)
        if out: print 'DQM::ERROR: Tracks file copy failed, exit code', out
        out = subprocess.call('ls -l {0}/Run{1}*.root'.format(track_dir, args.run), shell=True)
        sys.stdout.flush()

    # format data decoding command
    out = subprocess.call("./setup_cmslpc.sh", shell=True)

    cmd = 'python automation/DecodeData.py {0} \\\n'.format(args.run)
    cmd += ' --daq_dir={0}\\\n'.format(args.daq_dir)
    cmd += ' --track_dir={0}\\\n'.format(track_dir)
    cmd += ' --NimPlus_dir={0}\\\n'.format(nim_dir)    
    cmd += ' --VME_raw_dir={0}\\\n'.format(vme_raw_dir)
    cmd += ' --NetScope_raw_dir={0}\\\n'.format(scope_raw_dir)
    cmd += ' --VME_root_dir={0}\\\n'.format(vme_root_dir)
    cmd += ' --NetScope_root_dir={0}\\\n'.format(scope_root_dir)
    # cmd += ' -N 100\\\n'
    if (args.no_vme): cmd += ' --no_VME --config_NetScope={0} --NetScope_root_dir={1}\\\n'.format(args.config_scope, scope_root_dir)
    if (args.no_scope): cmd += ' --no_NetScope --config_VME={0} --VME_root_dir={1}\\\n'.format(args.config_vme, vme_root_dir)
    cmd += '\n'
    print 'DQM::INFO: Copy raw data and decode. Command:\n', clrs.green+cmd+clrs.end, '\nRunning...'
    sys.stdout.flush()
    out = subprocess.call(cmd, shell=True) #bbtl
    if out: print 'DQM::ERROR: Failed, exit code:', out
    sys.stdout.flush()

    print 'Processing run {0} finished.'.format(args.run)


