REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/dacSlopesVal.bat,v 1.2 2006/10/11 17:22:39 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\dacSlopesVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: dacSlopesVal: CALIBGENCALROOT must be defined

:EXIT

