"""
Loop throough all TCanvas & TH* objects in ROOT file & save plots to image file

python dumpROOTPlots.py [-f ps|pdf|gif|png] <rootFile>

where:
       -f ps|pdf|gif|png = specify output image file type (default = pdf)
       <rootFile>        = input root file from which plots to print.
"""

__facility__    = "Offline"
__abstract__    = "Plot all TCanvas & TH?? objects in ROOT file"
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/03/15 14:30:46 $"
__version__     = "$Revision: 1.5 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

usage = "python dumpROOTPlots.py [-f ps|pdf|gif|png] <rootFile>"


### IMPORTS ###
import sys
import logging
import getopt

from ROOT import TFile, TCanvas, TH1, gROOT


### SUBROUTINES ###
def print_canvas(cvs):
    """ save ROOT.TCanvas object to file of type imgType """

    # bitmap type files need a Draw() first...
    if (imgType in bmpImgTypes):
        cvs.Draw()

    plotname = rootFile.GetName() + "." + cvs.GetName() + "." + imgType
    cvs.Print(plotname)

# setup logger
logging.basicConfig()
log = logging.getLogger('dumpROOTPlots')
log.setLevel(logging.INFO)


### COMMANDLINE ARGS ####
# 1st check for optional args
try:
    opts, args = getopt.getopt(sys.argv[1:], "-f:")
except getopt.GetoptError:
    log.error(usage)
    sys.exit(1)

imgType = "pdf" #default 
for o, a in opts:
    if o == "-f":
        imgType = a

# now check for req'd params
if len(args) != 1:
    log.error("bad n args: " + str(len(args)) + " " + usage)
    sys.exit(1)

# get filenames
rootPath = args[0]

# list of all valid image types
imgTypes = set(['ps','pdf','gif','png'])
# list of all bitmap-type image types
bmpImgTypes = set(['png','gif'])

if imgType not in imgTypes:
    log.error("bad imgType: " + imgType + " " + usage)

        

### MAIN LOOP ###
# setup ROOT
gROOT.Reset()
rootFile = TFile(rootPath,"READ")

for k in rootFile.GetListOfKeys():
    cls = gROOT.GetClass(k.GetClassName());

    # HISTOGRAMS:
    if cls.InheritsFrom("TH1"):
        hist = k.ReadObj()
        # Draw me to a canvas before printing....
        cvs = TCanvas(hist.GetTitle(), hist.GetName(),-1)
        hist.Draw()
        cvs.Update()

        print_canvas(cvs)

    # TCVS (plain ol' plots are stored in TCanvas
    if cls.InheritsFrom("TCanvas"):
        cvs = k.ReadObj()

        print_canvas(cvs)




