"""
This script will run calibGenCAL C++ analysis on parallel calibration data.

"""

__facility__  = "Offline"
__abstract__  = "Process full set of GLAST CAL ground calibration event files."
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/04/13 21:45:28 $"
__version__   = "$Revision: 1.13 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import ConfigParser
import logging
from optparse import OptionParser
import re

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

def gen_inl_output_path(infile, output_dir, tower, ext):
    """ Generate intNonlin output filename from input root file & tower bay num.  Try to emulate what runCIFit.exe would do."""
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    newname = 'ci_intnonlin.%s.T%02d.%s' % (basename, int(tower),ext)
    return os.path.join(output_dir,newname)

def gen_inl_final_path(infile, output_dir, ext):
    """ Generate intNonlin output filename w/ out tower info """
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    newname = 'ci_intnonlin.%s.%s' % (basename, ext)
    return os.path.join(output_dir,newname)
    

def gen_mc_output_path(infile, output_dir, tower, calib_type, ext):
    """ Generate muonCalib output filename from input root file & tower bay num.  Try to emulate what runMuonCalib.exe would do."""
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    newname = '%s.%s.T%02d.%s' % (calib_type,basename, int(tower),ext)
    return os.path.join(output_dir,newname)

def gen_mc_final_path(infile, output_dir, calib_type, ext):
    """ Generate muonCalib output filename w/ out tower info """
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    newname = '%s.%s.%s' % (calib_type,basename,ext)
    return os.path.join(output_dir,newname)

def gen_adc2nrg_path(infile, output_dir, tower, ext):
    """ Generate adc2nrg output filename, Try to emulate what runMuonCalib.exe would do."""
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    nameparts = basename.split('_')

    # find module name
    modnames = filter(re.compile('^FM\d\d\d').match, nameparts)
    if (len(modnames)):
        modname = modnames[0]
    else:
        modname = "FMxxx"

    # find SLAC runId (or NRL timestamp)
    runidnames = filter(re.compile('^\d{9,12}$').match, nameparts)
    if (len(runidnames)):
        runidname = runidnames[0]
    else:
        runidname = "XXXXXXXXXX"

    newname = '%s_T%02d_%s_cal_adc2nrg.%s' % (runidname, int(tower), modname,ext)
    return os.path.join(output_dir, newname)

def gen_bias_path(infile, output_dir, tower, ext):
    """ Generate bias output filename. Try to emulate whate runMuonCalib.exe would do."""
    
    basename = os.path.basename(infile)
    (basename,tmp) = os.path.splitext(basename)
    nameparts = basename.split('_')

    # find module name
    modnames = filter(re.compile('^FM\d\d\d').match, nameparts)
    if (len(modnames)):
        modname = modnames[0]
    else:
        modname = "FMxxx"

    # find SLAC runId (or NRL timestamp)
    runidnames = filter(re.compile('^\d{9,12}$').match, nameparts)
    if (len(runidnames)):
        runidname = runidnames[0]
    else:
        runidname = "XXXXXXXXXX"

    newname = '%s_T%02d_%s_cal_bias.%s' % (runidname, int(tower), modname, ext)
    return os.path.join(output_dir, newname)


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


# parse command line
usage = "usage: %prog [options] [cfgfile]"
parser = OptionParser(usage)
parser.add_option("-b", "--batch-mode", action="store_true", dest="batch_mode",
                  help="batch mode. do not run other apps, just generate a batch script")
parser.add_option("--batch-file", dest="batch_file", type="string",
                  help="specify batch file name", default="runSuiteParallel.bat")
parser.add_option("-c", "--no-cifit", action="store_true", dest="no_cifit",
                  help="disable cifit.  override cfg file")
parser.add_option("-m", "--no-merge", action="store_true", dest="no_merge",
                  help="disable merge. override cfg file")
parser.add_option("-o", "--no-muopt", action="store_true", dest="no_muopt",
                  help="disable muopt. override cfg file")
parser.add_option("-p", "--no-muped", action="store_true", dest="no_muped",
                  help="disable muped. override cfg file")
parser.add_option("-r", "--no-mutrig", action="store_true", dest="no_mutrig",
                  help="disable mutrig. override cfg file")
parser.add_option("-t", "--tower", type="int", dest="single_tower", metavar="TOWER_ID",
                  help="process single tower..  override cfg file")
