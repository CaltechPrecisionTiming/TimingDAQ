#! /usr/bin/env python
import os, sys, argparse, subprocess, glob, re
from ROOT import *

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('run', type=int)
    p.add_argument('--vme_root_dir', default='/eos/uscms/store/user/cmstestbeam/BTL_ETL/2018_11/data/VME/RECO/v0/')
    return p.parse_args()

def natural_keys(text):
    return int(re.split('(\d+)', text)[1])

if __name__ == '__main__':
    args = GetCommandLineArgs()
    
    pulse = TChain('pulse')
    pulse.Add(args.vme_root_dir+'/DataVME_Run{}.root'.format(args.run))
    
    hxy_all = TH2D('xy_all','XY, all events; X [mm]; Y [mm]',50,0,50,50,0,50)
    pulse.Project(hxy_all.GetName(), 'xIntercept:yIntercept')

    #--------------------------------------------------------
    #          Locations of the various DUTs
    #--------------------------------------------------------
    hxy_amp50 = []
    for ich in range(0,32):
        hxy_amp50.append(TH2D('xy_amp100_ch{}'.format(ich),'Ch {}: XY, amp > 50; X [mm]; Y [mm]'.format(ich),50,0,50,50,0,50))
        pulse.Project(hxy_amp50[ich].GetName(), 'xIntercept:yIntercept','amp['+str(ich)+']>50')

    # XY plots
    pane_map = {}
    pane_map['G0: BTL 1: 8x8'] = [1]
    pane_map['G0: BTL 2: 8x8'] = [3]
    pane_map['G0: BTL 3: 5x12'] = [5]
    pane_map['G0: BTL 4: 8x8'] = [6]
    pane_map['G0: BTL 5: 8x8'] = [7]
    pane_map['G1: BTL bar 1'] = [10,11] 
    pane_map['G1: BTL bar 2'] = [12,13] 
    pane_map['G1: 2x2 cm trigger'] = [14]
    pane_map['G1: ETL LP W7'] = [15]
    pane_map['G2: ETL SP W4'] = [17]
    pane_map['G2: ETL SP W1'] = [18]
    pane_map['G2: ETL LP W5'] = [19]
    pane_map['G2: ETL HPK'] = range(20,24)
    pane_map['G3: ETL LP W7'] = range(24,32)
    
    can_xy = TCanvas("xy")
    can_xy.Divide(5, 3)

    can_xy.cd(1)
    hxy_all.DrawCopy("COLZ")

    hxy_dut = []
    ip = 2
    for dut, ch in pane_map.iteritems():
        ip += 1
        can_xy.cd(ip)
        hxy_amp50[ch[0]].SetTitle(dut)
        if len(ch)>1:
            for i in range(1, len(ch)):
                hxy_amp50[ch[0]].Add(hxy_amp50[ch[i]])
        hxy_amp50[ch[0]].DrawCopy('COLZ')
    can_xy.Update()    

    #--------------------------------------------------------
    #          Amplitudes for all channels
    #--------------------------------------------------------
    chan_list = [None] * 32
    chan_list[0] = 'G0: Photek 1'
    chan_list[1] = 'G0: BTL 1: 8x8 raw/2'
    chan_list[2] = 'G0: BTL 1: 8x8 raw*X'
    chan_list[3] = 'G0: BTL 2: 8x8 raw/2'
    chan_list[4] = 'G0: BTL 2: 8x8 raw*X'
    chan_list[5] = 'G0: BTL 3: 5x12 raw'
    chan_list[6] = 'G0: BTL 4: 8x8 raw'
    chan_list[7] = 'G0: BTL 5: 8x8 raw'
    chan_list[8] = 'G1: Photek 1'
    chan_list[9] = 'G1: Photek 2'
    chan_list[10] = 'G1: BTL bar 1: ch0'
    chan_list[11] = 'G1: BTL bar 1: ch1'
    chan_list[12] = 'G1: BTL bar 2: ch0'
    chan_list[13] = 'G1: BTL bar 2: ch1'
    chan_list[14] = 'G1: 2x2 cm trigger'
    chan_list[15] = 'G1: ETL LP W7 ch0'
    chan_list[16] = 'G2: Photek 1'
    chan_list[17] = 'G2: ETL SP W4'
    chan_list[18] = 'G2: ETL SP W1'
    chan_list[19] = 'G2: ETL LP W5'
    chan_list[20] = 'G2: HPK ch0'
    chan_list[21] = 'G2: HPK ch1'
    chan_list[22] = 'G2: HPK ch2'
    chan_list[23] = 'G2: HPK ch3'
    chan_list[24] = 'G3: Photek 1'
    chan_list[25] = 'G3: LP W7 ch 1'
    chan_list[26] = 'G3: LP W7 ch 2'
    chan_list[27] = 'G3: LP W7 ch 3'
    chan_list[28] = 'G3: LP W7 ch 4'
    chan_list[29] = 'G3: LP W7 ch 5'
    chan_list[30] = 'G3: LP W7 ch 6'
    chan_list[31] = 'G3: LP W7 ch 7'

    can_amp = TCanvas('amp')
    can_amp.Divide(8,4)
    
    hamp = []
    for ch in range(0,32):
        can_amp.cd(ch+1)
        hamp.append(TH1D('amp_ch{}'.format(ch),'Ch {}: amp; Amplitude [mV]'.format(ch),50,0,300))
        pulse.Project(hamp[ch].GetName(), 'amp['+str(ch)+']')
        hamp[ch].SetLineColor(kRed)
        hamp[ch].SetLineWidth(2)
        hamp[ch].DrawCopy()

    can_amp.Update()

    raw_input('Press enter to exit.')

