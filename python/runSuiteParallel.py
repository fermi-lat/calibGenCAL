"""
This script will run calibGenCAL C++ analysis on REM  parallelized calibration data w/ proper calibGain settings.

runSuiteParallel <cfg_file>

where:
    <cfg_file>      = .ini style configuration file.  see calibGenCAL/python/cfg/runSuiteParallel.cfg for example/documentation

"""

import sys, os
import ConfigParser
import logging

usage = 'runSuiteParallel <cfg_file>'
# a timestamp no one would take seriously
dflt_timestamp = '2000-1-1-00-00:00'

#########################################
########### SUBROUTINES #################
#########################################

def getcfg_def(configFile, section, name, default):
    """ Get value from ConfigParser, subsitute default if parm does not exist """
    if not configFile.has_option(section,name):
        return default
    return configFile.get(section,name)

def getcfg_nodef(configFile, section, name):
    """ Get value from ConfigParser, quit program if it is not defined """
    if not configFile.has_option(section,name):
        log.error('Config file missing [%s]:%s option' % (configName, section, parm))
        print configFile
        sys.exit(1)
    return configFile.get(section,name)

def gen_inl_output_path(infile, ouput_dir, tower,ext):
    """ Generate intNonlin output filename from input root file & tower bay num.  Try to emulate what runCIFit.exe would do."""
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    inl_txtname = 'ci_intnonlin.%s.T%02d.%s' % (basename, int(tower),ext)
    return os.path.join(output_dir,inl_txtname)

def gen_mc_output_path(infile, ouput_dir, tower, calib_type, ext):
    """ Generate muonCalib output filename from input root file & tower bay num.  Try to emulate what runMuonCalib.exe would do."""
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    inl_txtname = '%s.%s.T%02d.%s' % (calib_type,basename, int(tower),ext)
    return os.path.join(output_dir,inl_txtname)

########################################
########### MAIN #######################
########################################

# setup logger
logging.basicConfig()
log = logging.getLogger('runSuiteParallel')
log.setLevel(logging.DEBUG)

# get environment settings
try:
    calibUtilRoot = os.environ["CALIBUTILROOT"]
except:
    log.error('CALIBUTILROOT must be defined')
    sys.exit(1)

# check command line
if len(sys.argv) != 2:
    log.error(usage)
    sys.exit(1)
configName = sys.argv[1]

# read config file settings
log.info('Reading file %s', configName)
configFile = ConfigParser.SafeConfigParser()
configFile.read(configName)
sections = configFile.sections()
if len(sections) == 0:
    log.error('Config file %s missing or empty' % configName)
    sys.exit(1)
            
# retrieve shared cfg
section = 'shared'
# - required parameters
tower_bay_list = getcfg_nodef(configFile,section,'tower_bay_list')
# - optional parameters
instrument = getcfg_def(configFile, section, 'instrument','LAT')
output_dir = getcfg_def(configFile, section, 'output_dir','./')

# - populate environment variables for template config files
os.environ['CGC_INSTRUMENT'] = instrument
os.environ['CGC_OUTPUT_DIR'] = output_dir


tower_bays = tower_bay_list.split()


######################################
####### PHASE 1: runCIFit ############
######################################

# retrieve ciFit cfg
section = 'runCIFit'
# - required parameters
calibGen_timestamp = getcfg_def(configFile,section,'calibGen_timestamp',dflt_timestamp)
input_rootfile_le  = getcfg_nodef(configFile,section,'input_rootfile_le')
input_rootfile_he  = getcfg_nodef(configFile,section,'input_rootfile_he')

# - setup environment for template config files
os.environ['CGC_CALIBGEN_TIMESTAMP'] = calibGen_timestamp
os.environ['CGC_INTLIN_ROOT_LE']     = input_rootfile_le
os.environ['CGC_INTLIN_ROOT_HE']     = input_rootfile_he


