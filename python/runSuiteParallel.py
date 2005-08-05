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
        log.error('Config file missing [%s]:%s option' % (section, name))
        print configFile
        sys.exit(1)
    return configFile.get(section,name)

def gen_inl_output_path(infile, output_dir, tower,ext):
    """ Generate intNonlin output filename from input root file & tower bay num.  Try to emulate what runCIFit.exe would do."""
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    inl_txtname = 'ci_intnonlin.%s.T%02d.%s' % (basename, int(tower),ext)
    return os.path.join(output_dir,inl_txtname)

def gen_inl_final_path(infile, output_dir, ext):
    """ Generate intNonlin output filename w/ out tower info """
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    inl_txtname = 'ci_intnonlin.%s.%s' % (basename, ext)
    return os.path.join(output_dir,inl_txtname)
    

def gen_mc_output_path(infile, output_dir, tower, calib_type, ext):
    """ Generate muonCalib output filename from input root file & tower bay num.  Try to emulate what runMuonCalib.exe would do."""
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    inl_txtname = '%s.%s.T%02d.%s' % (calib_type,basename, int(tower),ext)
    return os.path.join(output_dir,inl_txtname)

def gen_mc_final_path(infile, output_dir, calib_type, ext):
    """ Generate muonCalib output filename w/ out tower info """
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    inl_txtname = '%s.%s.%s' % (calib_type,basename,ext)
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

try:
    calibGenCALRoot = os.environ["CALIBGENCALROOT"]
except:
    log.error('CALIBGENCALROOT must be defined')
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
ci_rootfile_le  = getcfg_nodef(configFile,section,'ci_rootfile_le')
ci_rootfile_he  = getcfg_nodef(configFile,section,'ci_rootfile_he')
cifit_enabled   = getcfg_def(configFile,section,'enable_section',1)

# - setup environment for template config files
os.environ['CGC_CALIBGEN_TIMESTAMP'] = calibGen_timestamp
os.environ['CGC_INTLIN_ROOT_LE']     = ci_rootfile_le
os.environ['CGC_INTLIN_ROOT_HE']     = ci_rootfile_he

if int(cifit_enabled):
    for tower in tower_bays:
        # - set current tower environment variable
        os.environ['CGC_TOWER'] = tower
        # - specify output txt filename b/c it will be needed in later stages
        inl_txtname = gen_inl_output_path(ci_rootfile_le, output_dir, tower,'txt')
        os.environ['CGC_INL_TXT'] = inl_txtname
        # - specify output xml filename b/c it will be needed in later stages
        inl_xmlname = gen_inl_output_path(ci_rootfile_le, output_dir, tower,'xml')
        os.environ['CGC_INL_XML'] = inl_xmlname
        
        # - run ciFit
        os.system('runCIFit.exe %s/python/cfg/ciFit_option_template.xml'%calibGenCALRoot)


##################################################
####### PHASE 2: muonCalib (pedestals) ###########
##################################################
section = 'muon_peds'
muped_timestamp     = getcfg_def(configFile,section,'muped_timestamp',dflt_timestamp)
muped_rootfile_list = getcfg_nodef(configFile,section,'muped_rootfiles')
muped_rootfiles     = muped_rootfile_list.split()
first_mupedfile     = muped_rootfiles[0]
muped_enabled       = getcfg_def(configFile,section,'enable_section',1)

os.environ['CGC_MUON_TIMESTAMP'] = calibGen_timestamp
os.environ['CGC_MUON_ROOT_FILELIST'] = muped_rootfile_list
os.environ['CGC_READ_IN_PEDS']       = "0"
os.environ['CGC_PEDS_ONLY']          = "1"

