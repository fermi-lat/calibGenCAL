REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/charVal.bat,v 1.2 2006/06/21 18:43:12 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\charVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: charVal: CALIBGENCALROOT must be defined

:EXIT