for tower in tower_bays:
    # - set current tower environment variable
    os.environ['CGC_TOWER'] = tower
    # - specify output txt filename b/c it will be needed in later stages
    inl_txtname = gen_inl_output_path(input_rootfile_le, output_dir, tower,'txt')
    os.environ['CGC_INL_TXT'] = inl_txtname
    # - specify output xml filename b/c it will be needed in later stages
    inl_xmlname = gen_inl_output_path(input_rootfile_le, output_dir, tower,'xml')
    os.environ['CGC_INL_XML']
    os.system('runCIFit.exe %CALIBGENCALROOT%\python\cfg\ciFit_option_template.xml')


######################################
####### PHASE 2: muonCalib ###########
######################################
section = 'runMuonCalib'
muon_timestamp      = getcfg_def(configFile,section,'muon_timestamp',dflt_timestamp)
input_rootfile_list = getcfg_nodef(configFile,section,'input_rootfile_list')
input_rootfiles     = input_rootfile_list.split()
first_rootfile        = input_rootfiles[0]

os.environ['CGC_CALIBGEN_TIMESTAMP'] = calibGen_timestamp
os.environ['CGC_MUON_ROOT_FILELIST'] = input_rootfile_list

# - emulate 
for tower in tower_bays:
    # - set current tower environment variable
    os.environ['CGC_TOWER'] = tower
    # - use same output txt intlin filename from intNonlin phase
    inl_txtname = gen_inl_output_path(input_rootfile_le, output_dir, tower,'txt')
    os.environ['CGC_INL_TXT'] = inl_txtname
    # - specify output xml files b/c they will be used in later stages
    ped_xmlname = gen_mc_output_path(first_rootfile, output_dir, tower, 'mc_ped', 'xml')
    os.environ['CGC_PED_XML'] = ped_xmlname
    asym_xmlname = gen_mc_output_path(first_rootfile, output_dir, tower, 'mc_asym', 'xml')
    os.environ['CGC_ASYM_XML'] = asym_xmlname
    mpd_xmlname = gen_mc_output_path(first_rootfile, output_dir, tower, 'mc_mpd', 'xml')
    os.environ['CGC_MPD_XML'] = mpd_xmlname
    os.system('runMuonCalib.exe %CALIBGENCALROOT%\python\cfg\muonCalib_option_template.xml')

######################################
####### PHASE 3: muTrig ##############
######################################
section = 'runMuTrigEff'
muTrig_timestamp      = getcfg_nodef(configFile,section,  'muTrig_timestamp')
input_rootfile_muon_A = getcfg_nodef(configFile,section,  'input_rootfile_muon_A')
input_rootfile_muon_B = getcfg_nodef(configFile,section,  'input_rootfile_muon_B')
input_rootfile_ci     = getcfg_nodef(configFile,section,  'input_rootfile_ci')

for tower in tower_bays:
    # - set current tower environment variable
    os.environ['CGC_TOWER'] = tower
    #os.system('runMuTrigEff.exe %CALIBGENCALROOT%\python\cfg\muTrigEff_option_template.xml')

######################################
###### PHASE 4: merge ################
######################################

# - pedestals
# - intNonlin
# - asymmetry
# - MeVPerDAC

# - generate cfg file
# - run merge scripts
#os.system('python $CALIBGENCALROOT/python/pedMerge.py %s',tempCfgFilename);
#os.system('python $CALIBGENCALROOT/python/intNonlinMerge.py %s',tempCfgFilename);
#os.system('python $CALIBGENCALROOT/python/asymMerge.py %s',tempCfgFilename);
#os.system('python $CALIBGENCALROOT/python/mevPerDacMerge.py %s',tempCfgFilename);


######################################
###### PHASE 5: validate #############
######################################

# - pedestals
# - intNonlin
# - asymmetry
# - MeVPerDAC

# - generate cfg file
# - run val scripts
#os.system('python $CALIBGENCALROOT/python/pedVal.py %s',tempCfgFilename);
#os.system('python $CALIBGENCALROOT/python/intNonlinVal.py %s',tempCfgFilename);
#os.system('python $CALIBGENCALROOT/python/asymVal.py %s',tempCfgFilename);
#os.system('python $CALIBGENCALROOT/python/mevPerDacVal.py %s',tempCfgFilename);


