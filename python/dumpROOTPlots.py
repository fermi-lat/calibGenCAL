"""
Loop throough all TCanvas & TH* objects in ROOT file & save plots to image file

python dumpROOTPlots.py [-f ps|pdf|gif|png] [-n namematch] <rootFile>

where:
       -f ps|pdf|gif|png = specify output image file type (default = pdf)
       -n namematch      = only output plots whose name contains the 'namematch' string         
       <rootFile>        = input root file from which plots to print.
"""

__facility__    = "Offline"
__abstract__    = "Plot all TCanvas & TH?? objects in ROOT file"
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/07/21 17:59:39 $"
__version__     = "$Revision: 1.11 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"



### IMPORTS ###
import sys
import logging
import getopt

from ROOT import TFile, TCanvas, TH1, gROOT
import ROOT

# setup logger
logging.basicConfig()
log = logging.getLogger('dumpROOTPlots')
log.setLevel(logging.INFO)

### SUBROUTINES ###
def print_canvas(cvs):
    """ save ROOT.TCanvas object to file of type imgType """

    # bitmap type files need a Draw() first...
    if (imgType in bmpImgTypes):
        cvs.Draw()

    plotname = rootFile.GetName() + "." + cvs.GetName() + "." + imgType
    log.info("Writing %s"%plotname)
    cvs.Print(plotname)



### COMMANDLINE ARGS ####
# 1st check for optional args
try:
    opts, args = getopt.getopt(sys.argv[1:], "-f:-n:")
except getopt.GetoptError:
    log.error(__doc__)
    sys.exit(1)

imgType = "png" #default
namematch = "" # disabled by default
for o, a in opts:
    if o == "-f":
        imgType = a
    if o == "-n":
        namematch = a

# now check for req'd params
if len(args) != 1:
    log.error("bad n args: " + str(len(args)) + " " + __doc__)
    sys.exit(1)

# get filenames
rootPath = args[0]

# list of all valid image types
imgTypes = set(['ps','pdf','gif','png'])
# list of all bitmap-type image types
bmpImgTypes = set(['png','gif'])

if imgType not in imgTypes:
    log.error("bad imgType: " + imgType + " " + __doc__)

        

### MAIN LOOP ###
# setup ROOT
gROOT.Reset()
rootFile = TFile(rootPath,"READ")


def processDir(directory):
    """
    make image file for each TCanvas & TH1 in directory
    directory could either be a ROOT TFile or TDirectory
    recusively process sub directories
    """

    # don't know if inputDir is TFile or TDirectory, simplest is to
    # treat all as if TDirectory, if I want to do this, I need to read
    # all objs into memory (otherwise I would have to use
    # GetListOfKeys() instead of GetList())
    #
    # if you know a better way, don't be shy!
    directory.ReadAll()

    l = directory.GetList()
    for obj in l:
        cls = ROOT.gROOT.GetClass(obj.ClassName())

        # optional name check
        if namematch != "":
            if k.GetName().find(namematch) < 0:
                continue

        # HISTOGRAMS:
        if cls.InheritsFrom("TH1"):
            hist = obj
            # Draw me to a canvas before printing....
            cvs = TCanvas(hist.GetTitle(), hist.GetName(),-1)
            hist.Draw()
            cvs.Update()
            
            print_canvas(cvs)

            # TCVS (plain ol' plots are stored in TCanvas
        elif cls.InheritsFrom("TCanvas"):
            cvs = obj
            print_canvas(cvs)
                
        elif cls.InheritsFrom("TDirectory"):
            processDir(obj)

processDir(rootFile)






