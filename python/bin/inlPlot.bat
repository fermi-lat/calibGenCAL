REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/inlPlot.bat,v 1.1 2007/12/13 21:56:14 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\inlPlot.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: inlPlot: CALIBGENCALROOT must be defined

:EXIT

