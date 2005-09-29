import sys, os
import logging
import string
import ConfigParser
import glob
import os.path
from optparse import OptionParser

# finds files for gensettings and creates bat file with adcsmooth commands to produce filtered files
# NOTE:  folder must contain only one file for ea module and type
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



# open batch file to receive adcsmooth commands

cmdbat = open('adcsmooth.bat','w')

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
		flenamelist = glob.glob('*'+idet+'*fle2adc*.xml')
		if len(flenamelist) != 1:
			log.error(' %s entries in flenamelist',len(flenamelist))
			sys.exit(1)
		else:
			flename = flenamelist[0]
	if fhename is None:
		fhenamelist = glob.glob('*'+idet+'*fhe2adc*.xml')
		if len(fhenamelist) != 1:
			log.error(' %s entries in flenamelist',len(fhenamelist))
			sys.exit(1)
		else:
			fhename = fhenamelist[0]
	if lacname is None:
		lacnamelist = glob.glob('*'+idet+'*lac2adc*.xml')
		if len(lacnamelist) != 1:
			log.error(' %s entries in flenamelist',len(lacnamelist))
			sys.exit(1)
		else:
			lacname = lacnamelist[0]
	if uldname is None:
		uldnamelist = glob.glob('*'+idet+'*uld2adc*.xml')
		if len(uldnamelist) != 1:
			log.error(' %s entries in flenamelist',len(uldnamelist))
			sys.exit(1)
		else:
			uldname = uldnamelist[0]


# add adcsmooth commands to output batch file

	if flename != 'skip':
# Write out run command to batch file
		filesplit = os.path.splitext(flename)
		cmdline = r"python %CALIBGENCALROOT%/python/adcsmooth.py -V "+\
			flename+" "+filesplit[0]+"_filtered.xml\n"
		log.info(cmdline)
		cmdbat.write(cmdline)
	if fhename != 'skip':
# Write out run command to batch file
		filesplit = os.path.splitext(fhename)
		cmdline = r"python %CALIBGENCALROOT%/python/adcsmooth.py -V "+\
			fhename+" "+filesplit[0]+"_filtered.xml\n"
		cmdbat.write(cmdline)
	if lacname != 'skip':
# Write out run command to batch file
		filesplit = os.path.splitext(lacname)
		cmdline = r"python %CALIBGENCALROOT%/python/adcsmooth.py -V "+\
			lacname+" "+filesplit[0]+"_filtered.xml\n"
		cmdbat.write(cmdline)
	if uldname != 'skip':
# Write out run command to batch file
		filesplit = os.path.splitext(uldname)
		cmdline = r"python %CALIBGENCALROOT%/python/adcsmooth.py -V "+\
			uldname+" "+filesplit[0]+"_filtered.xml\n"
		cmdbat.write(cmdline)

# close output file and terminate
cmdbat.close()

sys.exit(0)
