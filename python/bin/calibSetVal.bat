REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/calibSetVal.bat,v 1.1 2007/02/16 18:57:36 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\calibSetVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: calibSetVal: CALIBGENCALROOT must be defined

:EXIT

