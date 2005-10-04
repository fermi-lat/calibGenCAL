"""
Identifies files required for each gensettings script and uses the filenames to build the configuration file for the gensettings.py script

note:
    see calibGenCAL/doc/gensettings_scripts.html for more information.

"""

__facility__  = "Offline"
__abstract__  = "Identifies files and builds config file for gensettings.py"
__author__    = "M.Strickman"
__date__      = "$Date: 2005/09/30 20:58:34 $"
__version__   = "$Revision: 1.5 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import logging
import string
import ConfigParser
import glob
from optparse import OptionParser

usage = 'build_gensettings_cfg [-f fileroot][--file fileroot]'

# finds files for gensettings and creates cfg file
# NOTE:  folder must contain only one file for ea module and type
# for the desired gensettings cfg file. Script has no way of distinguishing
# multiple files of the same module and type but different dates

# set up logging
logging.basicConfig()
log = logging.getLogger('build_gensettings_cfg')
log.setLevel(logging.INFO)

# see if cfg file for input is specified in cmd line

parser = OptionParser()
parser.add_option('-f','--file',dest='fileroot')
(options, args) = parser.parse_args()
if options.fileroot is None:
    fileroot = 'build_gensettings_cfg'
else:
    fileroot = options.fileroot

log.info(' fileroot: %s',fileroot)

# set up input config file and read in sections

cfilename = fileroot+'.cfg'
log.info(" Reading file %s", cfilename)
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

# find and read output section

outputsection = 'OUTPUT'

outfilepath = None
outfilename = None

if cfile.has_section(outputsection):
	outoptions = cfile.options(outputsection)
	for opt in outoptions:
		if opt == 'outfilepath':
			outfilepath = cfile.get(outputsection,opt)
		if opt == 'outfilename':
			outfilename = cfile.get(outputsection,opt)

if outfilepath is None:
    outfilepath = '.'
if outfilename is None:
    outfilename = 'gensettings.cfg'

# set up output config file

outcfg = ConfigParser.SafeConfigParser()

# loop over modules

for idet in detsections:

# read module setups

    log.info(idet)
    filepath = None
    config = None
    tower = None
    biasname = None
    adc2nrgname = None
    relgainname = None
    flename = None
    fhename = None
    lacname = None
    uldname = None

    doptions = cfile.options(idet)
    for opt in doptions:
        if opt == 'filepath':
            filepath = cfile.get(idet,opt)
        elif opt == 'config':
            config = cfile.get(idet,opt)
        elif opt == 'tower':
            tower = cfile.get(idet,opt)

# the following are overrides for automatic filename searching (optional)

        elif opt == 'biasname':
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


# check if all module items are present (error out if not)

    if filepath is None:
        log.error(' %s filepath missing',idet)
        sys.exit(1)
    if config is None:
        log.warning(' %s set config to default = 1',idet)
        config = '1'
    if tower is None:
        log.error(' %s tower number missing',idet)
        sys.exit(1)

# check if specified config exists in setup file

    if 'CFG'+config not in configsections:
        log.error(' %s: specified config not in available sections',idet)
        sys.exit(1)
        
# create a tower string for finding the adc2nrg file

    if int(tower) < 10:
        towerstr = '0'+tower.strip()
    elif int(tower) >= 10:
        towerstr = tower.strip()
        
# get each file type required for this module from the filepath
# Note the required name patterns.  If the filenames do not conform,
# either change them or use override

    if biasname is None:
        biasname = glob.glob(filepath+'/*'+idet+'*[bB]ias*.xml')
    if adc2nrgname is None:
        adc2nrgname = glob.glob(filepath+'/*T'+towerstr+'*adc2nrg*.xml')

# in case adc2nrg uses module number rather than tower...

        if len(adc2nrgname) == 0:
            adc2nrgname = glob.glob(filepath+'/*'+idet+'*adc2nrg*.xml')    
    if relgainname is None:
        relgainname = glob.glob(filepath+'/*'+idet+'*relgain*.xml')
    if flename is None:
        flename = glob.glob(filepath+'/*'+idet+'*fle2adc_filtered*.xml')
    if fhename is None:
        fhename = glob.glob(filepath+'/*'+idet+'*fhe2adc_filtered*.xml')
    if lacname is None:
        lacname = glob.glob(filepath+'/*'+idet+'*lac2adc_filtered*.xml')
    if uldname is None:
        uldname = glob.glob(filepath+'/*'+idet+'*uld2adc_filtered*.xml')

