// (Special "header" just for doxygen)

/*! @mainpage  package calibGenCAL

This package contains codes to generate and study calibration constants for CAL using muon data. The package includes following executables

<ul>
<li> <b>runMuonCalib.exe</b></b>. It reads data from a digi root file and produce 3 calibration constant files in ascii and XML format. It also produces two root files for studying quality of the calibration constants. The executable takes inputs from muonCalib_option.dat</li>
<li> <b>runMuonCalibChain.exe</b>. Its function is similar to the function of runMuonCalib.exe. runMuonCalibChain can read data from a list of digi root files. The executable takes inputs from muonCalibChain_option.dat.  runMuonCalibChain also generates light asymetry data.</li>
<li> <b>getPedestalsXML.exe</b> converts calibration constants for pedestals from ascii table to xml format. Usage: getPedestalsXML.exe ../output/pedestal.txt ../xml/pedestals.xml </li>
<li> <b>getCorrelatedPedestalsXML.exe</b> converts calibration constants for pedestals, using correlations betwen diodes, from ascii to xml format. Usage: getCorrelatedPedestalsXML.exe ../output/correlatedpedestal.txt ../xml/correlatedpedestals.xml </li>
<li> <b>getMuSlopesXML.exe</b> converts calibration constants for muon slope from ascii to xml format. Usage: getMuSlopesXML.exe ../output/muslopes.txt ../xml/muslopes.xml </li>
<li> <b>getGainsXML.exe</b> converts calibration constants for gain from ascii to xml format. Usage: getGainsXML.exe ../output/gains.txt ../xml/pedestals.xml </li>
<li>genNtuple.exe</b>, compare_constants.exe utility executables used to inspect calibration constants
<li> <b>runLightTaperCalib.exe</b>. It reads data from a digi root file and a recon root file. The executable takes inputs from lightTaperCalib_option.dat</li>
<li> <b>runLightTaperCalibChain.exe</b>. Its function is similar to the function of runLightTaperCalib.exe. The only differenece is that it can read data from a chain of digi root files and recon root files. The executable takes inputs from lightTaperCalibChain_option.dat</li>
<li> <b>ciFit.exe</b>.  Generates spline functions with integral nonlinearity constants.  Reads digi root files containing internal DAC calibration data.  Expected test configuration is described in code.   Reads options in from xml/IFile based config file ciFit_option.xml</li>
<li> <b> RootFileAnalysis class </b> This class provides generic root file input for Glast root data.  This functionality was extracted from RootAnalysis/RootFileAnalysis.exe.  It allows the user to treat an arbitrary collection of digi/mc/recon GLAST root data as a collection of events which may be stepped through/'seeked' into/rewound, etc...  This class is intended to be inherited by application specific class that provide the data structures/event loop/output code.
<li> <b>muonCalib class</b> This class provides data structures, data processing routines and file format/io for GLAST Cal muon based calibration.  This class is used by runMuonCalib*.exe as well as get*XML.exe.  
</ul>

*/
