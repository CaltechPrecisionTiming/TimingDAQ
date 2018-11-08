#! /usr/bin/env python
import os, sys, time
import subprocess, argparse

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('--config_vme', default='FNAL_TestBeam_1811/VME_DQM.config')
    p.add_argument('--config_scope', default='NONE') #FNAL_TestBeam_1811/NetScope_DQM.config
    p.add_argument('--max_attempts', default=120) #FNAL_TestBeam_1811/NetScope_DQM.config

    return p.parse_args()

if __name__ == '__main__':
    args = GetCommandLineArgs()

    daq_dir = '/data/TestBeam/2018_11_November_CMSTiming/'
    data_dir = '/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data'

    if (args.config_scope=='NONE'):
        print "DQM::INFO: Running DQM without NetScope!"
    if (args.config_vme=='NONE'):
        print "DQM::INFO: Running DQM without VME!"
    if (args.config_scope=='NONE' and args.config_vme=='NONE'):
        sys.exit('DQM::ERROR: No config file given. Exit.')

    processing_run = 0

    attempt = 0
    while (True):
        cmd = "rsync -art otsdaq@ftbf-daq-08.fnal.gov:~/otsdaq/srcs/otsdaq_cmstiming/dqm/last_run.log ."
        out = subprocess.call(cmd, shell=True)

        new_run = -1
        with open("last_run.log") as frun:
            new_run = int(frun.readline())

        if new_run<processing_run:
            print "DQM::ERROR: Run number from last_run.log is ",new_run,
            print " which is earlier than the run being currently processed:", processing_run
            sys.exit(1)
        elif (new_run==processing_run):
            print "DQM::INFO: No new runs available. Please wait..."
            attempt +=1
            if (attempt>=args.max_attempts):
                print "DQM::ERROR: Reached max number of attempts to get a new run. Exit."
                sys.exit(0)
        else:
            attempt = 0
            processing_run = new_run

            cmd = 'nohup python automation/prompt_reco.py {0}'.format(new_run)
            if (args.config_scope=='NONE'):
                cmd += ' --no_scope'
            else:
                cmd += ' --config_scope={0}'.format(args.config_scope)
            if (args.config_vme=='NONE'):
                cmd += ' --no_vme'
            else:
                cmd += ' --config_vme={0}'.format(args.config_vme)
            cmd += ' --daq_dir={0}'.format(daq_dir)
            cmd += ' --data_dir={0}'.format(data_dir)
            cmd += " >{0}/DQM/dqm_log_run{1}.log  2>&1 &".format(data_dir, new_run)
            print 'Starting data processing and DQM as a background process on LPC cluster...'
            print '\033[92m'+cmd+'\033[0m'
            out = subprocess.call(cmd, shell=True)
            if out: print 'DQM::ERROR: Failed, exit code:', out

        time.sleep(60)
