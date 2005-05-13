// (Special "header" just for doxygen)

/*! @mainpage  package calibGenCAL

This package contains codes to generate and study calibration constants for CAL 
using muon and charge injection data. The package generates the following
calibrations.

- Pedestals – Noise pedestal position and width are measured.
- Light Asymmetry – Light asymmetry vs position represented by a table 
of 10 spline points along each crystal length.
- Integral Nonlinearity - adc2dac conversion in all 4 ranges.  
Corrects for nonlinearity in adc scale.
- MevPerDAC - Relastionship of total energy deposited to the geometric mean of 
the DAC values at both ends of the xtal.  Onboard DAC scale units are used for 
calculation b/c it is much more linear than ADC scale.
 
\b Applications:
-  \b runMuTrigEff.exe
-  \b runCIFit.exe  Generates spline function points for adc2dac conversions.
Reads digi root files containing charge injection calibration data. 
Expected test configuration is described in code.   Reads config in from 
ciFit_option.xml
-  \b runMuonCalib.exe Reads data from one or more muon collection digi root 
files and produces calibration constant files in both ascii and XML format 
(pedestals,asymetry and MevPerDAC). It also produces root histogram files 
for studying the quality of the calibration constants. The executable takes 
input from muonCalib_option.xml.

\b Shared Classes & Headers:
- \b CalDefs.h A collection of classes used for providing contiguous indexes for
the different cal components. Supports numerous conversions, incrementing, 
sub-field get() & set() routines.  Also contains useful Cal constant definitions.
- \b RootFileAnalysis This class provides generic root file input for Glast root
data.  This functionality was extracted from RootAnalysis/RootFileAnalysis.exe. 
It allows the user to treat an arbitrary collection of digi/mc/recon GLAST root 
data as a collection of events which may be stepped through/'seeked'/rewound, 
etc...  This class is intended to be inherited by application 
specific class that provide the data structures/event loop/output code.
- \b CGCUtil.h - contains generic, non-glast related utility functions which are
shared throughout the calibGenCAL package. (string, stream & vector manipulation
for example).

\b Python Script Utilities:

The calibGenCAL python utilites have been tested using Python 2.3 and Python 2.4.
Their use also requires that the following python extensions be installed:
- \b Numeric
- \b PyXML

A set of shell scripts in the %CALIBGENCALROOT%/python directory may be used to
launch the tools.

- \b intNonlinMerge [-V] <cfg_file> <out_xml_file>
- \b pedMerge [-V] <cfg_file> <out_xml_file>
- \b mevPerDacMerge [-V] <cfg_file> <out_xml_file>
- \b asymMerge [-V] <cfg_file> <out_xml_file>

- \b genLACsettings [-V] <MeV> <cfg_file> <out_xml_file>
- \b genFLEsettings [-V] <MeV> <cfg_file> <out_xml_file>
- \b genFHEsettings [-V] <GeV> <cfg_file> <out_xml_file>
- \b genULDsettings [-V] [-M <margin>] <uld2adc_file> <out_xml_file>

The merge tools take multiple single-tower CAL calibration XML files and produce 
a single output file of the same type, with the option to specify the source and
destination tower addressing.  All of the python merge tools take a configuration 
file as input.  This configuration file specifies the input data sets and tower 
addressing. The -V option increases the verbosity of the diagnostic output.

The DAC settings generation tools produce configuration XML files providing values
to configure each channel.  The LAC, FLE, and, FHE tools take a configuration file
and a threshold energy as input.  The ULD file takes a ULD characterization
file and saturation margin value as input.  The -V option increases the verbosity of 
the diagnostic output.

Sample configuration scripts for these tools are included in the 
python/cfg folder.  
*/
