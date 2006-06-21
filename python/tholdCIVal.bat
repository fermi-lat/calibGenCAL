REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/tholdCIVal.bat,v 1.3 2006/01/13 17:17:57 fewtrell Exp $
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

