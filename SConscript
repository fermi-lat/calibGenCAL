# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/SConscript,v 1.2 2008/09/23 17:30:16 glastrm Exp $ 
# Authors: Zachary Fewtrell <zfewtrell@ssd5.nrl.navy.mil>
# Version: calibGenCAL-05-09-01
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('calibGenCALLib', depsOnly = 1)
calibGenCAL = libEnv.SharedLibrary('calibGenCAL', listFiles(['src/lib/*.cxx','src/lib/CalibDataTypes/*.cxx',
                                                             'src/lib/Hists/*.cxx','src/lib/Specs/*.cxx',
                                                             'src/lib/Util/*.cxx','src/lib/Algs/*.cxx']))

progEnv.Tool('calibGenCALLib')
progEnv.Tool('addLibrary', library = baseEnv['rootLibs'])

genMuonPed = progEnv.Program('genMuonPed',['src/Ped/genMuonPed.cxx']+['src/Ped/MuonPedAlg.cxx'])
genCIDAC2ADC = progEnv.Program('genCIDAC2ADC',['src/CIDAC2ADC/genCIDAC2ADC.cxx']+['src/CIDAC2ADC/IntNonlinAlg.cxx'])
smoothCIDAC2ADC = progEnv.Program('smoothCIDAC2ADC',['src/CIDAC2ADC/smoothCIDAC2ADC.cxx'])
splitDigi = progEnv.Program('splitDigi',['src/Util/splitDigi.cxx'])
sumHists = progEnv.Program('sumHists',['src/Util/sumHists.cxx'])
genNeighborXtalk = progEnv.Program('genNeighborXtalk',['src/CIDAC2ADC/genNeighborXtalk.cxx']+['src/CIDAC2ADC/NeighborXtalkAlg.cxx'])
genMuonAsym = progEnv.Program('genMuonAsym',['src/Optical/genMuonAsym.cxx']+['src/Optical/MuonAsymAlg.cxx'])
genMuonMPD = progEnv.Program('genMuonMPD',['src/Optical/genMuonMPD.cxx']+['src/Optical/MuonMPDAlg.cxx'])
genGCRHists = progEnv.Program('genGCRHists',['src/Optical/genGCRHists.cxx']+['src/Optical/GCRCalibAlg.cxx'])
fitGCRHists = progEnv.Program('fitGCRHists',['src/Optical/fitGCRHists.cxx'])
genMuonCalibTkr = progEnv.Program('genMuonCalibTkr',['src/Optical/genMuonCalibTkr.cxx']+['src/Optical/MuonCalibTkrAlg.cxx'])
fitMuonCalibTkr = progEnv.Program('fitMuonCalibTkr',['src/Optical/fitMuonCalibTkr.cxx'])
genLACHists = progEnv.Program('genLACHists',['src/Thresh/genLACHists.cxx'])
fitLACHists = progEnv.Program('fitLACHists',['src/Thresh/fitLACHists.cxx'])
fitThreshSlopes = progEnv.Program('fitThreshSlopes',['src/Thresh/fitThreshSlopes.cxx'])
genFLEHists = progEnv.Program('genFLEHists',['src/Thresh/genFLEHists.cxx']+['src/Thresh/LPAFleAlg.cxx'])
genFHEHists = progEnv.Program('genFHEHists',['src/Thresh/genFHEHists.cxx']+['src/Thresh/LPAFheAlg.cxx'])
fitTrigHists = progEnv.Program('fitTrigHists',['src/Thresh/fitTrigHists.cxx'])
genULDHists = progEnv.Program('genULDHists',['src/Thresh/genULDHists.cxx'])
fitULDHists = progEnv.Program('fitULDHists',['src/Thresh/fitULDHists.cxx'])
fitULDSlopes = progEnv.Program('fitULDSlopes',['src/Thresh/fitULDSlopes.cxx'])
genTrigMonitorHists = progEnv.Program('genTrigMonitorHists',['src/Thresh/genTrigMonitorHists.cxx'])
fitTrigMonitorHists = progEnv.Program('fitTrigMonitorHists',['src/Thresh/fitTrigMonitorHists.cxx'])
genAliveHists = progEnv.Program('genAliveHists',['src/Thresh/genAliveHists.cxx'])
genSciLACHists = progEnv.Program('genSciLACHists',['src/Thresh/genSciLACHists.cxx'])

progEnv.Tool('registerObjects', package = 'calibGenCAL', libraries = [calibGenCAL],
             binaries = [genMuonPed,genCIDAC2ADC,smoothCIDAC2ADC,splitDigi,sumHists,
                         genNeighborXtalk,genMuonAsym,genMuonMPD,genGCRHists,
                         fitGCRHists,genMuonCalibTkr,fitMuonCalibTkr,genLACHists
                         ,fitLACHists,fitThreshSlopes,genFLEHists,genFHEHists,
                         fitTrigHists,genULDHists,fitULDHists,fitULDSlopes,
                         genTrigMonitorHists,fitTrigMonitorHists,genAliveHists,
                         genSciLACHists],includes = listFiles(['calibGenCAL/*.h']))