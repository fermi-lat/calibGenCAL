REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/adcplot.bat,v 1.3 2006/06/21 18:43:12 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\adcplot.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: adcplot: CALIBGENCALROOT must be defined

:EXIT