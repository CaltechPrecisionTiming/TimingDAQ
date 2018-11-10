import os, sys, argparse, subprocess, glob
import ROOT as rt
import numpy as np

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('runs', type=int, nargs='+')
    p.add_argument('--no_tracks', action='store_true', default=False)
    p.add_argument('--no_Dat2Root', action='store_true')
    p.add_argument('--no_VME', action='store_true')
    p.add_argument('--no_NetScope', action='store_true')
    p.add_argument('-f','--force', action='store_true')

    p.add_argument('--daq_dir', default='/data/TestBeam/2018_11_November_CMSTiming')
    p.add_argument('--NimPlus_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/NimPlus')
    p.add_argument('--VME_raw_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/VME/RAW')
    p.add_argument('--NetScope_raw_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/NetScope/RAW')
    p.add_argument('--track_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/Tracks')

    p.add_argument('--VME_root_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/VME/RECO/v1')
    p.add_argument('--NetScope_root_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/NetScope/RECO/v0')
    p.add_argument('--code_dir', default=os.environ['PWD'])

    p.add_argument('--config_VME', default='FNAL_TestBeam_1811/VME_DQM_v1.config')
    p.add_argument('--config_NetScope', default='FNAL_TestBeam_1811/NetScope_DQM.config')
    p.add_argument('--NO_save_meas', default=False, action='store_true')
    p.add_argument('-N', '--N_evts', type=str, default='0')
    p.add_argument('--draw_debug_pulses', default=False, action='store_true')

    p.add_argument('-v', '--verbose', default=False, action='store_true')
    p.add_argument('--N_max_job', type=int, default=1000)

    p.add_argument('--N_skip', type=int, nargs='+')

    return p.parse_args()