# - emulate
if int(muped_enabled):
    for tower in tower_bays:
        # - set current tower environment variable
        os.environ['CGC_TOWER'] = tower
        
        # - use same output txt intlin filename from intNonlin phase
        inl_txtname = gen_inl_output_path(ci_rootfile_le, output_dir, tower,'txt')
        os.environ['CGC_INL_TXT'] = inl_txtname
        
        # - specify output xml files b/c they will be used in later stages
        ped_xmlname = gen_mc_output_path(first_mupedfile, output_dir, tower, 'mc_peds', 'xml')
        os.environ['CGC_PED_XML']  = ped_xmlname

        ped_txtname = gen_mc_output_path(first_mupedfile, output_dir, tower, 'mc_peds', 'txt')
        os.environ['CGC_PED_TXT']  = ped_txtname

        asym_xmlname = gen_mc_output_path(first_mupedfile, output_dir, tower, 'mc_asym', 'xml')
        os.environ['CGC_ASYM_XML'] = asym_xmlname

        mpd_xmlname = gen_mc_output_path(first_mupedfile, output_dir, tower, 'mc_mpd', 'xml')
        os.environ['CGC_MPD_XML']  = mpd_xmlname
        
        # - run muonCalib
        os.system('runMuonCalib.exe %s/python/cfg/muonCalib_option_template.xml'%calibGenCALRoot)


##################################################
####### PHASE 3: muonCalib (optical) #############
##################################################
section = 'muon_optical'
muopt_timestamp     = getcfg_def(configFile,section,'muopt_timestamp',dflt_timestamp)
muopt_rootfile_list = getcfg_nodef(configFile,section,'muopt_rootfiles')
muopt_rootfiles     = muopt_rootfile_list.split()
first_muoptfile     = muopt_rootfiles[0]
muopt_enabled       = getcfg_def(configFile,section,'enable_section',1)

os.environ['CGC_MUON_TIMESTAMP'] = calibGen_timestamp
os.environ['CGC_MUON_ROOT_FILELIST'] = muopt_rootfile_list
os.environ['CGC_READ_IN_PEDS']       = "1"
os.environ['CGC_PEDS_ONLY']          = "0"

# fill in unused vars w/ blanks -F*#$IING windows does not support empty env vars
os.environ['CGC_PED_XML']="dummy_cgc_ped_xml"

# - emulate
if int(muopt_enabled):
    for tower in tower_bays:
        # - set current tower environment variable
        os.environ['CGC_TOWER'] = tower
        
        # - use same output txt intlin filename from intNonlin phase
        inl_txtname = gen_inl_output_path(ci_rootfile_le, output_dir, tower,'txt')
        os.environ['CGC_INL_TXT'] = inl_txtname
        
        # - specify output xml files b/c they will be used in later stages
        input_ped_txt = gen_mc_output_path(first_mupedfile, output_dir, tower, 'mc_peds', 'txt')
        os.environ['CGC_PED_TXT'] = input_ped_txt
        asym_xmlname = gen_mc_output_path(first_muoptfile, output_dir, tower, 'mc_asym', 'xml')
        os.environ['CGC_ASYM_XML'] = asym_xmlname
        mpd_xmlname = gen_mc_output_path(first_muoptfile, output_dir, tower, 'mc_mpd', 'xml')
        os.environ['CGC_MPD_XML'] = mpd_xmlname
        
        # - run muonCalib
        os.system('runMuonCalib.exe %s/python/cfg/muonCalib_option_template.xml'%calibGenCALRoot)


######################################
####### PHASE 4: muTrig ##############
######################################
section = 'runMuTrigEff'
muTrig_timestamp   = getcfg_nodef(configFile,section,  'muTrig_timestamp')
mt_rootfile_muon_A = getcfg_nodef(configFile,section,  'mt_rootfile_muon_A')
mt_rootfile_muon_B = getcfg_nodef(configFile,section,  'mt_rootfile_muon_B')
mt_rootfile_ci     = getcfg_nodef(configFile,section,  'mt_rootfile_ci')
mutrig_enabled     = getcfg_def(configFile,section, 'enable_section',1)

os.environ['CGC_MUTRIG_TIMESTAMP'] = muTrig_timestamp
os.environ['CGC_MT_MUON_A'] = mt_rootfile_muon_A
os.environ['CGC_MT_MUON_B'] = mt_rootfile_muon_B
os.environ['CGC_MT_CI'] = mt_rootfile_ci


if int(mutrig_enabled):
    for tower in tower_bays:
        # - set current tower environment variable
        os.environ['CGC_TOWER'] = tower

        ped_txtname = gen_mc_output_path(first_mupedfile, output_dir, tower, 'mc_peds', 'txt')
        os.environ['CGC_PED_TXT']  = ped_txtname

        os.system('runMuTrigEff.exe %s/python/cfg/muTrigEff_option_template.xml'%calibGenCALRoot)

