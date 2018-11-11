### Script to submit dat2root jobs using condor
### Author: Dustin Anderson

import subprocess
import argparse

def GetCommandLineArgs():
    p = argparse.ArgumentParser()
    p.add_argument('-R', '--runs', type=int, nargs='+', help='List of runs to be processed. If two runs are given: if the order is increasing all the runs in the middle are processed as well, otherwise not.')
    return p.parse_args()

if __name__ == '__main__':
    args = GetCommandLineArgs()
    run = args.runs[0]
    print run
    # base = os.path.basename(outfile).replace('.root','')
    subfile = "automation/condor/cmd/condor_"+str(run)+".cmd"
    f = open(subfile,"w")
    f.write("Universe = vanilla\n")
    f.write("Executable = automation/condor/job.sh\n")
    f.write("Arguments = {0}\n".format(run))
    # f.write("Should_Transfer_Files = YES\n")
    # f.write("Transfer_Input_Files = "+infile+"\n")
    # f.write("Transfer_Output_Files = "+os.path.basename(outfile)+"\n")
    # f.write('Transfer_Output_Remaps = "'+os.path.basename(outfile)+"="+outfile+'"\n')
    # f.write("WhenToTransferOutput = ON_EXIT\n")
    f.write("Output = automation/condor/"+str(run)+".out.$(Cluster).$(Process)\n")
    f.write("Log = automation/condor/"+str(run)+".log.$(Cluster).$(Process)\n")
    f.write("Error = automation/condor/"+str(run)+".err.$(Cluster).$(Process)\n")
    f.write("Notification = Never\n")
    f.write("RequestMemory = 4000\n")
    # f.write("x509userproxy={0}\n".format(proxy))
    f.write("Queue 1\n")
    f.close()

    cmd = "condor_submit "+subfile
    print cmd
    subprocess.call(cmd, shell=True)
