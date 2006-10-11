REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/dacSlopesVal.bat,v 1.1 2006/07/17 21:20:49 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\dacSlopesPlot.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: dacSlopesPlot: CALIBGENCALROOT must be defined

:EXIT

