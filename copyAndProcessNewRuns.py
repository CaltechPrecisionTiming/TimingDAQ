# /usr/bin/python

#Author: Ben Tannenwald
#Date: March 30, 2018
#Purpose: Script to rsync from run directory to LPC EOS and run processing for new files 

import os, sys, subprocess

targetDir = "/eos/uscms/store/user/cmstestbeam/BTL/March2018/OTSDAQ/CMSTiming/"

print "####  Running rsync on to get .dat files from ftbf  ####"
copyDatCommand = "rsync otsdaq@ftbf-daq-08.fnal.gov:/data/TestBeam/2018_03_March_CMSTiming/CMSTiming/*.dat {0}".format(targetDir)
#copyDatCommand = "rsync -rtlv ../TimingDAQ/data_03-30-18/*.dat ./"
proc1 = subprocess.Popen(copyDatCommand,stdout=subprocess.PIPE,shell=True)
(out1, err1) = proc1.communicate()
out1 = out1.split('\n')
for file in out1:
    if '.dat' in file:
        print "-- Copied RAW: {0}".format(file)

print "####  Check for pixel files  ####\n"
copyPixelCommand = "otsdaq@rulinux04:/data/TestBeam/2018_03_March_CMSTiming/CMSTimingConverted/*.root {0}".format(targetDir)
#copyPixelCommand = "rsync -rtlv ../TimingDAQ/data_03-30-18/*.root ./"
proc2 = subprocess.Popen(copyPixelCommand,stdout=subprocess.PIPE,shell=True)
(out2, err2) = proc2.communicate()
out2 = out2.split('\n')
for file in out2:
    if '.root' in file:
        print "-- Copied Pixel: {0}".format(file)


print "####  Check output storage for files to process ####\n"
runsToProcess = []
for file in out1:
    if '.dat' in file:
        #print "RawDataSaver0CMSVMETiming_Run81_0_Raw.dat", 
        #print file, file.split('RawDataSaver0CMSVMETiming_Run')[1].split('_Raw')[0]
        runNumber = file.split('RawDataSaver0CMSVMETiming_Run')[1].split('_Raw')[0]
        runNumber_pixel = runNumber.split('_')[0]
        print runNumber, runNumber_pixel, file
        # now check for matching pixel file
        for file in out2:
            if file == "Run{0}_CMSTiming_converted.root".format(runNumber_pixel):
                print "Will Process {0}".format(runNumber_pixel)
                runsToProcess.append(runNumber)

#print runsToProcess
print "####  Process files with both .dat and .root files  ####\n"

for run in runsToProcess:
    processCommandString = "echo  /uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ/VMEDat2Root --input_file={0}/RawDataSaver0CMSVMETiming_Run{1}_Raw.dat --pixel_input_file={0}/Run{2}_CMSTiming_converted.root --output_file={0}/V1/RawDataSaver0CMSVMETiming_Run{1}.root --config=/uscms_data/d2/sxie/releases/CMSSW_9_0_2/src/TimingDAQ/config/VME_FNALTestbeam_180329_v1.config.config".format(targetDir, run, run.split('_')[0])

    os.system(processCommandString)

