import ROOT
# Workaround to fix threadlock issues with GUI
ROOT.PyConfig.StartGuiThread = False
import logging
logging.basicConfig(level=logging.INFO)
from optparse import OptionParser

parser = OptionParser()
parser.add_option("--submitDir", help="dir to store the output", default="submit_dir")
parser.add_option("--dataDir", help="dir to search for input"  , default="/afs/cern.ch/work/r/rsmith/")
parser.add_option("--driver", help="select where to run", choices=("direct", "prooflite", "LSF","grid"), default="direct")
parser.add_option('--doOverwrite', help="Overwrite submit dir if it already exists",action="store_true", default=False)

#parser.add_option("--whichAnalysis", help="select analysis", choices=("noCut", "Zmumu" , "Zee", "Wenu","NONE"), default="NONE")
#parser.add_option("--errorLevel", help="select error level", choices=("VERBOSE","DEBUG","WARNING","ERROR"), default="WARNING")


(options, args) = parser.parse_args()

import atexit
@atexit.register
def quiet_exit():
    ROOT.gSystem.Exit(0)

ROOT.gROOT.Macro( '$ROOTCOREDIR/scripts/load_packages.C' )

# Initialize the xAOD infrastructure
#ROOT.xAOD.Init()

# create a new sample handler to describe the data files we use
logging.info("creating new sample handler")
sh_all = ROOT.SH.SampleHandler()

list = ROOT.SH.DiskListLocal("/afs/cern.ch/work/r/rsmith/lvlv_datasets")
ROOT.SH.scanDir(sh_all,list, "*")

sh_all.setMetaString ("nc_tree", "CollectionTree");

sh_all.printContent();

# this is the basic description of our job
logging.info("creating new job")
job = ROOT.EL.Job()
job.sampleHandler(sh_all)
job.useXAOD()

logging.info("creating algorithms")
calibrateST        = ROOT.CalibrateST()
selectDileptonicWW = ROOT.SelectDileptonicWWEvents()

calculateRJigsawVariables                   = ROOT.CalculateRJigsawVariables()
calculateRJigsawVariables.m_calculator_name = 1#lvlv enum

job.algsAdd(calibrateST)
job.algsAdd(selectDileptonicWW)

import os
if os.path.isdir(options.submitDir) :
    print options.submitDir + " already exists."
    if options.doOverwrite :
        print "Overwriting previous submitDir"
        import shutil
        shutil.rmtree(options.submitDir)
    else :
        print "Exiting.  If you want to overwrite the previous submitDir, use --doOverwrite"
        quiet_exit()

logging.info("creating driver")
driver = None
if (options.driver == "direct"):
    print "direct driver"
    logging.info("running on direct")
    driver = ROOT.EL.DirectDriver()
    logging.info("submit job")
    driver.submit(job, options.submitDir)
elif (options.driver == "prooflite"):
    print "prooflite"
    logging.info("running on prooflite")
    driver = ROOT.EL.ProofDriver()
    logging.info("submit job")
    driver.submit(job, options.submitDir)
elif (options.driver == "grid"):
    print "grid driver"
    logging.info("running on Grid")
    driver = ROOT.EL.PrunDriver()
    driver.options().setString("nc_outputSampleName", "user.rsmith.metNote.weekOne.v3.%in:name[2]%.%in:name[3]%");
#    driver.options().setString(EL::Job::optGridNfilesPerJob, "1")
#driver.options().setDouble("nc_disableAutoRetry", 1)
#    driver.options().setDouble("nc_nFilesPerJob", 1)
    driver.options().setDouble(ROOT.EL.Job.optGridMergeOutput, 1);

    logging.info("submit job")
    driver.submitOnly(job, options.submitDir+ options.whichAnalysis + "_mediumBad_" + options.dataDir)

