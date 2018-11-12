#! /usr/bin/env python
import os, sys, argparse, subprocess, glob, re, time
import collections
from ROOT import *
from array import array

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('-r', '--runs', type=int, nargs='+', default=-1)
    p.add_argument('--min_amp', type=int, default=15)

    p.add_argument('-e', '--event',type=int, default=-1)

    return p.parse_args()

def natural_keys(text):
    return int(re.split('(\d+)', text)[1])

if __name__ == '__main__':
    args = GetCommandLineArgs()

    gStyle.SetOptStat(1111)

    trig_chan = 0

    data_dir = '/Users/ana/mtd/testbeam/data/scope/RECO'

    runs_list = []
    if len(args.runs) == 2:
        runs_list = range(args.runs[0], args.runs[1]+1)
    else:
        runs_list = args.runs

    pulse = TChain('pulse')
    for run in runs_list:
        pulse.Add(data_dir+'/run_scope{}_converted_full.root'.format(run))

    #--------------------------------------------------------
    #          Amplitudes for all channels
    #--------------------------------------------------------
    chan_dict = collections.OrderedDict()
    chan_dict[0] = 'ch 0: Photek 1'
    chan_dict[1] = 'ch 1: SP W4 pre-rad' # red 
    chan_dict[2] = 'ch 2: SP W5 8e14'  # green
    chan_dict[3] = 'ch 3: SP W5 1.5e15' # blue
    
    can_amp = TCanvas('amplitudes', 'amplitudes', 2000,500)
    can_amp.Divide(4,1)
    
    #--------------------------------------------------------
    #      Channels to plot waveforms
    #--------------------------------------------------------
    can_wfs = TCanvas('waveforms', 'waveforms', 2000,1000)
    colors = [kBlack, kRed+1, kGreen+1, kAzure+1, kOrange] 
    ngr_max = 15
    can_wfs.Divide(5,3)    

    res_chans = [1,2,3]
    can_res = TCanvas('resolution')
    can_res.Divide(3,1)


    #--------------------------------------------------------
    #          Amplitudes for all channels
    #--------------------------------------------------------    
    hamp = []
    for ch in range(0,4):
        can_amp.cd(ch+1)
        hamp.append(TH1D('amp_ch{}'.format(ch),
                         'Ch '+str(ch)+': '+chan_dict[ch]+'; Amplitude [mV]'.format(ch),50,0,100))
        pulse.Project(hamp[ch].GetName(), 'amp['+str(ch)+']',
                      'amp['+str(ch)+']>'+str(args.min_amp))
        hamp[ch].SetLineColor(kRed)
        hamp[ch].SetLineWidth(2)
        hamp[ch].DrawCopy()
    can_amp.Update()

    #--------------------------------------------------------
    #         Time resolution for select channels
    #--------------------------------------------------------
    hres = []
    idut = 0
    for ch in res_chans: 
        hres_name = 'res_ch{}'.format(ch)
        hres.append(TH1D(hres_name, 'Ch '+str(ch)+': '+chan_dict[ch]+'; LP2_20 #Delta t [ns]',300,1,10))
        pulse.Project(hres_name, '(LP2_20['+str(ch)+']-LP2_20['+str(trig_chan)+'])*1.0e9', 
                      'amp['+str(trig_chan)+']>30 && amp['+str(trig_chan)+'] < 50 && amp['+str(ch)+']>25 && amp['+str(ch)+']<60')
        
        hres[idut].SetLineColor(kGreen+1)
        hres[idut].SetLineWidth(2)
        can_res.cd(idut+1)
        hres[idut].DrawCopy()
        hres[idut].Fit("gaus")
        can_res.Update()
        idut +=1 

    #--------------------------------------------------------
    #          Waveforms for select channels
    #--------------------------------------------------------
    ipass = 0
    hwf = []
    for ent in pulse:
        if (args.event!=-1 and ent.i_evt!=args.event): continue
        for ch in range(1,4):
            if ent.amp[ch] > args.min_amp:
                hwf.append([])
                for jch in range(4):
                    wf = array( 'f', [0.]*1000)
                    time = array( 'f', [0.]*1000)
                    for ipt in range(0,1000):
                        wf[ipt] = ent.channel[1000*jch+ipt]
                        time[ipt] = ent.time[ipt]
                    hwf[ipass].append(TGraph(1000, time, wf))
                    hwf[ipass][jch].SetMarkerColor(jch+1)
                    hwf[ipass][jch].SetLineColor(jch+1)
                    hwf[ipass][jch].SetLineWidth(2)
                    if (jch==0): 
                        if (args.event==-1): can_wfs.cd(ipass+1)
                        else: can_wfs.cd()
                        hwf[ipass][jch].Draw('ALP')
                        hwf[ipass][jch].GetYaxis().SetRangeUser(-200,50)
                        hwf[ipass][jch].SetTitle('Event number:'+str(ent.i_evt))
                    else: 
                        if (args.event==-1): can_wfs.cd(ipass+1)
                        else: can_wfs.cd()
                        hwf[ipass][jch].Draw('LP')
                ipass += 1
                continue
        if (ipass==ngr_max): break
    can_wfs.Update()


    raw_input('Press enter to exit.')

