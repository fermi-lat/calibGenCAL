"""
Builds configuration file for each gensettings run (for each setting and module).  Builds batch file of commands to run gensettings scripts

note:
    currently only runs on windows
"""

__facility__  = "Offline"
__abstract__  = "Prepares config and commands to run gensettings scripts"
__author__    = "M.Strickman"
__date__      = "$Date: 2005/09/30 19:09:08 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import logging
import string
import ConfigParser
from optparse import OptionParser
from time import gmtime,strftime

# create time tag for this run

timetag = strftime('%y%m%d%H%M%S',gmtime())

# set up logging
logging.basicConfig()
log = logging.getLogger('gensettings')
log.setLevel(logging.INFO)
# see if cfg file for input is specified in cmd line
parser = OptionParser()
parser.add_option('-f','--file',dest='fileroot')
(options, args) = parser.parse_args()
if options.fileroot is None:
    fileroot = 'gensettings'
else:
    fileroot = options.fileroot

log.info(' fileroot: %s',fileroot)

# set up config file and read in sections
cfilename = fileroot+'.cfg'
log.info("Reading file %s", cfilename)
cfile = ConfigParser.SafeConfigParser()
cfile.read(cfilename)
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
# open batch file for run commands
cmdbat = open(fileroot+".bat","w")
cmdbat.write("if not defined CALIBGENCALROOT goto :ERROR\n")
cmdbat.write("setlocal\n")
cmdbat.write(r"set PYTHONROOT=%CALIBGENCALROOT%\python;%PYTHONROOT%;"+"\n")

# loop over detectors
for idet in detsections:
# get input file names, src and dest towers, desired config
    log.info(idet)
    biasname = None
    adc2nrgname = None
    relgainname = None
    flename = None
    fhename = None
    lacname = None
    uldname = None
    srctower = None
    desttower = None
    config = None

    doptions = cfile.options(idet)
    for opt in doptions:
        if opt == 'biasname':
            biasname = cfile.get(idet,opt)
        elif opt == 'adc2nrgname':
            adc2nrgname = cfile.get(idet,opt)
        elif opt == 'relgainname':
            relgainname = cfile.get(idet,opt)
        elif opt == 'flename':
            flename = cfile.get(idet,opt)
        elif opt == 'fhename':
            fhename = cfile.get(idet,opt)
        elif opt == 'lacname':
            lacname = cfile.get(idet,opt)
        elif opt == 'uldname':
            uldname = cfile.get(idet,opt)
        elif opt == 'src':
            srctower = cfile.get(idet,opt)
        elif opt == 'dest':
            desttower = cfile.get(idet,opt)
        elif opt == 'config':
            config = cfile.get(idet,opt)
            config = config.strip()
            config = 'CFG'+config

# make sure all file names, towers, config exist
    if biasname is None:
        log.error('%s bias file missing', idet)
        sys.exit(1)
    if adc2nrgname is None:
        log.error('%s adc2nrg file missing', idet)
        sys.exit(1)
    if relgainname is None:
        log.error('%s relgain file missing', idet)
        sys.exit(1)
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
# In the following, if characterization file == "skip", skip test
# Build genFLE config file
    if flename != "skip":
        flecfg = ConfigParser.SafeConfigParser()
        flecfg.add_section("infiles")
        flecfg.set("infiles","adc2nrg",adc2nrgname)
        flecfg.set("infiles","relgain",relgainname)
        flecfg.set("infiles","fle2adc",flename)
        flecfg.set("infiles","bias",biasname)
        flecfg.add_section("gains")
        flecfg.set("gains","legain",legain)
        flecfg.add_section("towers")
        flecfg.set("towers","srctower",srctower)
        flecfg.set("towers","desttower",desttower)
# Write out genFLE config file
        fleout = open(fileroot+'_'+idet+"genFLE.cfg","w")
        flecfg.write(fleout)
        fleout.close()
        log.info("genFLE")
# Write out run command to batch file
        cmdline = r"python %CALIBGENCALROOT%\python\genFLEsettings.py -V "+\
                  fle+" "+fileroot+'_'+idet+"genFLE.cfg "+timetag+"_G"+legain.strip()+\
                  "_MeV"+fle.strip()+"_"+idet+"_CAL_fle.xml\n"
        cmdbat.write(cmdline)

