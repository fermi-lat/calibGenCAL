/"""/
/given the calibDAC suitesummary run number for each module of interest, retrieve all calibDAC files from slac using scp/

/note:/
/    currently only runs on windows/
/"""/

__facility__  = "Offline"
__abstract__  = "Retrieve DAC settings files from SLAC using SCP"
__author__    = "M.Strickman"
__date__      = "$Date: 2005/09/29 22:08:26 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"





import sys, os
import logging
import string
import ConfigParser

# given the calibDAC suitesummary run number for each module of interest,
# retreive all calibDAC files from slac using scp and build gensettings.py
# configuration file

# set up logging

logging.basicConfig()
log = logging.getLogger('get_slac_calibDAC')
log.setLevel(logging.INFO)

# set up input config file and read in sections (one per CAL module)

icfilename = r'get_slac_calibdac.cfg'
log.info(' Reading file %s',icfilename)
icfile = ConfigParser.SafeConfigParser()
icfile.read(icfilename)
isections = icfile.sections()
log.info(isections)
modsections = []
slacuser = None
batfileroot = None

# identify SETUP section and individual module sections, load info from each

for isec in isections:
    if isec[0:2] == 'FM':
        modsections.append(isec)
    elif isec == 'SETUP':
        sshoptions = icfile.options(isec)
        for opt in sshoptions:
            if opt == 'slacuser':
                slacuser = icfile.get(isec,opt)
            elif opt == 'batfileroot':
                batfileroot = icfile.get(isec,opt)
if slacuser is None:
    log.error(' Must enter slac user name in [SETUP] section of config file')
    sys.exit(1)
if batfileroot is None:
    batfileroot = 'scpcmds'
    log.warning(' Using default batch filename root: scpcmds')
log.info(' slacuser: '+slacuser)
log.info(modsections)

# open batch file to receive scp commands

cmdbat = open(batfileroot+'.bat','w')

# loop over input CAL modules

for isec in modsections:
    log.info(isec)
    summaryrun = None
    relgainrun = None
    fle2adcrun = None
    fhe2adcrun = None
    lac2adcrun = None
    uld2adcrun = None
    disk = None

    options = icfile.options(isec)
    for opt in options:
        if opt == 'summary':
            summaryrun = icfile.get(isec,opt)
        elif opt == 'relgain':
            relgainrun = icfile.get(isec,opt)
        elif opt == 'fle2adc':
            fle2adcrun = icfile.get(isec,opt)
        elif opt == 'fhe2adc':
            fhe2adcrun = icfile.get(isec,opt)
        elif opt == 'lac2adc':
            lac2adcrun = icfile.get(isec,opt)
        elif opt == 'uld2adc':
            uld2adcrun = icfile.get(isec,opt)
        elif opt == 'disk':
            disk = icfile.get(isec,opt)

# if summaryrun is specified, construct run numbers unless overridden

    if summaryrun is not None:
        srint = int(summaryrun)
        if uld2adcrun is None:
            uld2adcrun = str(srint-1)
        if lac2adcrun is None:
            lac2adcrun = str(srint-2)
        if fhe2adcrun is None:
            fhe2adcrun = str(srint-3)
        if fle2adcrun is None:
            fle2adcrun = str(srint-4)
        if relgainrun is None:
            relgainrun = str(srint-5)
        if disk is None:
            disk = 'u21'

# add scp commands to output batch file

    if relgainrun != 'skip':
        cmdbat.write(r'scp '+slacuser+'@glast01.slac.stanford.edu:'+\
                 '/nfs/farm/g/glast/'+disk+'/Integration/rawData/'+\
                 relgainrun+r'/*relgain*.xml .'+'\n')
    if fle2adcrun != 'skip':
            cmdbat.write(r'scp '+slacuser+'@glast01.slac.stanford.edu:'+\
                 '/nfs/farm/g/glast/'+disk+'/Integration/rawData/'+\
                 fle2adcrun+r'/*fle2adc*.xml .'+'\n')
    if fhe2adcrun != 'skip':
            cmdbat.write(r'scp '+slacuser+'@glast01.slac.stanford.edu:'+\
                 '/nfs/farm/g/glast/'+disk+'/Integration/rawData/'+\
                 fhe2adcrun+r'/*fhe2adc*.xml .'+'\n')
    if lac2adcrun != 'skip':
            cmdbat.write(r'scp '+slacuser+'@glast01.slac.stanford.edu:'+\
                 '/nfs/farm/g/glast/'+disk+'/Integration/rawData/'+\
                 lac2adcrun+r'/*[0-9]_FM*lac2adc*.xml .'+'\n')        #ignore smooth files
    if uld2adcrun != 'skip':
            cmdbat.write(r'scp '+slacuser+'@glast01.slac.stanford.edu:'+\
                 '/nfs/farm/g/glast/'+disk+'/Integration/rawData/'+\
                 uld2adcrun+r'/*uld2adc*.xml .'+'\n')
cmdbat.close()
os.system(batfileroot+'.bat > '+batfileroot+'.log 2>&1')
sys.exit(0)    
