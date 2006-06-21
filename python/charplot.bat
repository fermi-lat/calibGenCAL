REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/charplot.bat,v 1.1 2006/01/26 19:38:12 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\charplot.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: charplot: CALIBGENCALROOT must be defined

:EXIT