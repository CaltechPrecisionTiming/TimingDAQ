### Script to submit dat2root jobs using condor
### Author: Dustin Anderson

import os
import sys
import argparse

def submitCondorJob(executable, options, infile, outfile, 
        submit=False, proxy="/tmp/x509up_u31156"):
    base = os.path.basename(outfile).replace('.root','')
    subfile = "condor/condor_"+base+".cmd"
    f = open(subfile,"w")
    f.write("Universe = vanilla\n")
    f.write("Executable = "+executable+"\n")
    f.write("Arguments =  "+(' '.join(options))+"\n")
    f.write("Should_Transfer_Files = YES\n")
    f.write("Transfer_Input_Files = "+infile+"\n")
    f.write("Transfer_Output_Files = "+os.path.basename(outfile)+"\n")
    f.write('Transfer_Output_Remaps = "'+os.path.basename(outfile)+"="+outfile+'"\n')
    f.write("WhenToTransferOutput = ON_EXIT\n")
    f.write("Output = condor/"+base+".out.$(Cluster).$(Process)\n")
    f.write("Log = condor/"+base+".log.$(Cluster).$(Process)\n")
    f.write("Error = condor/"+base+".err.$(Cluster).$(Process)\n")
    f.write("Notification = Never\n")
    f.write("x509userproxy={0}\n".format(proxy))
    f.write("Queue 1\n")
    f.close()

    cmd = "condor_submit "+subfile
    print cmd
    if submit:
        os.system(cmd)

if __name__ == '__main__':
    # assumes this script is in the same directory as the executable
    local_dir = os.path.dirname(os.path.realpath(__file__))+'/'

    parser = argparse.ArgumentParser()
    parser.add_argument('infile', help="Full path to input file")
    parser.add_argument('--no-sub', dest='noSub', action='store_true', 
            help='Do not submit jobs')
    parser.add_argument('--proxy', dest="proxy", help="Path to proxy", 
            default='/tmp/x509up_u48463')
    parser.add_argument('--out-dir', dest='outDir', help='Output directory', 
            default=local_dir)
    parser.add_argument('--config', help='Config file', default='15may2017.config')
    args = parser.parse_args()

    outfile_local = os.path.basename(args.infile.replace('.dat', '.root'))
    outfile = args.outDir+'/'+outfile_local
    executable = local_dir+'/runDat2Root'

    infiles = [
           args.infile,
           local_dir+"config/"+args.config,
           local_dir+"v1740_bd1_group_0_dV.txt",
           local_dir+"v1740_bd1_group_0_offset.txt",
           local_dir+"v1740_bd1_group_1_dV.txt",
           local_dir+"v1740_bd1_group_1_offset.txt",
           local_dir+"v1740_bd1_group_2_dV.txt",
           local_dir+"v1740_bd1_group_2_offset.txt",
           local_dir+"v1740_bd1_group_3_dV.txt",
           local_dir+"v1740_bd1_group_3_offset.txt",
           ]
    infiles = ', '.join(infiles)

    options = [local_dir,
            os.path.basename(args.infile), 
            outfile_local,
            args.config]
    print "runDat2Root options:",(' '.join(options))
    submitCondorJob(executable, options, infiles, outfile, 
            submit=(not args.noSub), proxy=args.proxy)
