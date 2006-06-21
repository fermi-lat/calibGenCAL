"""
Builds configuration file for each gensettings run (for each setting and module).  Builds .bat && .sh file of commands to run gensettings scripts

note:
    see calibGenCAL/doc/gensettings_scripts.html for more information.

"""

__facility__  = "Offline"
__abstract__  = "Prepares config and commands to run gensettings scripts"
__author__    = "M.Strickman"
__date__      = "$Date: 2006/05/01 19:40:06 $"
__version__   = "$Revision: 1.12 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import logging
import string
import ConfigParser
from optparse import OptionParser
from time import gmtime,strftime


def gen_basename(timetag, gain, mev, idet, dac_type):
    """ generate output filename (sans extension for non-uld dac settings) """
    return  "%s_G%d_MeV%d_%s_CAL_%s"%(timetag, int(gain), int(mev), idet, dac_type)
    
def gen_basename_uld(timetag, adcmargin, idet):
    """ generate output filename (sans extension) for uld dac settings"""
    return "%s_adc%d_%s_CAL_uld"%(timetag, int(adcmargin), idet)

def gen_cfgname(fileroot, idet, dac_type):
    """ generate cf filename for given dac type"""
    return "%s_%sgen%s.cfg"%(fileroot,idet,dac_type)

usage = 'gensettings [-f fileroot | --file fileroot] [-m | --muon]'

# create time tag for this run

timetag = strftime('%y%m%d%H%M%S',gmtime())

# set up logging
logging.basicConfig()
log = logging.getLogger('gensettings')
log.setLevel(logging.INFO)

# see if cfg file for input is specified in cmd line
parser = OptionParser(usage = usage)
parser.add_option('-f','--file',dest='fileroot')
parser.add_option('-m','--muon',dest='muon',action='store_true',default=False)
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
cmdbat = open("run_"+fileroot+".bat","w")
cmdbat.write("if not defined CALIBGENCALROOT goto :ERROR\n")
cmdbat.write("setlocal\n")
cmdbat.write("set PYTHONPATH=%CALIBGENCALROOT%/python/lib;%PYTHONROOT%;"+"\n")

# open .sh file for run commands
cmdsh = open("run_"+fileroot+".sh", "w")
cmdsh.write("#! /bin/sh\n")
cmdsh.write("PYTHONPATH=${CALIBGENCALROOT}/python/lib:${PYTHONPATH}\n")
cmdsh.write("export PYTHONPATH\n")

CALIBGENCALROOT = os.environ["CALIBGENCALROOT"]

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
    fhegev = None
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
            fle = cfile.getfloat(config,opt)
        elif opt == 'fhe':
            fhegev = cfile.getfloat(config,opt)
        elif opt == 'lac':
            lac = cfile.getfloat(config,opt)
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
    if fhegev is None:
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
        cfgname  = gen_cfgname(fileroot, idet, "FLE")
        fleout = open(cfgname,"w")
        flecfg.write(fleout)
        fleout.close()
        log.info("genFLE")

        # generate base filename used in all output files
        basename    = gen_basename(timetag, legain, fle, idet, "fle")
        latest_base = gen_basename("latest", legain, fle, idet, "fle")


        # Write out run command to batch file
        cmdline = "python %s/python/genFLEsettings.py -V %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                           float(fle),
                                                                           cfgname,
                                                                           basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)


        # Write out dacVal command to batch file
        cmdline = "python %s/python/dacVal.py -V -R %s.root -L %s.val.log FLE %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                                                basename,
                                                                                                basename,
                                                                                                float(fle),
                                                                                                cfgname,
                                                                                                basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)

        # Write out cp -> "latestXXX.xml" command to batch file
        cmdline = "copy %s.xml %s.xml\n"%(basename, latest_base)
        cmdbat.write(cmdline)
        cmdline = "cp %s.xml %s.xml\n"%(basename, latest_base)
        cmdsh.write(cmdline)


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
        cfgname  = gen_cfgname(fileroot, idet, "FHE")
        fheout   = open(cfgname,"w")
        fhecfg.write(fheout)
        fheout.close()
        log.info("genFHE")

        # generate base filename used in all output files
        basename    = gen_basename(timetag, hegain, fhegev*1000, idet, "fhe")
        latest_base = gen_basename("latest", hegain, fhegev*1000, idet, "fhe")

        # Write out run command to batch file
        cmdline = "python %s/python/genFHEsettings.py -V %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                          float(fhegev),
                                                                          cfgname,
                                                                          basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)

        # Write out dacVal command to batch file
        cmdline = "python %s/python/dacVal.py -V -R %s.root -L %s.val.log FHE %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                                                basename,
                                                                                                basename,
                                                                                                float(fhegev*1000),
                                                                                                cfgname,
                                                                                                basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)

        # Write out cp -> "latestXXX.xml" command to batch file
        cmdline = "copy %s.xml %s.xml\n"%(basename, latest_base)
        cmdbat.write(cmdline)
        cmdline = "cp %s.xml %s.xml\n"%(basename, latest_base)
        cmdsh.write(cmdline)


