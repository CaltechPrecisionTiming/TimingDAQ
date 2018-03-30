# /usr/bin/python

#Author: Ben Tannenwald
#Date: March 29, 2018
#Purpose: Script to combine executables for 1) converting testbeam .dat files to .root, and 2) analyzing root file and producing timing-relevant information by channel

import os,sys, argparse

# *** 0. setup parser for command line
parser = argparse.ArgumentParser()
parser.add_argument("--inputDatFile", help="Name of input .dat file containing raw data from testbeam")
parser.add_argument("--outputFile", help="Name of .root file after ROOT converstion and BTL analysis (if --doAnalysis set to True")
parser.add_argument("--nEvents", help="Number of events to process")
parser.add_argument("--config", help="File to use for configuring channels. Default will be used (at user's peril!) if no option provided")
parser.add_argument("--BTLRunMode", help="Flag for BTL_Analysis run mode. 'None'== do nothing, 'Calibration' produces channel-by-channel calibrations, 'Analysis' produces plots and trees for timing analysis")
parser.add_argument("--inputPixelFile", help="Name of input .root file containing pixel data from testbeam")
parser.add_argument("--useOldCode", help="Flag to use new code (TimingDAQ) if == true or old code (FNALTestbeam_052017) if == false")

args = parser.parse_args()

if (len(vars(args)) != 7): # 6 --> three for options
    os.system('python combine_dat2RootPixels-BTL_Analysis.py -h')
    quit()

# ** A. Test input .dat file and exit if DNE
if(args.inputDatFile is None):
    print "#### Need to specify input .dat file using --inputDatFile <address to .dat file> ####\nEXITING"
    quit()
else:
    if ( not os.path.exists(args.inputDatFile) ):
        print "#### Specified input .dat file ({0}) DNE ####.\nEXITING".format(args.inputDatFile)
        quit()
    else:
        print '-- Setting inputDatFile = {0}'.format(args.inputDatFile)

# ** B. Test output file entry
if(args.outputFile is None):
    print "#### Need to specify output .root filename using --outputFile <desired filename> ####\nEXITING"
    quit()
else:
    print '-- Setting outputFile = {0}'.format(args.outputFile)

# ** C. Test nEvents file entry
if(args.nEvents is None):
    print "#### Need to specify nEvents to run over using --nEvents <desired # of events> ####\nEXITING"
    quit()
else:
    print '-- Setting nEvents = {0}'.format(args.nEvents)

# ** D. Test useOldCode flag and exit if not sensible
if(args.useOldCode is None):
    print "#### Need to specify useOldCode flag using --useOldCode <true/false> ####"
    print "#### Using default  [===== WARNING =====] useOldCode = FALSE [===== WARNING =====] ####\n"
    args.useOldCode = "false"
else:
    if( not(args.useOldCode == "true" or args.useOldCode == "True" or args.useOldCode == "false" or args.useOldCode == "False") ):
        print "#### Please use true/false when setting --useOldCode. Supplied value ({0}) does not match ####\nEXITING".format(args.useOldCode)
    else:
        print '-- Setting useOldCode = {0}'.format(args.useOldCode)

# ** E. Test config file entry
config = "config/VME_FNALTestbeam_180330_UVA.config"
#if (args.useOldCode == "true" || args.useOldCode == "True"):
#    config = "config/15may2017.config"

if(args.config is None):
    print "#### Need to specify config file using --config <desired config file> ####"
    print "#### Using default  [===== WARNING =====] {0} [===== WARNING =====] ####\n".format(config)
    args.config = config
else:
    if ( not os.path.exists(args.config) ):
        print "#### Specified input .config file ({0}) DNE. Using default (DANGER) ####.\nEXITING".format(args.config)
        print "#### Using default  [===== WARNING =====] {0} [===== WARNING =====] ####\n".format(config)
        args.config = config
    else:
        print '-- Setting config = {0}'.format(args.config)

# ** F. Test BTLRunMode flag and exit if not sensible
if(args.BTLRunMode is None):
    print "#### Need to set either BTLRunMode using --BTLRunMode <None/Calibration/Analysis> ####\nEXITING"
    quit()
else:
    if( not(args.BTLRunMode == "None" or args.BTLRunMode == "Calibration" or args.BTLRunMode == "Analysis") ):
        print "#### Please use None/Calibration/Analysis when setting --BTLRunMode <option>. Supplied value ({0}) does not match ####\nEXITING".format(args.BTLRunMode)
    else:
        print '-- Setting BTLRunMode = {0}'.format(args.BTLRunMode)

# ** G. Test input pixel .root file and ignore if DNE
if(args.inputPixelFile is None):
    print "#### No input pixel .root file specified. ===> MOVING ON =====> ####\nEXITING" 
    
else:
    if ( not os.path.exists(args.inputPixelFile) ):
        print "#### Specified pixel input {0} file DNE ===> MOVING ON =====> ####\nEXITING".format(args.inputPixelFile)
        quit()
    else:
        print '-- Setting inputPixelFile = {0}'.format(args.inputPixelFile)



# *** 1. Create .tar of directory and store in personal EOS
print "##########     Processing raw data     ##########"
#os.system("./dat2rootPixels {0} {1} {2} {3} {4}".format(args.inputDatFile, args.inputPixelFile, args.inputNIMFile, args.outputFile, args.nEvents)) 
if ( (args.useOldCode == "true" or args.useOldCode == "True") ):
    if (args.inputPixelFile is None):
        processRunString = "./ConvertDat2Root --inputFileName={0} --outputFileName={1} --nEvents={2} --config={3}".format(args.inputDatFile, args.outputFile, args.nEvents, args.config)
    else: # run with pixels
        processRunString = "./ConvertDat2RootWithPixels --inputFileName={0} --outputFileName={1} --nEvents={2}  --pixelInputFileName={3} --config={4}".format(args.inputDatFile, args.outputFile, args.nEvents, args.inputPixelFile, args.config)
else: # new code
        processRunString = "./VMEDat2Root --input_file={0} --output_file={1} --N_evts={2} --config={3} --save_meas --save_raw".format(args.inputDatFile, args.outputFile, args.nEvents, args.config, args.inputPixelFile)    
        if ( args.inputPixelFile is not None):
            processRunString += " --pixel_input_file={0}".format( args.inputPixelFile)

print processRunString
os.system(processRunString)

#./VMEDat2Root --input_file=data_03-29-18/RawDataSaver0CMSVMETiming_Run35_0_Raw.dat --output_file=data_03-29-18/v2_RawDataSaver0CMSVMETiming_Run35_0_Raw.root --N_evts=3000 --config=config/VME_FNALTestbeam_180329.config --save_meas --save_raw

# *** 2. Run BTL_Analysis in proper mode
if (args.BTLRunMode == "None"):
    quit()

print "##########     Running BTL_Analysis in {0} Mode     ##########".format(args.BTLRunMode)

btlRunString = " ./BTL_Analysis --inputRootFile={0}".format(args.outputFile)
if (args.BTLRunMode == "Calibrate"):
    btlRunString += " --calibrateChannels=true"
os.system(btlRunString)

