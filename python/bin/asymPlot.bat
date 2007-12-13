REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/asymPlot.bat,v 1.2 2007/03/27 18:50:47 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\asymPlot.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: asymPlot: CALIBGENCALROOT must be defined

:EXIT