parser.add_option("-v", "--no-validate", action="store_true", dest="no_validate",
                  help="disable validation.  override cfg file")

parser.set_defaults(batch_mode=False,
                    batch_file="runSuiteParallel.bat",
                    no_cifit=False,
                    no_merge=False,
                    no_muopt=False,
                    no_muped=False,
                    no_mutrig=False,
                    no_validate=False)

(options, args) = parser.parse_args()
if (options.batch_mode):
    batch_file = open(options.batch_file, 'w')

def run_cmd(cmdline):
    """ either run cmdline in shell or print it to batch file, depending on cfg """
    if (options.batch_mode):
        print>>batch_file, cmdline
    else:
        os.system(cmdline)


if (len(args) > 1):
    parser.error("incorrect number of arguments")

if (len(args) == 0):
    configName = "runSuiteParallel.cfg"
else:
    configName = args[0]

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

# - overwrite shared cfg w/ cmdline options
if options.single_tower != None:
    tower_bays = [str(options.single_tower)]


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

# - overwrite cfg file w/ cmd line options
if options.no_cifit:
    cifit_enabled = False

# - setup environment for template config files
os.environ['CGC_CALIBGEN_TIMESTAMP'] = calibGen_timestamp
os.environ['CGC_INTLIN_ROOT_LE']     = ci_rootfile_le
os.environ['CGC_INTLIN_ROOT_HE']     = ci_rootfile_he
os.environ['CGC_CALIBGEN_BCAST']     = "0"

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
        run_cmd('runCIFit.exe %s/python/cfg/ciFit_option_template.xml'%calibGenCALRoot)


##################################################
####### PHASE 2: muonCalib (pedestals) ###########
##################################################

# retrieve muped cfg
section = 'muon_peds'
muped_timestamp     = getcfg_def(configFile,section,'muped_timestamp',dflt_timestamp)
muped_rootfile_list = getcfg_nodef(configFile,section,'muped_rootfiles')
muped_rootfiles     = muped_rootfile_list.split()
first_mupedfile     = muped_rootfiles[0]
muped_enabled       = getcfg_def(configFile,section,'enable_section',1)

# overwrite cfg file w/ cmdline options
if options.no_muped:
    muped_enabled= False


# - need to fill in these settings to keep template file happy even
#   tho they are used only in the optical section
n_evt_asym          = getcfg_nodef(configFile,'muon_optical','n_evt_asym')
n_evt_mpd           = getcfg_nodef(configFile,'muon_optical','n_evt_mpd')

os.environ['CGC_MUON_TIMESTAMP'] = calibGen_timestamp
os.environ['CGC_MUON_ROOT_FILELIST'] = muped_rootfile_list
os.environ['CGC_READ_IN_PEDS']       = "0"
os.environ['CGC_PEDS_ONLY']          = "1"
os.environ['CGC_NEVT_ROUGHPED']      = "10000"
os.environ['CGC_NEVT_PED']           = "10000"
os.environ['CGC_NEVT_ASYM']          = n_evt_asym
os.environ['CGC_NEVT_MPD']           = n_evt_mpd
os.environ['CGC_ADC2NRG_XML']        = ""



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
        run_cmd('runMuonCalib.exe %s/python/cfg/muonCalib_option_template.xml'%calibGenCALRoot)


##################################################
####### PHASE 3: muonCalib (optical) #############
##################################################


# retrieve muopt_cfg 
section = 'muon_optical'
muopt_timestamp     = getcfg_def(configFile,section,'muopt_timestamp',dflt_timestamp)
muopt_rootfile_list = getcfg_nodef(configFile,section,'muopt_rootfiles')
muopt_rootfiles     = muopt_rootfile_list.split()
first_muoptfile     = muopt_rootfiles[0]
muopt_enabled       = getcfg_def(configFile,section,'enable_section',1)


# overwrite cfg w/ cmdline
if options.no_muopt:
    muopt_enabled= False

