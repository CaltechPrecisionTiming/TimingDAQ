import os,sys
import argparse

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser()
    parser.add_argument("--outputDir", default="/eos/cms/store/user/sixie/Timing/ETLSimulation/",
                        help="Output Directory for storing the ntuples")
    args = parser.parse_args()    
    outputDir = args.outputDir

    shapingTimeOptions = [0.5,1.0,1.5,2.0,2.5,3.0,4.0]
    SNROptions = [5,10,15,20,30,40,50,100,1000]
    LGADPulseLibraryOptions = ['',"LGADPulseLibrary_35micron_Gain15_Prerad.root","LGADPulseLibrary_55micron_Gain15_Prerad.root","LGADPulseLibrary_75micron_Gain15_Prerad.root","LGADPulseLibrary_W6_55micron_Gain16_5E14_480V.root","LGADPulseLibrary_W6_55micron_Gain10_1E15_550V.root","LGADPulseLibrary_W6_55micron_Gain4_3E15_700V.root"]

    shapingTimeName = {
        0.5 : "0p5",
        1.0 : "1p0",
        1.5 : "1p5",
        2.0 : "2p0",
        2.5 : "2p5",
        3.0 : "3p0",
        4.0 : "4p0"
        }  

    LGADPulseLibraryName = {
        "" : "DefaultFixedSignal",
        "LGADPulseLibrary_35micron_Gain15_Prerad.root" : "35MicronGain15Prerad",
        "LGADPulseLibrary_55micron_Gain15_Prerad.root" : "55MicronGain15Prerad",
        "LGADPulseLibrary_75micron_Gain15_Prerad.root" : "75MicronGain15Prerad",
        "LGADPulseLibrary_W6_55micron_Gain16_5E14_480V.root" : "W6_55MicronGain16_5E14_480V",
        "LGADPulseLibrary_W6_55micron_Gain10_1E15_550V.root" : "W6_55MicronGain10_1E15_550V",
        "LGADPulseLibrary_W6_55micron_Gain4_3E15_700V.root" : "W6_55MicronGain4_3E15_700V",
                }  
    
    Jobs = [1,2]
    NEvents = 500

    for shapingTime in shapingTimeOptions:
        for SNR in SNROptions:
            for LGADPulseFile in LGADPulseLibraryOptions:
                label = "ShapingTime"+shapingTimeName[shapingTime]+"_SNR"+str(SNR)+"_"+LGADPulseLibraryName[LGADPulseFile]
                outputFilename = "ETLSimulation_"+label
                #print outputFilename
                command = "hadd -f " + outputDir+"/"+outputFilename + "_ALL.root " + outputDir+"/"+outputFilename + "_Job*.root"
                print command
                os.system(command)
