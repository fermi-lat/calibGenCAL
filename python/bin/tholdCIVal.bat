REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/tholdCIVal.bat,v 1.4 2006/06/21 18:43:13 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\tholdCIVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: tholdCIVal: CALIBGENCALROOT must be defined

:EXIT

