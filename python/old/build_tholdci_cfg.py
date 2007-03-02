"""
Builds configuration file for tholdCIGen.py script from info in gensettings.py cfg file

Usage: build_tholdci_cfg [-b bias.xml] [-g gensettings.cfg] [-t dacfile_timetag] [-d dtdfile.dtd] [-p ped_dir] intnonlin.xml output_tholdci.cfg

Note: this program depends on input filenames w/ certain patterns.  Filename conventions change from time to time.  we try to keep the scripts current, so
you may have to modify the glob matches to work w/ older files.

where:
       intnonlin.xml      = offline adc->dac intNonlin calibration file.  (req'd as it is not specified in gensettings.cfg file)
       bias.xml           = trigger bias file (default is to glob for file in current directory)
       gensettings.cfg    = name of input gensettings.py cfg file (default = "gensettings.cfg")
       dacfile_timetag    = id string at front of all input dac settings filenames (usually 12 digit date string, could be 'latest') (default = "latest")
       dtdfile            = name of dtd file to use in output (will be found in "$CALIBUTILROOT/xml/Cal" )
       ped_dir       = folder in which to find pedestal files (default = "./" )
       output_tholdci.cfg = output cfg file for tholdCIGen.py script

note:
    see calibGenCAL/doc/gensettings_scripts.html for more information.

"""

__facility__  = "Offline"
__abstract__  = "Builds configuration file for tholdCIGen.py script from info in gensettings.py cfg file"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/07/28 01:42:57 $"
__version__   = "$Revision: 1.7 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import ConfigParser
import getopt
import glob

# set up logging
logging.basicConfig()
log = logging.getLogger('build_tholdci_cfg')
log.setLevel(logging.INFO)


# retrieve commandline parms
usage = "Usage: build_tholdci_cfg [-b bias.xml] [-g gensettings.cfg] [-t dacfile_timetag] [-d dtdfile.dtd] intnonlin.xml output_tholdci.cfg"
# check command line
try:
    (opts,args) = getopt.getopt(sys.argv[1:], "b:d:g:t:p:")
except getopt.GetoptError:
    log.error(usage)
    sys.exit(1)

# defaults
cfg_path           = "gensettings.cfg"
dacfile_timetag    = "latest"
dtd_path           = "calCalib_v2r3.dtd"
ped_dir            = "./"

# try to find bias file

bias_path = None
biasList = glob.glob('*[bB]ias*.xml')
if len(biasList) == 1:
    bias_path = biasList[0]

for (o,a) in opts:
    if o == '-b':
        bias_path = a
    elif o == '-d':
        dtd_path = a
    elif o == '-g':
        cfg_path = a
    elif o == '-t':
        dacfile_timetag = a
    elif o == '-p':
        ped_dir = a
        
if len(args) != 2:
    log.error(usage)
    sys.exit(1)

intlin_path        = args[0]
output_path        = args[1]

if bias_path is None:
    log.error("No bias file found or specified")
    sys.exit(1)

def gen_basename(dacfile_timetag, gain, mev, idet, dac_type):
    """ generate dac setting filename (sans extension for non-uld dac settings) """
    return  "%s_G%d_MeV%d_%s_CAL_%s"%(dacfile_timetag, int(gain), int(mev), idet, dac_type)
    
def gen_basename_uld(dacfile_timetag, adcmargin, idet):
    """ generate dac setting filename (sans extension) for uld dac settings"""
    return "%s_adc%d_%s_CAL_uld"%(dacfile_timetag, int(adcmargin), idet)


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
    
legain = cfile.get(configsections[0], "legain")
hegainmu = cfile.get(configsections[0], "hegainmu")
if legain is None:
    log.error('%s legain missing', config)
    sys.exit(1)
if hegainmu is None:
    log.error('%s hegainmu missing', config)
    sys.exit(1)

# create ConfigParser object for tholdCI.cfg
outcfg = ConfigParser.SafeConfigParser()

# set global parms

outcfg.add_section("gains")
outcfg.set("gains", "legain", legain)
outcfg.set("gains", "hegain", hegainmu)

outcfg.add_section("dacfiles")

outcfg.add_section("adcfiles")
outcfg.set("adcfiles","intnonlin",intlin_path)
outcfg.set("adcfiles", "bias", bias_path)

outcfg.add_section("dtdfiles")

outcfg.set("dtdfiles","dtdfile",dtd_path)

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
    if config is None:
        log.error('%s config missing', idet)
        sys.exit(1)
    # is desired cfg section present?
    if not cfile.has_section(config):
        log.error('%s config section %s',idet,config)
        sys.exit(1)

    # search for pedestal.xml
    pedname = glob.glob("%s/*pedestals_%s.xml"%(ped_dir,idet))[0];
    

    # read in configuration parameters
    legain = None
    hegain = None
    hegainmu = None
    fle = None
    fhe = None
    lac = None
    adcmargin = None
    
    coptions = cfile.options(config)
    for opt in coptions:
        if opt == 'legain':
            legain = cfile.get(config,opt)
        elif opt == 'hegain':
            hegain = cfile.get(config,opt)
        elif opt == 'hegainmu':
            hegainmu = cfile.get(config,opt)
        elif opt == 'fle':
            fle = cfile.get(config,opt)
        elif opt == 'fhe':
            fhe = cfile.get(config,opt)
        elif opt == 'lac':
            lac = cfile.get(config,opt)
        elif opt == 'adcmargin':
            adcmargin = cfile.get(config,opt)
    if legain is None:
        log.error('%s legain missing', config)
        sys.exit(1)
    if hegain is None:
        log.error('%s hegain missing', config)
        sys.exit(1)
    if hegainmu is None:
        log.error('%s hegainmu missing',config)
        sys.exit(1)
    if fle is None:
        log.error('%s fle missing', config)
        sys.exit(1)
    if fhe is None:
        log.error('%s fhe missing', config)
        sys.exit(1)
    if lac is None:
        log.error('%s lac missing', config)
        sys.exit(1)
    if adcmargin is None:
        log.error('%s adcmargin missing', config)
        sys.exit(1)

    # populate output cfg file
    
    outcfg.set("adcfiles","fle2adc_%d"%desttower,   flename+",%d"%srctower)
    outcfg.set("adcfiles","fhe2adc_%d"%desttower,   fhename+",%d"%srctower)
    outcfg.set("adcfiles","lac2adc_%d"%desttower,   lacname+",%d"%srctower)
    outcfg.set("adcfiles","uld2adc_%d"%desttower,   uldname+",%d"%srctower)
    outcfg.set("adcfiles","pedestals_%d"%desttower, pedname+",%d"%srctower)

    lac_adc_path = gen_basename(dacfile_timetag, legain,   float(lac), idet, "lac")+".xml"
    fle_adc_path = gen_basename(dacfile_timetag, legain,   float(fle), idet, "fle")+".xml"
    fhe_adc_path = gen_basename(dacfile_timetag, hegainmu, float(fhe) * 1000, idet, "fhe")+".xml"
    uld_adc_path = gen_basename_uld(dacfile_timetag, adcmargin, idet)+".xml"

    outcfg.set("dacfiles","lac_%d"%desttower,lac_adc_path+",%d"%desttower)
    outcfg.set("dacfiles","fle_%d"%desttower,fle_adc_path+",%d"%desttower)
    outcfg.set("dacfiles","fhe_%d"%desttower,fhe_adc_path+",%d"%desttower)
    outcfg.set("dacfiles","uld_%d"%desttower,uld_adc_path+",%d"%desttower)


outfile = open(output_path,'w')
outcfg.write(outfile)
sys.exit(0)