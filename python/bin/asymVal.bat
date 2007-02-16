REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/asymVal.bat,v 1.6 2006/06/21 18:43:12 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\asymVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: asymVal: CALIBGENCALROOT must be defined

:EXIT