if __name__ == '__main__':
    args = GetCommandLineArgs()

    runs_list = []
    if len(args.runs) == 2:
        runs_list = range(args.runs[0], args.runs[1]+1)
    else:
        runs_list = args.runs

    if not args.no_VME:
        print 'Processing VME'
        for run in runs_list:
            print '========================== Processing Run {} =========================='.format(run)

            print 'Getting NimPlus triggers'
            NimPlus_file = args.NimPlus_dir + '/TriggerCountNimPlusX_{}.cnt'.format(run)

            if not os.path.exists(NimPlus_file):
                cmd = 'rsync -art otsdaq@ftbf-daq-08.fnal.gov:{}/NimPlus/TriggerCountNimPlusX_{}.cnt {}'.format(args.daq_dir, run, NimPlus_file)
                out = subprocess.call(cmd, shell=True)
                if out:
                    print '[WARNING] NimPlus cnt file copy failed'

            N_expected_evts = -1
            if os.path.exists(NimPlus_file):
                cmd = 'more {} | grep sig_cms1 | awk \'{{print $3}}\''.format(NimPlus_file)
                proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
                (out, err) = proc.communicate()
                N_expected_evts = int(out)
                print 'Number of trigger expected', out
            else:
                print '[WARNING] NO NimPlus file present in ' + args.NimPlus_dir

            raw_filename = args.VME_raw_dir + '/RawDataVMETiming_Run{}.dat'.format(run)
            if not os.path.exists(raw_filename) or args.force:
                print '\nCreating the VME file: ', raw_filename
                cmd = 'rsync -artv otsdaq@ftbf-daq-08.fnal.gov:{}/CMSTiming/RawDataSaver0CMSVMETiming_Run{}_*_Raw.dat {}/'.format(args.daq_dir, run, args.VME_raw_dir)
                subprocess.call(cmd, shell=True)

                copied_files = glob.glob('{}/RawDataSaver0CMSVMETiming_Run{}_*_Raw.dat'.format(args.VME_raw_dir, run))

                if len(copied_files) == 0:
                    sys.exit('[ERROR] Unable to find files like: ' +  '{}/RawDataSaver0CMSVMETiming_Run{}_*_Raw.dat'.format(args.VME_raw_dir, run))
                elif len(copied_files) > 1:
                    cmd = 'cat ' + ' '.join(copied_files) + ' > ' +raw_filename
                    # print cmd
                    subprocess.call(cmd, shell=True)
                    for f in copied_files:
                        os.remove(f)
                else:
                    cmd = 'mv '+ copied_files[0] + ' ' + raw_filename
                    subprocess.call(cmd, shell=True)
            else:
                print '\nVME file found: ', raw_filename

            root_filename = args.VME_root_dir + '/DataVMETiming_Run{}.root'.format(run)
            if args.no_Dat2Root:
                print '[INFO] No Dat2Root flag active'
                continue
            if os.path.exists(root_filename) and not args.force:
                print root_filename, 'already present'
                continue

            cmd_Dat2Root = args.code_dir + '/VMEDat2Root'
            cmd_Dat2Root += ' --input_file=' + raw_filename
            cmd_Dat2Root += ' --config=' + args.code_dir + '/config/' + args.config_VME
            if args.draw_debug_pulses:
                cmd_Dat2Root += ' --draw_debug_pulses'
            if args.verbose:
                cmd_Dat2Root += ' --verbose'
            if not args.NO_save_meas:
                cmd_Dat2Root += ' --save_meas'
            if not args.no_tracks:
                tracks_filename = args.track_dir + '/Run{}_CMSTiming_converted.root'.format(run)

                if os.path.exists(tracks_filename):
                    cmd_Dat2Root += ' --pixel_input_file=' + tracks_filename
                else:
                    print '[ERROR] Tracks file not found in', tracks_filename
                    print 'If you want to run  without tracks use <--no_tracks>.'
                    continue
            if not args.N_skip is None:
                for i,n in enumerate(args.N_skip):
                    cmd_Dat2Root += ' --NSkip{}={}'.format(i+1,n)

            N_tot = max(int(args.N_evts), N_expected_evts)
            nj = 1 + N_tot/args.N_max_job
            evt_start_list = np.arange(0,1,1)
            if (N_tot>0):
                evt_start_list = np.arange(0, N_tot, N_tot/float(nj))
                evt_start_list = np.uint32(np.ceil(evt_start_list))

            if evt_start_list.shape[0] == 1:
                cmd_Dat2Root += ' --N_evt_expected=' + str(N_expected_evts)
                cmd_Dat2Root += ' --output_file=' + root_filename
                cmd_Dat2Root += ' --N_evts=' + args.N_evts
                print '\n'+cmd_Dat2Root
                subprocess.call(cmd_Dat2Root, shell=True)
            else:
                print 'Dividing the run into', evt_start_list.shape[0], 'jobs'
                outfile_list = []
                for i in range(evt_start_list.shape[0]):
                    print '\n\n ----------> Job {}/{}\n'.format(i+1, evt_start_list.shape[0])
                    aux_name = root_filename.replace('.root', '_{}.root'.format(i))
                    outfile_list.append(aux_name)

                    aux_cmd = cmd_Dat2Root + ' --output_file=' + aux_name
                    aux_cmd += ' --start_evt=' + str(evt_start_list[i])

                    N_stop = 0
                    if i == evt_start_list.shape[0]-1:
                            N_stop = int(args.N_evts)
                    else:
                        N_stop = evt_start_list[i+1]
                    aux_cmd += ' --N_evts=' + str(N_stop)

                    subprocess.call(aux_cmd, shell=True)

                cmd = 'hadd ' + root_filename + ' ' + ' '.join(outfile_list)
                subprocess.call(cmd, shell=True)
                subprocess.call('rm '+' '.join(outfile_list), shell=True)

                f = rt.TFile.Open(root_filename, 'READ')
                t = f.Get('pulse')
                t.GetEntry(t.GetEntries()-1)
                N_evts_tree =  t.i_evt+1
                f.Close()

                if N_expected_evts != -1 and N_evts_tree != N_expected_evts:
                    print '\n\n\n[ERROR] Number of evts not matching the expected number'
                    print '============= ', N_evts_tree, '!=', N_expected_evts, ' ============'
                    print 'Moving to the currupted directory\n\n\n'
                    corrupted_name = os.path.dirname(root_filename) + '/corrupted/' + os.path.basename(root_filename)
                    subprocess.call('mv ' + root_filename + ' ' + corrupted_name, shell=True)
                elif N_expected_evts == -1:
                    print '[WARNING] Event number matching between trigger and pulse tree not performed'
                else:
                    print '\n\n[INFO] !!!!!! Number of events matching !!!!!\n\n'


            print 'Finished processing run ', run

    if not args.no_NetScope:
        print 'Processing NetScope'
        for run in runs_list:
            print '========================== Processing Run {} =========================='.format(run)

            raw_filename = args.NetScope_raw_dir + '/RawDataNetScope_Run{}.dat'.format(run)
            if not os.path.exists(raw_filename) or args.force:
                print '\nCreating the NetScope file: ', raw_filename
                cmd = 'rsync -artv otsdaq@ftbf-daq-08.fnal.gov:{}/NetScopeTiming/RawDataSaver0NetScope_Run{}_*_Raw.dat {}'.format(args.daq_dir, run, raw_filename)
                subprocess.call(cmd, shell=True)
            else:
                print '\nNetScope file found: ', raw_filename

            root_filename = args.NetScope_root_dir + '/DataNetScope_Run{}.root'.format(run)
            if args.no_Dat2Root:
                print '[INFO] No Dat2Root flag active'
                continue
            if os.path.exists(root_filename) and not args.force:
                print root_filename, 'already present'
                continue

            cmd_Dat2Root = args.code_dir + '/NetScopeDat2Root'
            cmd_Dat2Root += ' --input_file=' + raw_filename
            cmd_Dat2Root += ' --config=' + args.code_dir + '/config/' + args.config_NetScope
            cmd_Dat2Root += ' --output_file=' + root_filename
            cmd_Dat2Root += ' --N_evts=' + args.N_evts
            if args.draw_debug_pulses:
                cmd_Dat2Root += ' --draw_debug_pulses'
            if not args.NO_save_meas:
                cmd_Dat2Root += ' --save_meas'

            print '\n'+cmd_Dat2Root

            subprocess.call(cmd_Dat2Root, shell=True)

            print 'Finished processing run ', run
