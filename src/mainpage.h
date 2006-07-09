// (Special "header" just for doxygen)
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/mainpage.h,v 1.36 2006/07/05 15:59:50 dwood Exp $


/*! 
  @mainpage  package calibGenCAL
  @author Zach Fewtrell

  <h3> Introduction </h3>

This package contains codes to generate and study calibration
constants for CAL using muon and charge injection data. The package
generates the following calibrations for use in offline software:


- <b> Pedestal Noise </b> 
      ADC pedestal position and width are measured.

- <b> Light Asymmetry </b> 
      Light asymmetry vs position represented by 
      10 points along each crystal length.

- <b> Integral Nonlinearity </b> 
      adc2cidac conversion in all 4 ranges.
      Corrects for nonlinearity in adc scale.

- \b MevPerDAC  
  Ratio of total energy deposited in crystal to the
  geometric mean of the DAC values at both ends of the xtal.  Onboard
  Charge Injection (CIDAC) scale units are used for calculation b/c it is 
  more linear than ADC scale.

- \b Thresholds  
  Measurement of FLE & FHE triggers, ULD & LAC thresholds in ADC units.

 
<h3> C++ Applications:</h3>
-  \b runMuTrigEff.exe
      calculate FLE threshold vs Muon deposit & charge injection.
      calculate bias between the two.

-  \b genCIDAC2ADC.exe
      Generate spline function points for adc2cidac
      conversions. Read digi root files containing charge injection
      calibration data. Expected test configuration is described in code.

- \b genMuonPed.exe
   Generate ADC pedestals from 4-range non-zero suppressed event data.

- \b genMuonAsym.exe
   Generate light-asymmetry calibration from muon event data.

- \b genMuonMPD.exe
   Generate MeVPerDAC calibration from muon event data.


<h3> Python scripts: </h3>

The calibGenCAL python utilites require python version 2.4.

Their use also requires that the following python extensions be installed:
- Numeric
- 4Suite-XML

<h5> Python general usage: launcher scripts  </h5>
For each calibGenCAL python script XYZ.py there exists on XYZ.bat
script for windows shell use and one XYZ.sh launcher script for Linux
shell use. The 'xxx.sh' scripts have UNIX executable permission
bit set and CMT setup of calibGenCAL package will put these launcher
scripts in the system path. The user can launch any calibGenCAL
python script simply by typing the name of the appropriate
launcher script into the shell. All commandline paramters to the
launcher script will be passeed directly onto the python script.

<h5> Python general usage: commandline usage  </h5>
Most calibGenCAL python scripts will display commandline usage instructions to terminal 
if they are invoked w/ no arguments.

<h5> gensettings toplevel scripts </h5>

- <tt> get_slac_calibdac </tt>
- <tt> build_adcsmooth [-f fileroot][--file=fileroot] </tt>
- <tt> build_gensettings_cfg [-f fileroot][--file=fileroot] </tt>
- <tt> gensettings [-f fileroot][--file=fileroot] </tt>

<p>The four scripts get_slac_calibdac.py, build_acdsmooth.py, build_gensettings_cfg.py 
and gensettings.py automate the process of generating DAC settings using the 
genXXXsettings.py (genFLEsettings.py, genFHEsiettings.py, genLACsettings.py and 
genULDsettings.py) scripts. They are meant to operate in the LAT I&T environment, 
in that they assume run numbers and file names as generated by the I&T version 
of the calibDAC suite and subsequent pipeline analyses.</p>

<p>These scripts will generate associated validation output as well, by invoking appropriate dacVal and charVal scripts </p>

<p>see calibGenCAL/doc/gensettings_scripts.html for more information.</p>

- <tt> tholdCIGen [-V] <cfg_file> <out_xml_file> </tt>

<p>The tholdCIGen application produces a CAL threshold calibration XML file using
information from numerous input files.  A snapshot file provides the
configuration of the CAL for each calibration desired.  A set of characterization
files produced by the CI tests provide the ADC threshold data lookup for each configuration.</p>

- build_tholdci_cfg
<p> generates tholdCIGen cfg file from information in gensetttings cfg file </b>

<h5> generic utility scripts </h5>
- roothist2CSV.py   - dump all root histograms in file to collumnar text
- dumpROOTPlots.py  - dump all root histograms & TCanvas objects in file to image file.

<h5> dac settings scripts </h5>

- <tt> genLACsettings [-V] <MeV> <cfg_file> <out_xml_file> </tt>
- <tt> genFLEsettings [-V] <MeV> <cfg_file> <out_xml_file> </tt>
- <tt> genFHEsettings [-V] <GeV> <cfg_file> <out_xml_file> </tt>
- <tt> genULDsettings [-V] <cfg_file> <out_xml_file> </tt>

<p>The DAC settings generation tools produce configuration XML files
providing values to configure each channel.  The DAC settings tools
take a configuration file and possibly (except for ULD) a threshold
energy as input.  TThe -V option increases the verbosity of the
diagnostic output.</p>

- <tt> genGainSettings [-V] <leGain> <heGain> <out_xml_file> </tt>

<p>The genGainSettings tool is useful for producing a base CAL snapshot
fragment which only contains the gain settings in <config_0> elements.</p>

- dacDiff.py
<p>dacDiff.py will calculate per-channel difference between any two cal dac 
settings files.  This can be used for trending & validation.</p>


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

<h5> Validation scripts </h5>

- <tt> asymVal      [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> adc2nrgVal   [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> biasVal      [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> intNonlinVal [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> mevPerDacVal [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> pedVal       [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> tholdCIVal   [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> charVal      [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> muSlopeVal   [-V] [-R <root_file>] [-L <log_file>] <xml_file> </tt>
- <tt> dacVal       [-V] [-R <root_file>] [-L <log_file>] FLE|FHE|LAC <MeV> <cfg_file> <dac_xml_file> </tt>
- <tt> checkXML     [xml_file_0 xml_file_1 ...] </tt> 


The validation scripts perform simple checks on the values and formats
of the various CAL calibration XML file types.  The checks are usually nothing more than limit and
consitency checks.  Warnings or errors generated may be benign.  The '-R' produces plots and
histograms in ROOT format to help diagnose any issues with failures.
This requires a working implementation of PyROOT.

- <tt> adcplot [-V] <xml_file> <root_file> </tt>
- <tt> charplot [-V] <raw_xml_file> <filtered_xml_file> <root_file> </tt>

- The adcplot utility will generate ROOT plots of the CAL DAC/ADC
characterization tables. 
- The charplot utility overlays raw, fitered, and linear model characterization
data plots.  These require a working implementation of PyROOT. 

<h5> Merge scripts </h5>

- <tt> intNonlinMerge [-V] <cfg_file> <out_xml_file> </tt>
- <tt> pedMerge       [-V] <cfg_file> <out_xml_file> </tt>
- <tt> mevPerDacMerge [-V] <cfg_file> <out_xml_file> </tt>
- <tt> asymMerge      [-V] <cfg_file> <out_xml_file> </tt>
- <tt> calFitsMerge [-V] <output_xml_file> <input_xml_file1> [input_xml_fileN]* </tt> merge relgain files

The merge tools take multiple single-tower CAL calibration XML files
and produce a single output file of the same type, with the option to
specify the source and destination tower addressing.  All of the
python merge tools take a configuration file as input.  This
configuration file specifies the input data sets and tower addressing. 
The -V option increases the verbosity of the diagnostic output.

<h5> Conversion scripts </h5>
For converting xml files to and from columnar text and other formats.

- <tt> asymTXT2XML [-doptional.dtd] <input.txt> <output.xml> </tt>
- <tt> pedTXT2XML [-doptional.dtd] <input.txt> <output.xml> </tt>
- <tt> mpdTXT2XML [-doptional.dtd] <input.txt> <output.xml> </tt>
- <tt> tholdciXML2TXT.py </tt>
- <tt> pedXML2TXT.py </tt>
- <tt> asymXML2TXT.py </tt>
- <tt> mpdXML2TXT.py </tt>
- <tt> tholdCIXML2TXT.py </tt>
- <tt> genFlightPed [-o] [-v] [-k <key>] <ped_xml_file> </tt>
- <tt> genFlightGain [-o] [-v] [-k <key>] <gain_xml_file> </tt>
- <tt> adc2nrgTXT2XML </tt>
- <tt> adc2nrgTXT2XML input.txt output.xml </tt>
- <tt> muSlopeTXT2XML [-doptional.dtd] input.txt output.xml </tt>
- <tt> inlTXT2XML [-doptional.dtd] input.txt output.xml </tt>
- <tt> biasTXT2XML </tt>

Each of the TXT2XML scripts converts one offline calibration file type
from space delimited TXT file to proper XML file format.

The genFlightGain script converts a CAL_Ped XML file into a 'cal_pedestals.h' 
file for use in FSW on-board filter code

The genFlightGain script converts a CAL_MuSlope XML file into a 'cal_gains.h' 
file for use in FSW on-board filter code

<h3> cfg files </h3>
Sample configuration scripts for python and C++ tools are included in the cfg 
folder.  

<h3> unit test </h3>

<p>the unit_test subfolder contains cfg files & validated output for most of the
calibGenCAL applications.
<p>unit_test/output contains the validated output:</p>

<h3> other docs </h3>
- \b calibGenCAL/doc/calibGenCAL_configurationRecord_howto.txt - generate online
dac settings with calibGenCAL python scripts.
- \b calibGenCAL/doc/gensettings_scripts.html - in depth description of dac 
settings tools

*/
