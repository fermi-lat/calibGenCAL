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
*/
