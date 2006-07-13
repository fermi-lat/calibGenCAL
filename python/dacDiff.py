"""
Diff 2 CAL DAC settings XML files.  The command line is:

python dacDiff.py FLE|FHE|LAC <dac_xml_file1> <dac_xml_file2> <output_root_file>

where:
    FLE|FHE|LAC|ULD    = DAC dacType to validate
    <dac_xml_file1>    = GLAST Cal dac xml 'fragment' file (minuend)
    <dac_xml_file2>    = GLAST Cal dac xml 'fragment' file (subtrahend)
    <output_root_file> = ROOT overplots & residuals will be saved here.

    
"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL DAC settings XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2006/02/15 23:38:38 $"
__version__     = "$Revision: 1.1 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calDacXML
import calConstant

usage = "python dacDiff.py FLE|FHE|LAC|ULD <dac_xml_file1> <dac_xml_file2> <output_root_file>"


# check command line
if len(sys.argv) != 5:
    print usage
    sys.exit(1)


# get filenames
dacType  = sys.argv[1]
dacPath1 = sys.argv[2]
dacPath2 = sys.argv[3]
rootPath = sys.argv[4]

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
    print "DAC dacType %s not supported" % dacType
    sys.exit(1)


# read in dac xml files
dacFile1 = calDacXML.calDacXML(dacPath1, dacType)
dacFile2 = calDacXML.calDacXML(dacPath2, dacType)

# check that towers are the same (ok if they are, just print warning)
dacTwrs1 = dacFile1.getTowers()
dacTwrs2 = dacFile2.getTowers()

if (dacTwrs1 != dacTwrs2):
    print "WARNING: input dac files have different towerss.  I quit"


# load up arrays
dac1 = dacFile1.read()
dac2 = dacFile2.read()

# subtract all elements in arrays
dacDiff = dac2 - dac1

for twr in dacTwrs1:
    print dacDiff[twr,:]


#######################################
########### ROOT PLOTS ################
#######################################

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
rootFile = ROOT.TFile(rootPath, "recreate")

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

# fill histograms
for twr in dacTwrs1:
    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):
                h1.Fill(dac1[twr, row, end, fe])
                h2.Fill(dac2[twr, row, end, fe])

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


# done!
rootFile.Close()