# Do FHE again with muon gain (optional, only script we do this with)
        if options.muon:
            fhecfg.remove_option("gains","hegain")
            fhecfg.set("gains","hegain",hegainmu)
            # Write out genfhe (muon gain) config file
            cfgname  = gen_cfgname(fileroot, idet, "FHE_muon")
            fheout = open(cfgname,"w")
            fhecfg.write(fheout)
            fheout.close()
            log.info("genFHE_muon")

            # generate base filename used in all output files
            basename    = gen_basename(timetag, hegainmu, fhegev*1000, idet, "fhe")
            latest_base = gen_basename("latest", hegainmu, fhegev*1000, idet, "fhe")


            # Write out run command to batch file
            # Write out run command to batch file
            cmdline = "python %s/python/genFHEsettings.py -V %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                          float(fhegev),
                                                                          cfgname,
                                                                          basename)
            cmdbat.write(cmdline)
            cmdsh.write(cmdline)

            # Write out dacVal command to batch file
            cmdline = "python %s/python/dacVal.py -V -R %s.root -L %s.val.log FHE %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                                                basename,
                                                                                                basename,
                                                                                                float(fhegev*1000),
                                                                                                cfgname,
                                                                                                basename)
            cmdbat.write(cmdline)
            cmdsh.write(cmdline)

            # Write out cp -> "latestXXX.xml" command to batch file
            cmdline = "copy %s.xml %s.xml\n"%(basename, latest_base)
            cmdbat.write(cmdline)
            cmdline = "cp %s.xml %s.xml\n"%(basename, latest_base)
            cmdsh.write(cmdline)

        
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
        cfgname  = gen_cfgname(fileroot, idet, "LAC")
        lacout = open(cfgname,"w")
        laccfg.write(lacout)
        lacout.close()
        log.info("genLAC")

        # generate base filename used in all output files
        basename = gen_basename(timetag, legain, lac, idet, "lac")
        latest_base = gen_basename("latest", legain, lac, idet, "lac")


        # Write out run command to batch file
        cmdline = "python %s/python/genLACsettings.py -V %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                          float(lac),
                                                                          cfgname,
                                                                          basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)


        # Write out dacVal command to batch file
        cmdline = "python %s/python/dacVal.py -V -R %s.root -L %s.val.log LAC %f %s %s.xml\n"%(CALIBGENCALROOT,
                                                                                                basename,
                                                                                                basename,
                                                                                                float(lac),
                                                                                                cfgname,
                                                                                                basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)

        # Write out cp -> "latestXXX.xml" command to batch file
        cmdline = "copy %s.xml %s.xml\n"%(basename, latest_base)
        cmdbat.write(cmdline)
        cmdline = "cp %s.xml %s.xml\n"%(basename, latest_base)
        cmdsh.write(cmdline)


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
        cfgname  = gen_cfgname(fileroot, idet, "ULD")
        uldout = open(cfgname,"w")
        uldcfg.write(uldout)
        uldout.close()
        log.info("genULD")

        basename    = gen_basename_uld(timetag, adcmargin, idet)
        latest_base = gen_basename_uld("latest", adcmargin, idet)

        # Write out run command to batch file
        cmdline = "python %s/python/genULDsettings.py -V %s %s.xml\n"%(CALIBGENCALROOT,
                                                                        cfgname,
                                                                        basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)
        
        # Write out dacVal command to batch file
        cmdline = "python %s/python/uldVal.py -V -R %s.root -L %s.val.log %s %s.xml\n"%(CALIBGENCALROOT,
                                                                                                basename,
                                                                                                basename,
                                                                                                cfgname,
                                                                                                basename)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)

        # Write out cp -> "latestXXX.xml" command to batch file
        cmdline = "copy %s.xml %s.xml\n"%(basename, latest_base)
        cmdbat.write(cmdline)
        cmdline = "cp %s.xml %s.xml\n"%(basename, latest_base)
        cmdsh.write(cmdline)



# Close batch file and end
cmdbat.write("endlocal\n")
cmdbat.write("goto :EXIT\n")
cmdbat.write(":ERROR\n")
cmdbat.write("echo ERROR: CALIBGENCALROOT must be defined\n")
cmdbat.write(":EXIT\n")
cmdbat.close()
cmdsh.close()
sys.exit(0)
