// (Special "header" just for doxygen)

/*! @mainpage  package calibGenCAL

This package contains codes to generate and study calibration
constants for CAL using muon and charge injection data. The package
generates the following calibrations for use in offline software.


- <b> Pedestal Noise </b> 
      ADC pedestal position and width are measured.

- <b> Light Asymmetry </b> 
      Light asymmetry vs position represented by 
      10 points along each crystal length.

- <b> Integral Nonlinearity </b> 
      adc2dac conversion in all 4 ranges.
      Corrects for nonlinearity in adc scale.

- \b MevPerDAC  
  Relationship of total energy deposited to the
  geometric mean of the DAC values at both ends of the xtal.  Onboard
  DAC scale units are used for calculation b/c it is much more linear
  than ADC scale.

- \b Thresholds  
  Measurement of FLE & FHE triggers, ULD & LAC thresholds in ADC units.

 
\b Applications:
-  \b runMuTrigEff.exe

-  \b runCIFit.exe  
      Generates spline function points for adc2dac
      conversions. Reads digi root files containing charge injection
      calibration data. Expected test configuration is described in code.
      Reads config in from ciFit_option.xml, use
      ciFit_option_badCalibGen.xml if charge injection test used wrong
      calib_gain setting.

-  \b runMuonCalib.exe  
   Reads data from one or more muon collection
   digi root files and produces calibration constant files in XML
   format (pedestals,asymetry and MevPerDAC). It also produces root
   histogram files for studying the quality of the calibration
   constants. The executable takes input from muonCalib_option.xml,
   use muonCalib_option_badCalibGen.xml for CI tests w/ wrong
   calib_gain setting.


<b> Shared Classes & Headers: </b>
- \b CalDefs.h 
     Collection of classes used for contiguous indexing of
     different cal components. Supports numerous conversions, incremen, and
     field get/set() routines.  Also contains useful Cal constant
     definitions.

- \b RootFileAnalysis 
  This class provides generic root file playback
  for Glast ROOT format event data.  This functionality was extracted
  from RootAnalysis/RootFileAnalysis.cxx. It allows the user to treat
  an arbitrary collection of digi/mc/recon GLAST root data as a
  collection of events which may be stepped through/'seeked'/rewound,
  etc...  Intended to be inherited by application specific class that
  provides the data structures, event loop & output code.

- \b CGCUtil.h 
     contains generic, non-GLAST related functions which are shared 
     throughout the calibGenCAL package. (string, stream & vector manipulation, 
     etc...).


<b> Python Script Utilities: </b>

The calibGenCAL python utilites have been tested using Python 2.3 and
Python 2.4.

Their use also requires that the following python extensions be installed:
- \b Numeric
- \b PyXML

\b usage 
A set of shell scripts in the %CALIBGENCALROOT%/python directory may be used to
launch the tools in either Windows (.bat) or Linux (.sh)

<b> cfg files </b>
Sample configuration scripts for these tools are included in the 
python/cfg folder.  

<b> merge scripts </b>

- <tt> intNonlinMerge [-V] <cfg_file> <out_xml_file> </tt>
- <tt> pedMerge       [-V] <cfg_file> <out_xml_file> </tt>
- <tt> mevPerDacMerge [-V] <cfg_file> <out_xml_file> </tt>
- <tt> asymMerge      [-V] <cfg_file> <out_xml_file> </tt>

The merge tools take multiple single-tower CAL calibration XML files
and produce a single output file of the same type, with the option to
specify the source and destination tower addressing.  All of the
python merge tools take a configuration file as input.  This
configuration file specifies the input data sets and tower addressing. 
The -V option increases the verbosity of the diagnostic output.

<b> dac settings scripts </b>

- <tt> genLACsettings [-V] <MeV> <cfg_file> <out_xml_file> </tt>
- <tt> genFLEsettings [-V] <MeV> <cfg_file> <out_xml_file> </tt>
- <tt> genFHEsettings [-V] <GeV> <cfg_file> <out_xml_file> </tt>
- <tt> genULDsettings [-V] <cfg_file> <out_xml_file> </tt>

The DAC settings generation tools produce configuration XML files
providing values to configure each channel.  The DAC settings tools
take a configuration file and possibly (except for ULD) a threshold
energy as input.  TThe -V option increases the verbosity of the
diagnostic output.


- <tt> genGainSettings [-V] <leGain> <heGain> <out_xml_file> </tt>

The genGainSettings tool is useful for producing a base CAL snapshot
fragment which only contains the gain settings in <config_0> elements.


- <tt> tholdCIGen [-V] <cfg_file> <out_xml_file> </tt>

The tholdCIGen application produces a CAL threshold calibration XML file using
information from numerous input files.  A snapshot file provides the
configuration of the CAL for each calibration desired.  A set of characterization
files produced by the CI tests provide the ADC threshold data lookup for each configuration.

<b> validation scripts </b>

- <tt> asymVal      [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> adc2nrgVal   [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> biasVal      [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> intNonlinVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> mevPerDacVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> pedVal       [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> tholdCIVal   [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> charVal      [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file> </tt>

The validation scripts perform simple checks on the values and formats
of the various CAL calibration XML file types.  The checks are usually nothing more than limit and
consitency checks.  Warnings or errors generated may be benign.  The '-R' produces plots and
histograms in ROOT format to help diagnose any issues with failures.
This requires a working implementation of PyROOT.


- <tt> adcplot [-V] <xml_file> <root_file> </tt>

The adcplot utility will generate ROOT plots of the CAL DAC/ADC
characterization tables. This requires a working implementation of PyROOT. 


- <tt> adcsmooth [-V] <in_file> <out_file> </tt>

The adcsmooth tool performs various fixups of the DAC/ADC
characterization data XML files:

 - Remove sparse and dropout data points
 - Remove outlying data points
 - Perform linear extrapolation for FLE LEX8 characterization data; needed 
   for higher FLE thresholds
 - Digital smoothing filter as final processing
This tool can be used as a pre-processor for other tools which require
clean characterization tables to work well (e.g. genXXXsettings).

<b> conversion scripts </b>

- <tt> asymTXT2XML [-doptional.dtd] <input.txt> <output.xml> </tt>
- <tt> pedTXT2XML [-doptional.dtd] <input.txt> <output.xml> </tt>
- <tt> mpdTXT2XML [-doptional.dtd] <input.txt> <output.xml> </tt>

Each of the TXT2XML scripts converts one offline calibration file type
from space delimited TXT file to proper XML file format.


<b> gensettings toplevel scripts </b>

- <tt> get_slac_calibdac </tt>
- <tt> build_adcsmooth [-f fileroot][--file=fileroot] </tt>
- <tt> build_gensettings_cfg [-f fileroot][--file=fileroot] </tt>
- <tt> gensettings [-f fileroot][--file=fileroot] </tt>

The four scripts get_slac_calibdac.py, build_acdsmooth.py, build_gensettings_cfg.py 
and gensettings.py automate the process of generating DAC settings using the genXXXsettings.py
(genFLEsettings.py, genFHEsiettings.py, genLACsettings.py and genULDsettings.py)
scripts. They are meant to operate in the LAT I&T environment, in that they assume 
run numbers and file names as generated by the I&T version of the calibDAC suite and 
subsequent pipeline analyses.

see calibGenCAL/doc/gensettings_scripts.html for more information.

*/
