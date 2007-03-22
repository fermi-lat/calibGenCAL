"""
Fit max(0,pol1) to all neighbor xtalk curves.

output is space delimited, to .txt file:
destIdx, srcIdx, srcDAC, destDAC

python fitNeighborXtalk.py <neighborXtalk.txt> <outputROOTPath>

where:
       <neighborXtalk.txt> = csv type text file (destIdx, srcIdc, srcDAC, destDAC)
       <outputROOTPath>    = output root file for histograms & fitted graphs.
       
"""

__facility__    = "Offline"
__abstract__    = "Fit max(0,pol1) to all neighbor xtalk curves."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/03/20 19:23:47 $"
__version__     = "$Revision: 1.1 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

### IMPORTS ###
import sys
import logging
import getopt
import array
import math

import ROOT
import csv

import zachUtil

usage = "fitNeighborXtalk.py <neighborXtalk.txt> <outputROOTPath>"

# setup logger
logging.basicConfig()
log = logging.getLogger('charVal')
log.setLevel(logging.INFO)

### COMMANDLINE ARGS ####
# 1st check for optional args
try:
    opts, args = getopt.getopt(sys.argv[1:], "")
except getopt.GetoptError:
    log.error(usage)
    sys.exit(1)

# now check for req'd params
if len(args) != 2:
    log.error("bad n args: " + str(len(args)) + " " + usage)
    sys.exit(1)

(xtalkPath, rootPath) = args

### open input 'csv' file ###
log.info("Opening input neighborXtalk txt file "+xtalkPath)
csv_sample = file(xtalkPath).read(8192)
dialect = csv.Sniffer().sniff(csv_sample)
has_header = csv.Sniffer().has_header(csv_sample)
infile = csv.reader(open(xtalkPath,"r"),dialect = dialect)

# skip optional header in first line
if has_header : infile.next()

# dictionary key =(destChan, srcChan) val = [(srcDac,destDac),(srcDac1,destDac1),.....]
inputDict = {}
### MAIN INPUT LOOP ###
for row in infile:
    (destChan, srcChan, srcDac, destDac) = row

    # skip le->le xtalk & keep only le->he xtalk
    (twr,lyr,col,face,diode) = zachUtil.diodeIdx2tuple(destChan)
    if diode == 0:
        continue

    k = (int(destChan), int(srcChan))

    # create new
    if not inputDict.has_key(k):
        inputDict[k] = []

    inputDict[k].append((float(srcDac), float(destDac)))

# open output ROOT file
outFile = ROOT.TFile(rootPath,"RECREATE")

# fitting func
#max_pol1 = ROOT.TF1("max_pol1","max(0,[0]+x*[1])")
f = ROOT.TF1("pol1","pol1")

#print ";dest src x_int y_int slope chisq max_src_dac max_dest_dac"

# histogram x_intercepts as we want to find the mean
h8_xint_hist = ROOT.TH1I("h8_xint","h8_xint",1000,0,0)
# store slope for all output-worthy channels
output_dict = {}
### FIT LOOP ###
for k,v in inputDict.iteritems():
    srcDacs = []
    destDacs = []

    for pair in v:
        (srcDac, destDac) = pair

        srcDacs.append(srcDac)
        destDacs.append(destDac)

    max_src_dac = srcDacs[-1]
    max_dest_dac = destDacs[-1]
    
    srcDacs = array.array('f',srcDacs)
    destDacs = array.array('f',destDacs)

    g = ROOT.TGraph(len(srcDacs),srcDacs, destDacs)
    (dest,src) = k
    name = "neighborXtalk_%d_from_%d"%(dest,src)
    
    g.SetNameTitle(name,name)
    c = ROOT.TCanvas(name,name)
    
    g.Draw("AL*")
    
    g.Fit(f,"Q","",100,4100)
    slope = f.GetParameter(1)
    y_int = f.GetParameter(0)
    chisq = f.GetChisquare()
    x_int = 0
    if slope != 0:
        x_int = -1*y_int/slope
        #print dest, src, x_int, y_int, slope, chisq, max_src_dac, max_dest_dac

    # skip channels w/ positive slope
    if slope <= 0 or max_dest_dac <= 0:
        continue

    c.Write()

    #fill x_int histogram
    if slope > 0.001: # only curves w/ higher slopes give good x_intercepts
        h8_xint_hist.Fill(x_int)

    # fill output dict
    if slope > 0.001:          # only keep channels w/ meaningful slope
        output_dict[k] = slope
       


# determine mean x_int #

#av  = h8_xint_hist.GetMean()
#err = h8_xint_hist.GetRMS()
h8_xint_hist.Fit("gaus","Q")
gaus_peak = h8_xint_hist.GetFunction("gaus").GetParameter(1)
gaus_wid  = h8_xint_hist.GetFunction("gaus").GetParameter(2)
#log.info( "HEX8_X_INT %f (%f) gaus %f (%f)"%(av, err, gaus_peak, gaus_wid))
### remove outliers ####
#for i in range(3):
#     h8_xint_hist.SetAxisRange(av-2*err,av+2*err)
#     av  = h8_xint_hist.GetMean()
#     err = h8_xint_hist.GetRMS()
#     h8_xint_hist.Fit("gaus","Q")
#     gaus_peak = h8_xint_hist.GetFunction("gaus").GetParameter(1)
#     gaus_wid  = h8_xint_hist.GetFunction("gaus").GetParameter(2)
#     log.info( "HEX8_X_INT %f (%f) gaus %f (%f)"%(av, err, gaus_peak, gaus_wid))

av_xint = gaus_peak


### print column headers ###
print ";dest src x_int slope"
for k,slope in output_dict.iteritems():
     (dest,src) = k
     #print dest, twr,lyr,col,face,rng, src, x_int, av_xint, y_int, slope
     print dest, src, av_xint, slope
    

outFile.Write()
outFile.Close()
