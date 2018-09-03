import os,sys
import time
import argparse

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser()
    parser.add_argument("--inputDir", default="/eos/cms/store/user/sixie/Timing/ETLSimulation/",
                        help="Input Directory for storing the ntuples")
    parser.add_argument("--outputDir", default="/eos/cms/store/group/phys_susy/razor/Timing/ETL_ASIC_Simulation/",
                        help="Output Directory for storing the ntuples")
    args = parser.parse_args()    
    inputDir = args.inputDir
    outputDir = args.outputDir

    current_dir = os.getcwd()

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
    
    os.system("mkdir -p jobs/src;mkdir -p jobs/out;mkdir -p jobs/log")
    
    for shapingTime in shapingTimeOptions:
        for SNR in SNROptions:
            for LGADPulseFile in LGADPulseLibraryOptions:
                label = "ShapingTime"+shapingTimeName[shapingTime]+"_SNR"+str(SNR)+"_"+LGADPulseLibraryName[LGADPulseFile]
                outputFilename = "ETLSimulation_"+label

                filename = "jobs/src/"+label+".sh"
                print filename
                f = open(filename, 'w')
                f.write("cd "+current_dir+"\n")
                f.write("eval `scramv1 runtime -sh`\n")
                run_command = "./ETL_ASIC_Dat2Root --input_file=" +inputDir+"/"+outputFilename+"_ALL.root  --config_file=config/ETL_ASIC/ETL_ASIC_Simulation.config  --verbose  --save_meas --output_file="+outputDir+"/"+outputFilename+"_Analyzed.root\n"
                f.write(run_command)
                f.close()
                os.system("chmod 744 "+ filename)
                #print outputFilename
                #command = "hadd -f " + outputDir+"/"+outputFilename + "_ALL.root " + outputDir+"/"+outputFilename + "_Job*.root"
                command = "bsub -q 1nh -o "+current_dir+"/jobs/out/"+outputFilename+".out"+ " -J "+outputFilename+" " + current_dir+"/"+filename 
                print command
                os.system(command)
                time.sleep(0.2)
