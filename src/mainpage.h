// (Special "header" just for doxygen)

/*! @mainpage  package calibGenCAL

This package contains codes to generate and study calibration constants for CAL using muon data. The package includes following executables

calibGenCAL v2r5 performs a variety of muon calibrations.  These include:
 - Pedestals – Noise pedestal position and width are measured.
 - Light Asymmetry – Light asymmetry vs position represented by a table of 12 spline points.
 - Integral Nonlinearity - adc2dac conversion in all 4 ranges.  Corrects for nonlinearity in adc scale.
 - MevPerDac - Relastionship of total energy deposited to the sum of the DAC values at both ends of the xtal.  Onboard DAC scale units are used for calculation b/c it is more linear than ADC scale.
 
\b Applications:
 -  \b runMuonCalib.exe. It reads data from one or more digi root files and produces calibration constant files in both ascii and XML format (pedestals,asymetry and MevPerDac). It also produces root histogram files for studying the quality of the calibration constants. The executable takes input from muonCalib_option.xml.
 -  \b genNtuple.exe</b>, compare_constants.exe utility executables used to inspect calibration constants
 -  \b runLightTaperCalib.exe. It reads data from a digi root file and a recon root file. The executable takes inputs from lightTaperCalib_option.dat
 -  \b runLightTaperCalibChain.exe. Its function is similar to the function of runLightTaperCalib.exe. The only differenece is that it can read data from a chain of digi root files and recon root files. The executable takes inputs from lightTaperCalibChain_option.dat
 -  \b ciFit.exe.  Generates spline functions with integral nonlinearity constants.  Reads digi root files containing internal DAC calibration data.  Expected test configuration is described in code.   Reads options in from xml/IFile based config file ciFit_option.xml

\b Classes:
 - \b  RootFileAnalysis This class provides generic root file input for Glast root data.  This functionality was extracted from RootAnalysis/RootFileAnalysis.exe.  It allows the user to treat an arbitrary collection of digi/mc/recon GLAST root data as a collection of events which may be stepped through/'seeked' into/rewound, etc...  This class is intended to be inherited by application specific class that provide the data structures/event loop/output code.
 -  \b muonCalib This class provides data structures, data processing routines and file format/io for GLAST Cal muon based calibration.  This class is used by runMuonCalib*.exe as well as get*XML.exe.  

*/
