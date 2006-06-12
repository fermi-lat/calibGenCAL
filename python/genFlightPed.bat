REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/genFHEsettings.bat,v 1.4 2006/01/13 17:17:56 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\genFlightPed.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: genFHEsettings: CALIBGENCALROOT must be defined

:EXIT