# Build genFHE config file
    if fhename != "skip":
        fhecfg = ConfigParser.SafeConfigParser()
        fhecfg.add_section("infiles")
        fhecfg.set("infiles","adc2nrg",adc2nrgname)
        fhecfg.set("infiles","relgain",relgainname)
        fhecfg.set("infiles","fhe2adc",fhename)
        fhecfg.set("infiles","bias",biasname)
        fhecfg.add_section("gains")
        fhecfg.set("gains","hegain",hegain)
        fhecfg.add_section("towers")
        fhecfg.set("towers","srctower",srctower)
        fhecfg.set("towers","desttower",desttower)
# Write out genfhe config file
        fheout = open(fileroot+'_'+idet+"genFHE.cfg","w")
        fhecfg.write(fheout)
        fheout.close()
        log.info("genFHE")
# Write out run command to batch file
        cmdline = r"python %CALIBGENCALROOT%\python\genFHEsettings.py -V "+\
                  fhe+" "+fileroot+'_'+idet+"genFHE.cfg "+timetag+"_G"+hegain.strip()+\
                  "_GeV"+fhe.strip()+"_"+idet+"_CAL_fhe.xml\n"
        cmdbat.write(cmdline)
# Do FHE again with muon gain (only script we do this with)
        fhecfg.remove_option("gains","hegain")
        fhecfg.set("gains","hegain",hegainmu)
# Write out genfhe (muon gain) config file
        fheout = open(fileroot+'_'+idet+"genFHE_muon.cfg","w")
        fhecfg.write(fheout)
        fheout.close()
        log.info("genFHE_muon")
# Write out run command to batch file
        cmdline = r"python %CALIBGENCALROOT%\python\genFHEsettings.py -V "+\
                  fhe+" "+fileroot+'_'+idet+"genFHE_muon.cfg "+timetag+"_G"+hegainmu.strip()+\
                  "_GeV"+fhe.strip()+"_"+idet+"_CAL_fhe.xml\n"
        cmdbat.write(cmdline)
        
# Build genLAC config file
    if lacname != "skip":
        laccfg = ConfigParser.SafeConfigParser()
        laccfg.add_section("infiles")
        laccfg.set("infiles","adc2nrg",adc2nrgname)
        laccfg.set("infiles","relgain",relgainname)
        laccfg.set("infiles","lac2adc",lacname)
        laccfg.add_section("gains")
        laccfg.set("gains","legain",legain)
        laccfg.add_section("towers")
        laccfg.set("towers","srctower",srctower)
        laccfg.set("towers","desttower",desttower)
# Write out genlac config file
        lacout = open(fileroot+'_'+idet+"genLAC.cfg","w")
        laccfg.write(lacout)
        lacout.close()
        log.info("genLAC")
# Write out run command to batch file
        cmdline = r"python %CALIBGENCALROOT%\python\genLACsettings.py -V "+\
                  lac+" "+fileroot+'_'+idet+"genLAC.cfg "+timetag+"_G"+legain.strip()+\
                  "_MeV"+lac.strip()+"_"+idet+"_CAL_lac.xml\n"
        cmdbat.write(cmdline)

# Build genULD config file
    if uldname != "skip":
        uldcfg = ConfigParser.SafeConfigParser()
        uldcfg.add_section("infiles")
        uldcfg.set("infiles","uld2adc",uldname)
        uldcfg.add_section("margins")
        uldcfg.set("margins","adcmargin",adcmargin)
        uldcfg.add_section("towers")
        uldcfg.set("towers","srctower",srctower)
        uldcfg.set("towers","desttower",desttower)
# Write out genuld config file
        uldout = open(fileroot+'_'+idet+"genULD.cfg","w")
        uldcfg.write(uldout)
        uldout.close()
        log.info("genULD")
# Write out run command to batch file
        cmdline = r"python %CALIBGENCALROOT%\python\genULDsettings.py -V "+\
                  fileroot+'_'+idet+"genULD.cfg "+timetag+"_adc"+adcmargin.strip()+"_"+\
                  idet+"_CAL_uld.xml\n"
        cmdbat.write(cmdline)

# Close batch file and end
cmdbat.write("endlocal\n")
cmdbat.write("goto :EXIT\n")
cmdbat.write(":ERROR\n")
cmdbat.write("echo ERROR: CALIBGENCALROOT must be defined\n")
cmdbat.write(":EXIT\n")
cmdbat.close()
sys.exit(0)
