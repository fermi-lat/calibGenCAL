"""
Diff 2 CAL DAC settings XML files.  The command line is:

dacDiff [-V] FLE|FHE|LAC|ULD <dac_xml_file1> <dac_xml_file2> <output_base_filename>

where:
    -V                 = verbose output
    FLE|FHE|LAC|ULD    = DAC dacType to validate
    <dac_xml_file1>    = GLAST Cal dac xml 'fragment' file (minuend)
    <dac_xml_file2>    = GLAST Cal dac xml 'fragment' file (subtrahend)
    <output_base_filename> = ROOT & txt logfile will base their filenames on this string

    
"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL DAC settings XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/02/03 00:51:49 $"
__version__     = "$Revision: 1.9 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


import sys
import os
import logging
import getopt

import numarray

import calDacXML
import calConstant



# setup logger

logging.basicConfig()
log = logging.getLogger('dacDiff')
log.setLevel(logging.INFO)

# check command line

try:
    opts = getopt.getopt(sys.argv[1:], "-V")
except getopt.GetoptError:
    log.error(__doc__)
    sys.exit(1)

optList = opts[0]
for o in optList:
    if o[0] == '-V':
        log.setLevel(logging.DEBUG)

args = opts[1] 
if len(args) != 4:
    log.error(__doc__)
    sys.exit(1)


# get filenames
dacType  = args[0]
dacPath1 = args[1]
dacPath2 = args[2]
baseFilename = args[3]

rootPath = baseFilename+".root"
logPath = baseFilename+".log.txt"

# add logfile
if os.path.exists(logPath):
    log.debug('Removing old log file %s', logPath)
    os.remove(logPath)
hdl = logging.FileHandler(logPath)
fmt = logging.Formatter('%(levelname)s %(message)s')
hdl.setFormatter(fmt)
log.addHandler(hdl)

#determine dac dacType
if dacType == 'FLE':
    dacType = 'fle_dac'
elif dacType == 'FHE':
    dacType = 'fhe_dac'
elif dacType == 'LAC':
    dacType = 'log_acpt'
elif dacType == 'ULD':
    dacType = 'rng_uld_dac'     
else:
    log.error("DAC dacType %s not supported" % dacType)
    sys.exit(1)


# read in dac xml files
dacFile1 = calDacXML.calSettingsXML(dacPath1, dacType)
dacFile2 = calDacXML.calSettingsXML(dacPath2, dacType)

# check that towers are the same (ok if they are, just print warning)
dacTwrs1 = set(dacFile1.getTowers())
dacTwrs2 = set(dacFile2.getTowers())

if dacTwrs1 != dacTwrs2:
    log.warning("input dac files have different towers")
    if dacTwrs1.issubset(dacTwrs2):
        towers = dacTwrs1
    elif dacTwrs2.issubset(dacTwrs1):
        towers = dacTwrs2
    else:
        log.error("input dac files have no matching towers")
        sys.exit(1)
else:
    towers = dacTwrs1
log.debug("using towers %s", towers)                    


# load up arrays
dac1 = dacFile1.read()
dac2 = dacFile2.read()

dacFile1.close()
dacFile2.close()

# subtract all elements in arrays
dacDiff = dac2.astype(numarray.Int8) - dac1.astype(numarray.Int8)

for twr in towers:
    log.info("Tower %d\n%s", twr, dacDiff[twr,:])


#######################################
########### ROOT PLOTS ################
#######################################

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "dacDiff(%s,%s)"%(dacPath1,dacPath2),
                      9)

# build ROOT canvas (for which onto drawem)
cs = ROOT.TCanvas(dacType + '_diff', dacType + '_diff', -1)
cs.SetGrid()
cs.SetLogy()
leg = ROOT.TLegend(0.5, 0.88, 0.99, 0.99)

# build 1st histogram
histName1 = dacPath1 + "_hist"
h1 = ROOT.TH1S(histName1, dacType, 128, 0,128)
h1.SetLineColor(1)
h1.SetStats(False) # turn off statistics box in plot

# build 2nd histogram
histName2 = dacPath2 + "_hist"
h2 = ROOT.TH1S(histName2, dacType, 128, 0,128)
h2.SetLineColor(2)
h2.SetStats(False) # turn off statistics box in plot

# build diff histogram
hdiff_name = dacType+"_diff"
hdiff = ROOT.TH1S(hdiff_name, hdiff_name, 256,0,0)


# fill histograms
for twr in towers:
    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):
                h1.Fill(dac1[twr, row, end, fe])
                h2.Fill(dac2[twr, row, end, fe])
                hdiff.Fill(dacDiff[twr,row,end,fe])

# add legend entries
leg.AddEntry(h1,dacPath1,"L")
leg.AddEntry(h2,dacPath2,"L")

# draw h1 1st
h1.Draw('')
cs.Update()
# overplot h2
h2.Draw('SAME')
cs.Update()

leg.Draw()
cs.Update()
cs.Write()
hdiff.Write()


# done!
rootFile.Close()

sys.exit(0)