######################################
###### PHASE 5: merge ################
######################################


# - get cfg
section = 'merge'
merge_enabled = getcfg_def(configFile, section, 'enable_section', 1)

# - generate final xml filenames
inlFinalXML  = gen_inl_final_path(ci_rootfile_le, output_dir, 'xml')
pedFinalXML  = gen_mc_final_path(first_mupedfile, output_dir, 'mc_peds', 'xml')
asymFinalXML = gen_mc_final_path(first_muoptfile, output_dir, 'mc_asym', 'xml')
mpdFinalXML  = gen_mc_final_path(first_muoptfile, output_dir, 'mc_mpd', 'xml')

if int(merge_enabled):
    # - generate cfg file
    mergeConfig = ConfigParser.SafeConfigParser()
    # - defaults
    mergeConfig.add_section('dtdfiles')
    mergeConfig.set('dtdfiles','dtdfile','calCalib_v2r2.dtd')
    
    mergeConfig.add_section('infiles')
    for tower in tower_bays:
        # - ped file
        ped_xmlname = gen_mc_output_path(first_mupedfile, output_dir, tower, 'mc_peds', 'xml')
        mergeConfig.set('infiles','ped_%d'%int(tower), '%s,%d' % (ped_xmlname,int(tower)))

        # - asym file
        asym_xmlname = gen_mc_output_path(first_muoptfile, output_dir, tower, 'mc_asym', 'xml')
        mergeConfig.set('infiles','asym_%d'%int(tower), '%s,%d' % (asym_xmlname,int(tower)))

        # - mpd file
        mpd_xmlname = gen_mc_output_path(first_muoptfile, output_dir, tower, 'mc_mpd', 'xml')
        mergeConfig.set('infiles','mevPerDac_%d'%int(tower), '%s,%d' % (mpd_xmlname,int(tower)))

        # - inl file
        inl_xmlname = gen_inl_output_path(ci_rootfile_le, output_dir, tower, 'xml')
        mergeConfig.set('infiles','intNonlin_%d'%int(tower), '%s,%d' % (inl_xmlname,int(tower)))

    # - name cfg file after 1st muoptical root file
    basename = os.path.basename(first_muoptfile)
    (basename,tmp) = os.path.splitext(basename)
    mergeCfgFilename = "cgc_merge.%s.cfg"%(basename)
    mergeConfigFile = open(mergeCfgFilename, 'w')
    mergeConfig.write(mergeConfigFile)
    mergeConfigFile.close()

    
        
    # - run merge scripts
    
    os.system('python %s/python/pedMerge.py %s %s'%(calibGenCALROOT, mergeCfgFilename, pedFinalXML));
    os.system('python %s/python/intNonlinMerge.py %s %s'%(calibGenCALROOT, mergeCfgFilename, inlFinalXML));
    os.system('python %s/python/asymMerge.py %s %s'%(calibGenCALROOT, mergeCfgFilename, asymFinalXML));
    os.system('python %s/python/mevPerDacMerge.py %s %s'%(calibGenCALROOT, mergeCfgFilename, mpdFinalXML));


######################################
###### PHASE 7: validate #############
######################################
# - get cfg
section = 'validate'
validate_enabled = getcfg_def(configFile, section, 'enable_section', 1)

# - generate validation logfiles
inlValLog  = gen_inl_final_path(ci_rootfile_le, output_dir, 'val.log')
pedValLog  = gen_mc_final_path(first_mupedfile, output_dir, 'mc_peds', 'val.log')
asymValLog = gen_mc_final_path(first_muoptfile, output_dir, 'mc_asym', 'val.log')
mpdValLog  = gen_mc_final_path(first_muoptfile, output_dir, 'mc_mpd', 'val.log')


if int(validate_enabled):
    # - run val scripts
    os.system('python %s/python/pedVal.py -L%s %s'%(calibGenCALROOT, pedValLog, pedFinalXML));
    os.system('python %s/python/intNonlinVal.py -L%s %s'%(calibGenCALROOT, inlValLog, inlFinalXML));
    os.system('python %s/python/asymVal.py -L%s %s'%(calibGenCALROOT, asymValLog, asymFinalXML));
    os.system('python %s/python/mevPerDacVal.py -L%s %s'%(calibGenCALROOT, mpdValLog, mpdFinalXML));


