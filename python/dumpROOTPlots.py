"""
Loop throough all TCanvas & TH* objects in ROOT file & save ps plots to file

python dumpROOTPlots.py <rootFile>

where:
       <rootFile> = input root file from which plots to print.
"""
__facility__    = "Offline"
__abstract__    = "Plot all TCanvas & TH?? objects in ROOT file"
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2006/02/15 23:38:38 $"
__version__     = "$Revision: 1.1 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
from ROOT import TFile, TCanvas, TH1, gROOT

usage = "python dumpROOTPlots.py <rootFile>"

# check command line
if len(sys.argv) != 2:
    print usage
    sys.exit(1)

# get filenames
rootPath = sys.argv[1]

gROOT.Reset()
rootFile = TFile(rootPath)

for k in rootFile.GetListOfKeys():
    cl = gROOT.GetClass(k.GetClassName());

    # HISTOGRAMS:
    if cl.InheritsFrom("TH1"):
        hist = k.ReadObj()
        plotname = rootPath + "." + hist.GetName() + ".ps"
        canvas = TCanvas(rootPath, hist.GetName(),-1)

        hist.Draw()
        canvas.Update()
        canvas.Print(plotname)

    # TCANVAS (plain ol' plots are stored in TCanvas
    if cl.InheritsFrom("TCanvas"):
        canvas = k.ReadObj()
        plotname = rootPath + "." + canvas.GetName() + ".ps"
        canvas.Print(plotname)

