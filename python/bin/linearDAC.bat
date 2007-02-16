REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/linearDAC.bat,v 1.1 2007/01/24 16:36:21 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\linearDAC.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: linearDAC: CALIBGENCALROOT must be defined

:EXIT
