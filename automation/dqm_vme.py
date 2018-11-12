#! /usr/bin/env python
import os, sys, argparse, subprocess, glob, re, time
import collections
from ROOT import *
from array import array

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('-r', '--runs', type=int, nargs='+', default=-1)
    p.add_argument('--min_amp', type=int, default=15)

    p.add_argument('--resolution', action='store_true')
    p.add_argument('-e', '--event',type=int, default=-1)

    p.add_argument('-q', '--quick', action='store_true')
    p.add_argument('-a', '--all', action='store_true')

    return p.parse_args()

def natural_keys(text):
    return int(re.split('(\d+)', text)[1])

def isGoodEvent(ent):
    if ent.ntracks==1:
        return True
    return False


if __name__ == '__main__':
    args = GetCommandLineArgs()

    # ----------- Config ------------
    baseline = 'ntracks==1'


    runs_list = []
    tag = ''
    if len(args.runs) == 2:
        runs_list = range(args.runs[0], args.runs[1]+1)
        print runs_list
        tag = 'Runs_'+str(args.runs[0])+'-'+str(args.runs[1])
    else:
        runs_list = args.runs
        tag = 'Runs_'+'-'.join([str(i) for i in args.runs])

    vme_reco_dir = '/eos/cms/store/group/phys_susy/razor/FNAL_TB_1811/data/VME/RECO/'
    if ('/Users/ana/mtd/' in os.environ['PWD']):
        vme_reco_dir = '/Users/ana/mtd/testbeam/data/'
    elif ('cmslpc' in os.environ['HOSTNAME']):
        vme_reco_dir = '/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/VME/RECO/'

    if (args.resolution):
        vme_reco_dir += 'v2/'
    else:
        vme_reco_dir += 'vf1/'

    pulse = TChain('pulse')
    for run in runs_list:
        fname = vme_reco_dir+'/DataVMETiming_Run{}.root'.format(run)
        if os.path.exists(fname):
            if (os.path.getsize(fname)>100e6 and (time.time()-os.path.getmtime(fname))>30):
                pulse.Add(fname)

    print "Total number of entries:", pulse.GetEntries()
    print "Total number of entries with tracks:", pulse.GetEntries("ntracks==1")

    #--------------------------------------------------------
    #          Amplitudes for all channels
    #--------------------------------------------------------
    chan_list = collections.OrderedDict()
    chan_list[0] = 'ch 0: Photek 1'
    chan_list[1] = 'ch 1: BTL 1: 8x8 raw/2'
    chan_list[2] = 'ch 2: BTL 1: 8x8 raw*X'
    chan_list[3] = 'ch 3: BTL 2: 8x8 raw/2'
    chan_list[4] = 'ch 4: BTL 2: 8x8 raw*X'
    chan_list[5] = 'ch 5: BTL 3: 5x12 raw'
    # chan_list[6] = 'ch 6: BTL 4: 8x8 raw'
    # chan_list[7] = 'ch 7: BTL 5: 8x8 raw'
    # chan_list[8] = 'ch 8: Trigger'

    # chan_list[9] =  'ch 9:  Photek 1'
    # chan_list[10] = 'ch 10: ---'
    # chan_list[11] = 'ch 11: ---'
    # chan_list[12] = 'ch 12: ---'
    # chan_list[13] = 'ch 13: ---'
    # chan_list[14] = 'ch 14: ---'
    # chan_list[15] = 'ch 15: ---'
    # chan_list[16] = 'ch 16: ---'
    # chan_list[17] = 'ch 17: Trigger'

    chan_list[18] = 'ch 18: Photek 1'
    chan_list[19] = 'ch 19: HPK OR trigger'
    chan_list[20] = 'ch 20: SP W4 pre-rad'
    chan_list[21] = 'ch 21: SP W5 8e14'
    chan_list[22] = 'ch 22: SP W5 1.5e15'
    chan_list[23] = 'ch 23: LP W5 pre-rad'
    # chan_list[24] = 'ch 24: ---'
    # chan_list[25] = 'ch 25: ---'
    # chan_list[26] = 'ch 26: Trigger'

    # chan_list[27] = 'ch 27: Photek 1'
    # chan_list[28] = 'ch 28: ---'
    # chan_list[29] = 'ch 29: 2x2 cm scint.'
    # chan_list[30] = 'ch 30: ---'
    # chan_list[31] = 'ch 31: ---'
    # chan_list[32] = 'ch 32: ---'
    # chan_list[33] = 'ch 33: ---'
    # chan_list[34] = 'ch 34: ---'
    # chan_list[35] = 'ch 35: Trigger'

    can_amp = TCanvas('amp', 'amp', 1600,1200)
    # can_amp.Divide(8,4)
    can_amp.Divide(4,3)
    
    #--------------------------------------------------------
    #      Grouping channels into devices
    #--------------------------------------------------------
    dut_dict = collections.OrderedDict()
    dut_dict['Ch 1: BTL 1: 8x8'] = [1]
    dut_dict['Ch 3: BTL 2: 8x8'] = [3]
    dut_dict['Ch 5: BTL 3: 5x12'] = [5]
    dut_dict['Ch 6: BTL 4: 8x8'] = [6]
    dut_dict['Ch 7: BTL 5: 8x8'] = [7]
    dut_dict['Ch 19: HPK OR trigger'] = [19]
    dut_dict['Ch 20: SP W4 pre-rad'] = [20]
    dut_dict['Ch 21: SP W5 8e14'] = [21]
    dut_dict['Ch 22: SP W5 1.5e15'] = [22]
    dut_dict['ch 23: LP W5'] = [23]
    dut_dict['Ch 29: 2x2 cm trigger'] = [29]

    can_xy = TCanvas("xy", 'xy', 1600,1200)
    can_xy.Divide(4,3)

    #--------------------------------------------------------
    #      Channels to plot waveforms
    #--------------------------------------------------------
    if (not args.quick):
        can_wfs = TCanvas('waves', 'waves', 2000,1000)
        colors = [kBlack, kRed+1, kGreen+1, kAzure+1, kOrange] 
        wf_chans = [20, 21, 22, 23]
        # wf_chans = [23, 24, 25]
        ngr_max = 15
        can_wfs.Divide(5,3)

    #--------------------------------------------------------
    #      Channels to determine time resol
    #--------------------------------------------------------
    dut_resol_dict = collections.OrderedDict()
    dut_resol_dict['Ch 19: ETL SP W1'] = [19]
    dut_resol_dict['Ch 20: ETL SP W4'] = [20]
    dut_resol_dict['Ch 21: ETL LP W5'] = [21]
    
    if (args.resolution):
        can_res = TCanvas('resol', 'resol', 2000,1000)
        can_res.Divide(3,1)

    #--------------------------------------------------------
    #          Locations of the various DUTs
    # --------------------------------------------------------
    can_xy.cd(1)

    hxy_all = TH2D('xy_all','XY, all events; X [mm]; Y [mm]',50,0,50,50,0,50)
    pulse.Project(hxy_all.GetName(), 'yIntercept:xIntercept', baseline)
    hxy_all.DrawCopy("COLZ")
    tot_trigs = hxy_all.GetEntries()
    print 'Total triggers:', tot_trigs

    hxy_amp = []
    ich = 0
    for ch,name in chan_list.iteritems():
        can_xy.cd(ich+1)
        hxy_amp.append(TH2D('xy_amp100_ch{}'.format(ch),
                            name+', amp>'+str(args.min_amp)+'; X [mm]; Y [mm]',
                            50,0,50,50,0,50))
        pulse.Project(hxy_amp[ich].GetName(), 'yIntercept:xIntercept',
                      baseline+'&& amp['+str(ch)+']>'+str(args.min_amp))
        if (hxy_amp[ich].GetEntries()):
            ch_trigs = hxy_amp[ich].GetEntries()
            print 'Ch', ich, 'triggers:', ch_trigs,'eff = ', float(ch_trigs)/tot_trigs 
        hxy_amp[ich].DrawCopy('COLZ')
        ich+=1
    can_xy.Update()    

    #--------------------------------------------------------
    #          Amplitudes for all channels
    #--------------------------------------------------------    
    hamp = []
    ich =0 
    for ch,name in chan_list.iteritems():
        can_amp.cd(ich+1)
        gPad.SetLogy()
        hamp.append(TH1D('amp_ch{}'.format(ich),
                         name+'; Amplitude [mV]',50,0,300))
        pulse.Project(hamp[ich].GetName(), 'amp['+str(ch)+']', baseline)
        hamp[ich].SetLineColor(kRed)
        hamp[ich].SetLineWidth(2)
        hamp[ich].DrawCopy()
        ich+=1
    can_amp.Update()
    # can_amp.SaveAs(tag+"_amp.root")

    #--------------------------------------------------------
    #          Waveforms for select channels
    #--------------------------------------------------------
    if (not args.quick):
        ipass = 0
        hwf = []
        for ent in pulse:
            ievt = ent.i_evt
            if (args.event!=-1 and ievt!=args.event): continue
            for ch in wf_chans:
                if ent.amp[ch] > args.min_amp and isGoodEvent(ent):
                    hwf.append([])
                    for jch in range(len(wf_chans)):
                        wf = array( 'f', [0.]*1024)
                        time = array( 'f', [0.]*1024)
                        for ipt in range(0,1024):
                            wf[ipt] = ent.channel[1024*wf_chans[jch]+ipt]
                            time[ipt] = ent.time[ipt]
                        hwf[ipass].append(TGraph(1024, time, wf))
                        hwf[ipass][jch].SetMarkerColor(jch+1)
                        hwf[ipass][jch].SetLineColor(jch+1)
                        hwf[ipass][jch].SetLineWidth(2)
                        if (jch==0): 
                            if (args.event==-1): can_wfs.cd(ipass+1)
                            else: can_wfs.cd()
                            hwf[ipass][jch].Draw('ALP')
                            hwf[ipass][jch].GetYaxis().SetRangeUser(-ent.amp[ch]-10,50)
                            hwf[ipass][jch].SetTitle('Event number:'+str(ievt))
                        else: 
                            if (args.event==-1): can_wfs.cd(ipass+1)
                            else: can_wfs.cd()
                            hwf[ipass][jch].Draw('LP')
                    ipass += 1
                    continue
            if (ipass==ngr_max): break
        can_wfs.Update()
        # can_wfs.SaveAs(tag+"_wfs.root")

    #--------------------------------------------------------
    #         Time resolution for select channels
    #--------------------------------------------------------
    if (args.resolution):
        hres = []
        idut = 0
        for name, ch in dut_resol_dict.iteritems():
            can_res.cd(idut+1)
            hres.append(TH1D('t0_20_ch{}'.format(ch),
                               'Ch {}: '.format(ch)+name+'; Time t0 [ns]'.format(ch),50,0,300))
            pulse.Project(hres[idut].GetName(), 't0_20['+str(ch)+']', baseline)
            # hres[idut].Fit("gaus")
            idut +=1 
        # can_res.SaveAs(tag+'_res.root')

    raw_input('Press enter to exit.')

