REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/muSlopeVal.bat,v 1.1 2006/06/26 20:45:20 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\muSlopeVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: pedVal: CALIBGENCALROOT must be defined

:EXIT
