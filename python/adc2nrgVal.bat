REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/adc2nrgVal.bat,v 1.2 2006/01/13 17:17:54 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\adc2nrgVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: adc2nrgVal: CALIBGENCALROOT must be defined

:EXIT