# if any name lists either empty or more than 1, report and exit with error

    listflg = 0
    if len(biasname) != 1:
        log.error(' %s entries in biasname list != 1',len(biasname))
        listflg = 1
    if len(adc2nrgname) != 1:
        log.error(' %s entries in adc2nrgname list != 1',len(adc2nrgname))
        listflg = 1
    if len(relgainname) != 1:
        log.error(' %s entries in relgainname list != 1',len(relgainname))
        listflg = 1
    if len(flename) != 1:
        log.error(' %s entries in flename list != 1',len(flename))
        listflg = 1
    if len(fhename) != 1:
        log.error(' %s entries in fhename list != 1',len(fhename))
        listflg = 1
    if len(lacname) != 1:
        log.error(' %s entries in lacname list != 1',len(lacname))
        listflg = 1
    if len(uldname) != 1:
        log.error(' %s entries in uldname list != 1',len(uldname))
        listflg = 1
    if listflg:
        sys.exit(1)

# add and populate section to output config file

    outcfg.add_section(idet)
    outcfg.set(idet,'biasname',biasname[0])
    outcfg.set(idet,'adc2nrgname',adc2nrgname[0])
    outcfg.set(idet,'relgainname',relgainname[0])
    outcfg.set(idet,'flename',flename[0])
    outcfg.set(idet,'fhename',fhename[0])
    outcfg.set(idet,'lacname',lacname[0])
    outcfg.set(idet,'uldname',uldname[0])

    outcfg.set(idet,'src',tower)
    outcfg.set(idet,'dest',tower)
    outcfg.set(idet,'config',config)

# loop through CFG sections of input config and copy to output

for icfg in configsections:
    log.info(icfg)

    legain = None
    hegain = None
    hegainmu = None
    lac = None
    fle = None
    fhe = None
    adcmargin = None

# read in config information for this CFG setup

    coptions = cfile.options(icfg)
    for opt in coptions:
        if opt == 'legain':
            legain = cfile.get(icfg,opt)
        elif opt == 'hegain':
            hegain = cfile.get(icfg,opt)
        elif opt == 'hegainmu':
            hegainmu = cfile.get(icfg,opt)
        elif opt == 'fle':
            fle = cfile.get(icfg,opt)
        elif opt == 'fhe':
            fhe = cfile.get(icfg,opt)
        elif opt == 'lac':
            lac = cfile.get(icfg,opt)
        elif opt == 'adcmargin':
            adcmargin = cfile.get(icfg,opt)

# check if all config info present

    if legain is None:
        log.error('%s legain missing', icfg)
        sys.exit(1)
    if hegain is None:
        log.error('%s hegain missing', icfg)
        sys.exit(1)
    if hegainmu is None:
        log.error('%s hegainmu missing',icfg)
        sys.exit(1)
    if fle is None:
        log.error('%s fle missing', icfg)
        sys.exit(1)
    if fhe is None:
        log.error('%s fhe missing', icfg)
        sys.exit(1)
    if lac is None:
        log.error('%s lac missing', icfg)
        sys.exit(1)
    if adcmargin is None:
        log.error('%s adcmargin missing', icfg)
        sys.exit(1)

# write config section

    outcfg.add_section(icfg)
    outcfg.set(icfg,'legain',legain)
    outcfg.set(icfg,'hegain',hegain)
    outcfg.set(icfg,'hegainmu',hegainmu)
    outcfg.set(icfg,'lac',lac)
    outcfg.set(icfg,'fle',fle)
    outcfg.set(icfg,'fhe',fhe)
    outcfg.set(icfg,'adcmargin',adcmargin)

# write output file and terminate

gensetout = open(outfilepath+'/'+outfilename,'w')
outcfg.write(gensetout)
gensetout.close()

sys.exit(0)

