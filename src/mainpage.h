// (Special "header" just for doxygen)

/*! @mainpage  package calibGenCAL

This package contains codes to generate and study calibration constants for CAL using muon data. The package includes following executables

<ul>
<li> runMuonCalib.exe. It reads data from a digi root file and produce 3 calibration constant files in ascii and XML format. It also produces two root files for studying quality of the calibration constants. The executable takes inputs from muonCalib_option.dat</li>
<li> runMuonCalibChain.exe. Its function is similar to the function of runMuonCalib.exe. runMuonCalibChain can read data from a chain of digi root files. The executable takes inputs from muonCalibChain_option.dat.  runMuonCalibChain also generates light asymetry data.</li>
<li> getPedestalsXML.exe produces calibration constants for pedestal in xml format. Usage: getPedestalsXML.exe ../output/pedestal.txt ../xml/pedestals.xml </li>
<li> getCorrelatedPedestalsXML.exe produces calibration constants for pedestals, using correlations betwen diodes, in xml format. Usage: getCorrelatedPedestalsXML.exe ../output/correlatedpedestal.txt ../xml/correlatedpedestals.xml </li>
<li> getMuSlopesXML.exe produces calibration constants for muon slope in xml format. Usage: getMuSlopesXML.exe ../output/muslopes.txt ../xml/muslopes.xml </li>
<li> getGainsXML.exe produces calibration constants for gain in xml format. Usage: getGainsXML.exe ../output/gains.txt ../xml/pedestals.xml </li>
<li>genNtuple.exe, compare_constants.exe utility executables used to inspect calibration constants
<li> runLightTaperCalib.exe. It reads data from a digi root file and a recon root file. The executable takes inputs from lightTaperCalib_option.dat</li>
<li> runLightTaperCalibChain.exe. Its function is similar to the function of runLightTaperCalib.exe. The only differenece is that it can read data from a chain of digi root files and recon root files. The executable takes inputs from lightTaperCalibChain_option.dat</li>
<li> ciFit.exe.  Generates spline functions with nonlinearity constants.  Reads digi root files containing internal DAC calibration data.  Expected test configuration is described in code.   Reads options in from k
</li>
</ul>

*/