os.environ['CGC_MUON_TIMESTAMP']     = calibGen_timestamp
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

        # - specify output adc2nrg filename, it is needed for validation later
        adc2nrg_xmlname = gen_adc2nrg_path(first_muoptfile, output_dir, tower, '.xml')
        os.environ['CGC_ADC2NRG_XML'] = adc2nrg_xmlname
        
        # - run muonCalib
        run_cmd('runMuonCalib.exe %s/python/cfg/muonCalib_option_template.xml'%calibGenCALRoot)

        # - run adc2nrgVal
        adc2nrg_rootname = gen_adc2nrg_path(first_muoptfile, output_dir, tower, 'root')
        adc2nrg_logname  = gen_adc2nrg_path(first_muoptfile, output_dir, tower, 'val.log')
        run_cmd('python %s/python/adc2nrgVal.py -R %s -L %s %s' %(calibGenCALRoot, adc2nrg_rootname, \
                                                                  adc2nrg_logname, adc2nrg_xmlname))


######################################
####### PHASE 4: muTrig ##############
######################################
# get cfg
section = 'runMuTrigEff'
muTrig_timestamp   = getcfg_nodef(configFile,section,  'muTrig_timestamp')
mt_rootfile_muon_A = getcfg_nodef(configFile,section,  'mt_rootfile_muon_A')
mt_rootfile_muon_B = getcfg_nodef(configFile,section,  'mt_rootfile_muon_B')
mt_rootfile_ci     = getcfg_nodef(configFile,section,  'mt_rootfile_ci')
mutrig_enabled     = getcfg_def(configFile,section, 'enable_section',1)

# overwrite cfg file w/ cmdline
if options.no_mutrig:
    mutrig_enabled= False


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

        bias_xmlname = gen_bias_path(first_mupedfile, output_dir, tower, 'xml')
        os.environ['CGC_BIAS_XML'] = bias_xmlname

        # run muTrigEff
        run_cmd('runMuTrigEff.exe %s/python/cfg/muTrigEff_option_template.xml'%calibGenCALRoot)

        # run biasVal
        bias_rootname = gen_bias_path(first_mupedfile, output_dir, tower, 'root')
        bias_logname  = gen_bias_path(first_mupedfile, output_dir, tower, 'val.log')
        run_cmd('python %s/python/biasVal.py -R %s -L %s %s'% \
                (calibGenCALRoot,bias_rootname, bias_logname, bias_xmlname))

######################################
###### PHASE 5: merge ################
######################################


# - get cfg
section = 'merge'
merge_enabled = getcfg_def(configFile, section, 'enable_section', 1)

# overwrite cfg w/ cmdline
if options.no_merge:
    merge_enabled = False

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
    mergeConfig.set('dtdfiles','dtdfile','calCalib_v2r3.dtd')
    
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
    run_cmd('python %s/python/pedMerge.py %s %s'%(calibGenCALRoot, mergeCfgFilename, pedFinalXML))
    run_cmd('python %s/python/intNonlinMerge.py %s %s'%(calibGenCALRoot, mergeCfgFilename, inlFinalXML))
    run_cmd('python %s/python/asymMerge.py %s %s'%(calibGenCALRoot, mergeCfgFilename, asymFinalXML));
    run_cmd('python %s/python/mevPerDacMerge.py %s %s'%(calibGenCALRoot, mergeCfgFilename, mpdFinalXML));

######################################
###### PHASE 7: validate #############
######################################
# - get cfg
section = 'validate'
validate_enabled = getcfg_def(configFile, section, 'enable_section', 1)

# - overwrite cfg w/ cmdline
if options.no_validate:
    validate_enabled = False

# - generate validation logfiles
inlValLog  = gen_inl_final_path(ci_rootfile_le, output_dir, 'val.log')
pedValLog  = gen_mc_final_path(first_mupedfile, output_dir, 'mc_peds', 'val.log')
asymValLog = gen_mc_final_path(first_muoptfile, output_dir, 'mc_asym', 'val.log')
mpdValLog  = gen_mc_final_path(first_muoptfile, output_dir, 'mc_mpd', 'val.log')


if int(validate_enabled):
    # - run val scripts
    run_cmd('python %s/python/pedVal.py -L%s %s'%(calibGenCALRoot, pedValLog, pedFinalXML));
    run_cmd('python %s/python/intNonlinVal.py -L%s %s'%(calibGenCALRoot, inlValLog, inlFinalXML));
    run_cmd('python %s/python/asymVal.py -L%s %s'%(calibGenCALRoot, asymValLog, asymFinalXML));
    run_cmd('python %s/python/mevPerDacVal.py -L%s %s'%(calibGenCALRoot, mpdValLog, mpdFinalXML));


