"""
build batch file containing commands to run adcsmooth script for filtering configuration files for each module and setting specified in setup file
also runs validation & plotting scripts.

Note:
    - this program depends on input filenames w/ certain patterns.  Filename conventions change from time to time.  we try to keep the scripts current, so
      you may have to modify the glob matches to work w/ older files.
    - see calibGenCAL/doc/gensettings_scripts.html for more information.

"""

__facility__  = "Offline"
__abstract__  = "Build batch file containing commands to run adcsmooth filtering script"
__author__    = "M.Strickman"
__date__      = "$Date: 2006/09/22 20:14:22 $"
__version__   = "$Revision: 1.16 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import stat
import logging
import string
import ConfigParser
import glob
import os.path
from optparse import OptionParser


# finds files for gensettings and creates bat and sh files with adcsmooth commands to produce filtered files
# NOTE:  folder must contain only one file for each module and type
# for the desired gensettings cfg file. Script has no way of distinguishing
# multiple files of the same module and type but different dates

# set up logging
logging.basicConfig()
log = logging.getLogger('build_adcsmooth')
log.setLevel(logging.INFO)

# see if cfg file for input is specified in cmd line

parser = OptionParser()
parser.add_option('-f','--file',dest='fileroot')
(options, args) = parser.parse_args()
if options.fileroot is None:
    fileroot = 'build_adcsmooth'
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

CALIBGENCALROOT = os.environ["CALIBGENCALROOT"]


# open batch files to receive adcsmooth commands

cmdbat = open('adcsmooth.bat','w')
cmdsh = open('adcsmooth.sh', 'w')

cmdsh.write('#!/bin/sh\n')
cmdsh.write('set -v\n')
cmdsh.write('PYTHONPATH=${CALIBGENCALROOT}/python/lib:${ROOTSYS}/bin:${PYTHONPATH}\n')
cmdsh.write('export PYTHONPATH\n')
cmdsh.write('export -n DISPLAY\n')

cmdbat.write('setlocal\n')
cmdbat.write('set PYTHONPATH=%CALIBGENCALROOT%/python/lib;%ROOTSYS%/bin;%PYTHONPATH%\n')

def process_file(filename, validate = True):
    """ create commandlines for adcsmooth, charplot & charVal, append to both cmdbat & cmdsh scripts files """
    filesplit = os.path.splitext(filename)
    smoothname  = filesplit[0]+"_filtered.xml"
    logname     = filesplit[0]+".val.log"
    plotname    = filesplit[0]+".root"
    valplotname = filesplit[0]+".val.root"

    # adcsmooth: .bat
    cmdline = r"python %s/python/adcsmooth.py  "%CALIBGENCALROOT+\
              filename + " " + smoothname + "\n"
    log.info(cmdline)
    cmdbat.write(cmdline)
    cmdsh.write(cmdline)

    # charplot: .bat
    cmdline = r"python %s/python/charplot.py  "%CALIBGENCALROOT+\
              filename + " " + smoothname + " " + plotname + "\n"
    log.info(cmdline)
    cmdbat.write(cmdline)
    cmdsh.write(cmdline)
    if (validate):
        # charVal: .bat
        cmdline = r"python %s/python/charVal.py  "%CALIBGENCALROOT+\
                  " -R " + valplotname + " -L " + logname + " " + smoothname + "\n"
        log.info(cmdline)
        cmdbat.write(cmdline)
        cmdsh.write(cmdline)


# loop over modules

for idet in detsections:

# read module setups

	log.info(idet)
	flename = None
	fhename = None
	lacname = None
	uldname = None

	doptions = cfile.options(idet)
	for opt in doptions:
 
# the following are overrides for automatic filename searching (optional)

		if opt == 'flename':
			flename = cfile.get(idet,opt)
		elif opt == 'fhename':
			fhename = cfile.get(idet,opt)
		elif opt == 'lacname':
			lacname = cfile.get(idet,opt)
		elif opt == 'uldname':
			uldname = cfile.get(idet,opt)

            
# get each file type required for this module from the run folder
# Note the required name patterns.  If the filenames do not conform,
# either change them or use override

	if flename is None:
		flenamelist = glob.glob('*fle2adc*'+idet+'*.xml')
		if len(flenamelist) != 1:
			log.error(' %s entries in flenamelist',len(flenamelist))
			sys.exit(1)
		else:
			flename = flenamelist[0]
	if fhename is None:
		fhenamelist = glob.glob('*fhe2adc*'+idet+'*.xml')
		if len(fhenamelist) != 1:
			log.error(' %s entries in fhenamelist',len(fhenamelist))
			sys.exit(1)
		else:
			fhename = fhenamelist[0]
	if lacname is None:
		lacnamelist = glob.glob('*lac2adc*'+idet+'*.xml')
		if len(lacnamelist) != 1:
			log.error(' %s entries in lacnamelist',len(lacnamelist))
			sys.exit(1)
		else:
			lacname = lacnamelist[0]
	if uldname is None:
		uldnamelist = glob.glob('*uld2adc*'+idet+'*.xml')
		if len(uldnamelist) != 1:
			log.error(' %s entries in uldnamelist',len(uldnamelist))
			sys.exit(1)
		else:
			uldname = uldnamelist[0]


# add adcsmooth commands to output batch file

	if flename != 'skip':
            process_file(flename)
	if fhename != 'skip':
            process_file(fhename)
	if lacname != 'skip':
            process_file(lacname)
	if uldname != 'skip':
            process_file(uldname)
            
# close output files 

cmdbat.write('endlocal')

cmdbat.close()
cmdsh.close()


# fixup permissions for UNIX sh script

fs = os.stat('adcsmooth.sh')
os.chmod('adcsmooth.sh', (fs.st_mode | stat.S_IXUSR))


# terminate

sys.exit(0)

