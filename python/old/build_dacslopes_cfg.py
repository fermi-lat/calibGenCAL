"""
Builds configuration file for dacSlopesGen.py script from info in gensettings.py cfg file

Usage: build_dacslopes_cfg [-b bias.xml] [-g gensettings.cfg] [-d dtdfile.dtd] muslope.xml output_dacslopes.cfg

Note: this program depends on input filenames w/ certain patterns.  Filename conventions change from time to time.  we try to keep the scripts current, so
you may have to modify the glob matches to work w/ older files.

where:
       muslope.xml          = offline adc->energy DacSlopes calibration file.  (req'd as it is not specified in gensettings.cfg file)
       bias.xml             = trigger bias file (default is to glob for file in current directory)
       gensettings.cfg      = name of input gensettings.py cfg file (default = "gensettings.cfg")
       dtdfile              = name of dtd file to use in output (will be found in "$CALIBUTILROOT/xml/Cal" )
       output_dacslopes.cfg = output cfg file for tholdCIGen.py script

note:
    see calibGenCAL/doc/gensettings_scripts.html for more information.

"""

__facility__  = "Offline"
__abstract__  = "Builds configuration file for dacSlopesGen.py script from info in gensettings.py cfg file"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/07/27 18:01:47 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import ConfigParser
import getopt
import glob

# set up logging
logging.basicConfig()
log = logging.getLogger('build_dacslopes_cfg')
log.setLevel(logging.INFO)


# retrieve commandline parms
usage = "Usage: build_dacslopes_cfg [-b bias.xml] [-g gensettings.cfg] [-d dtdfile.dtd] muslope.xml output_dacslopes.cfg"
# check command line
try:
    (opts,args) = getopt.getopt(sys.argv[1:], "b:d:g:")
except getopt.GetoptError:
    log.error(usage)
    sys.exit(1)

# defaults
cfg_path           = "gensettings.cfg"
dtd_path           = "calCalib_v2r3.dtd"

# try to find bias file

bias_path = None
biasList = glob.glob('*[bB]ias*.xml')
if len(biasList) == 1:
    bias_path = biasList[0]

for (o,a) in opts:
    if o == '-d':
        dtd_path = a
    elif o == '-g':
        cfg_path = a
    elif o == '-b':
        bias_path = a 
        
if len(args) != 2:
    log.error(usage)
    sys.exit(1)
    
muslope_path       = args[0]
output_path        = args[1]

if bias_path is None:
    log.error("No bias file found or specified")
    sys.exit(1)


##############################################
### READ GENSETTINGS.CFG #####################
### Stolen (of course) from gensettings.py ###
##############################################

# set up config file and read in sections
log.info("Reading file %s", cfg_path)
cfile = ConfigParser.SafeConfigParser()
cfile.read(cfg_path)
sections = cfile.sections()
log.info(sections)

# find detector sections
detsections=[]
for isec in sections:
    if isec[0:2] == "FM":
        detsections.append(isec)
log.info(detsections)
if len(detsections) == 0:
    log.error('No det sections')
    sys.exit(1)

#find config sections
configsections=[]
for isec in sections:
    if isec[0:3] == "CFG":
        configsections.append(isec)
log.info(configsections)
if len(configsections) == 0:
    log.error('No config sections')
    sys.exit(1)

# create ConfigParser object for dacSlopesGen.cfg
outcfg = ConfigParser.SafeConfigParser()

# set global parms
outcfg.add_section("energies")
outcfg.set("energies","lac_low","0.0")
outcfg.set("energies","lac_high","4.0")
outcfg.set("energies","fle_low","50.0")
outcfg.set("energies","fle_high","150.0")
outcfg.set("energies","fhe_low","900.0")
outcfg.set("energies","fhe_high","1100.0")

outcfg.add_section("gainfiles")
outcfg.set("gainfiles","muslope",muslope_path)

outcfg.add_section("dtdfiles")
outcfg.set("dtdfiles","dtdfile",dtd_path)

outcfg.add_section("adcfiles")
outcfg.set("adcfiles", "bias", bias_path)

# loop over detectors
for idet in detsections:
    # get input file names, src and dest towers, desired config
    log.info(idet)
    flename = None
    fhename = None
    lacname = None
    uldname = None
    srctower = None
    desttower = None
    config = None

    doptions = cfile.options(idet)
    for opt in doptions:
        if opt == 'flename':
            flename = cfile.get(idet,opt)
        elif opt == 'fhename':
            fhename = cfile.get(idet,opt)
        elif opt == 'lacname':
            lacname = cfile.get(idet,opt)
        elif opt == 'uldname':
            uldname = cfile.get(idet,opt)
        elif opt == 'src':
            srctower = int(cfile.get(idet,opt))
        elif opt == 'dest':
            desttower = int(cfile.get(idet,opt))
        elif opt == 'config':
            config = cfile.get(idet,opt)
            config = config.strip()
            config = 'CFG'+config

    # make sure all file names, towers, config exist
    
    if flename is None:
        log.error('%s fle file missing', idet)
        sys.exit(1)
    if fhename is None:
        log.error('%s fhe file missing', idet)
        sys.exit(1)
    if lacname is None:
        log.error('%s lac file missing', idet)
        sys.exit(1)
    if uldname is None:
        log.error('%s uld file missing', idet)
        sys.exit(1)
    if srctower is None:
        log.error('%s source tower missing', idet)
        sys.exit(1)
    if desttower is None:
        log.error('%s dest tower missing', idet)
        sys.exit(1)
    

    # populate output cfg file
    
    outcfg.set("adcfiles","fle2adc_%d"%desttower,   flename+",%d"%srctower)
    outcfg.set("adcfiles","fhe2adc_%d"%desttower,   fhename+",%d"%srctower)
    outcfg.set("adcfiles","lac2adc_%d"%desttower,   lacname+",%d"%srctower)
    outcfg.set("adcfiles","uld2adc_%d"%desttower,   uldname+",%d"%srctower)
    
    

outfile = open(output_path,'w')
outcfg.write(outfile)
sys.exit(0)
