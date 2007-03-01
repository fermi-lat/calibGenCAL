"""
Loop throough all TH* objects in ROOT file & save to csv files

python roothist2csv.py <rootFile>

where:
       <rootFile> = input root file from which plots to print.
"""
__facility__    = "Offline"
__abstract__    = "Dump all TH?? objects in ROOT file to csv"
__author__      = "M. Strickman/Z.Fewtrell"
__date__        = "$Date: 2006/02/21 22:50:48 $"
__version__     = "$Revision: 1.1 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys,csv
from ROOT import TFile, TCanvas, TH1, gROOT

usage = "python roothist2csv.py <rootFile>"

# check command line
if len(sys.argv) != 2:
    print usage
    sys.exit(1)

# get filenames
rootPath = sys.argv[1]

gROOT.Reset()
rootFile = TFile(rootPath,"READ")

for k in rootFile.GetListOfKeys():
    cl = gROOT.GetClass(k.GetClassName());

    # HISTOGRAMS:
    if cl.InheritsFrom("TH1"):
        hist = k.ReadObj()
        if hist.GetDimension() != 1: continue
        histfilename = rootPath + "." + hist.GetName() + ".csv"
        csvfile = file(histfilename,"w")
        csvwriter = csv.writer(csvfile,dialect='excel',lineterminator='\n')
        nbins = hist.GetNbinsX()
        print histfilename
        print nbins
        for ibin in range(nbins+2):
            row=[hist.GetBinLowEdge(ibin),hist.GetBinWidth(ibin),hist.GetBinContent(ibin)]
            csvwriter.writerow(row)
        csvfile.close()
sys.exit(0)
