REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/genFlightPed.bat,v 1.1 2006/06/12 19:53:49 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\genFlightPed.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: genFHEsettings: CALIBGENCALROOT must be defined

:EXIT
