// (Special "header" just for doxygen)

/*! @mainpage  package calibGenCAL

This package contains codes to generate and study calibration constants for CAL using muon data. The package includes following executables

calibGenCAL v2r5 performs a variety of muon calibrations.  These include:
 - Pedestals – Noise pedestal position and width are measured.
 - Mu Peaks (Gains) – Mu Peak is the position of the muon peak in ADC units, pedestal subtracted and corrected for path length.  Gain, which is stored in the output xml file, is 11.2 MeV/Mu Peak (note that this is not a proper “gain”, but rather an ADC bin width).
 - Mu Slopes – Conversion from asymmetry to position, measured as the slope of the linear portion of the asymmetry curve (central half of the curve).  Based on (L1-L2)/(L1+L2) definition of asymmetry.
 - Light Asymmetry – Light asymmetry vs position represented by a table of 12 spline points.


\b Applications:
 -  \b runMuonCalib.exe. It reads data from one or more digi root files and produce 4 calibration constant files in both ascii and XML format (ped,gain,asym,muon_slope). It also produces two root files for studying quality of the calibration constants. The executable takes inputs from muonCalib_option.dat
 -  \b getPedestalsXML.exe converts calibration constants for pedestals from ascii table to xml format. Usage: getPedestalsXML.exe ../output/pedestal.txt ../xml/pedestals.xml 
 -  \b getCorrelatedPedestalsXML.exe converts calibration constants for pedestals, using correlations betwen diodes, from ascii to xml format. Usage: getCorrelatedPedestalsXML.exe ../output/correlatedpedestal.txt ../xml/correlatedpedestals.xml 
 -  \b getMuSlopesXML.exe converts calibration constants for muon slope from ascii to xml format. Usage: getMuSlopesXML.exe ../output/muslopes.txt ../xml/muslopes.xml 
 -  \b getGainsXML.exe converts calibration constants for gain from ascii to xml format. Usage: getGainsXML.exe ../output/gains.txt ../xml/pedestals.xml 
 - genNtuple.exe</b>, compare_constants.exe utility executables used to inspect calibration constants
 -  \b runLightTaperCalib.exe. It reads data from a digi root file and a recon root file. The executable takes inputs from lightTaperCalib_option.dat
 -  \b runLightTaperCalibChain.exe. Its function is similar to the function of runLightTaperCalib.exe. The only differenece is that it can read data from a chain of digi root files and recon root files. The executable takes inputs from lightTaperCalibChain_option.dat
 -  \b ciFit.exe.  Generates spline functions with integral nonlinearity constants.  Reads digi root files containing internal DAC calibration data.  Expected test configuration is described in code.   Reads options in from xml/IFile based config file ciFit_option.xml

\b Classes:
 - \b  RootFileAnalysis This class provides generic root file input for Glast root data.  This functionality was extracted from RootAnalysis/RootFileAnalysis.exe.  It allows the user to treat an arbitrary collection of digi/mc/recon GLAST root data as a collection of events which may be stepped through/'seeked' into/rewound, etc...  This class is intended to be inherited by application specific class that provide the data structures/event loop/output code.
 -  \b muonCalib This class provides data structures, data processing routines and file format/io for GLAST Cal muon based calibration.  This class is used by runMuonCalib*.exe as well as get*XML.exe.  

*/
