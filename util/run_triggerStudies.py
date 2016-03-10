import ROOT
# Workaround to fix threadlock issues with GUI
ROOT.PyConfig.StartGuiThread = False
import logging
logging.basicConfig(level=logging.INFO)

import commonOptions

parser = commonOptions.parseCommonOptions()
#you can add additional options here if you want
#parser.add_option('--verbosity', help   = "Run all algs at the selected verbosity.",choices=("info", "warning","error", "debug", "verbose"), default="error")

(options, args) = parser.parse_args()
#print options

ROOT.gROOT.Macro( '$ROOTCOREDIR/scripts/load_packages.C' )

# Initialize the xAOD infrastructure
# create a new sample handler to describe the data files we use
logging.info("creating new sample handler")
sh_all = ROOT.SH.SampleHandler()

commonOptions.fillSampleHandler(sh_all, options.inputDS, options.gridUser, options.gridTag)

sh_all.setMetaString ("nc_tree", "CollectionTree");
#sh_all.printContent();

# this is the basic description of our job
logging.info("creating new job")
job = ROOT.EL.Job()
job.sampleHandler(sh_all)
job.useXAOD()

logging.info("creating algorithms")

outputFilename = "trees"
output = ROOT.EL.OutputStream(outputFilename);

import collections
algsToRun = collections.OrderedDict()


algsToRun["basicEventSelection"]       = ROOT.BasicEventSelection()
commonOptions.configBasicEventSelection(algsToRun["basicEventSelection"] )
#algsToRun["basicEventSelection"].setConfig("$ROOTCOREBIN/data/RJigsawTools/baseEvent_Trigger.config")
# algsToRun["calibrateST"]               = ROOT.CalibrateST()
algsToRun["triggerPassThrough"]        = ROOT.TriggerPassThrough()

#todo move the enums to a separate file since they are shared by multiple algs
algsToRun["calculateRJigsawVariables"]                = ROOT.CalculateRJigsawVariables()
algsToRun["calculateRJigsawVariables"].calculatorName = ROOT.CalculateRJigsawVariables.zlCalculator
algsToRun["calculateRegionVars"]                      = ROOT.CalculateRegionVars()
algsToRun["calculateRegionVars"].calculatorName       = ROOT.CalculateRegionVars.zlCalculator

for regionName in ["TriggerPassThrough"]:
    tmpWriteOutputNtuple                       = ROOT.WriteOutputNtuple()
    tmpWriteOutputNtuple.outputName            = outputFilename
    tmpWriteOutputNtuple.regionName            = regionName
    algsToRun["writeOutputNtuple_"+regionName] = tmpWriteOutputNtuple

if options.doSystematics : commonOptions.doSystematics(algsToRun)

job.outputAdd(output);
commonOptions.addAlgsFromDict(job , algsToRun , options.verbosity)

if options.nevents > 0 :
    logging.info("Running " + str(options.nevents) + " events")
    job.options().setDouble (ROOT.EL.Job.optMaxEvents, float(options.nevents));

commonOptions.overwriteSubmitDir(options.submitDir , options.doOverwrite)
commonOptions.submitJob         ( job , options.driver , options.submitDir , options.gridUser , options.gridTag)