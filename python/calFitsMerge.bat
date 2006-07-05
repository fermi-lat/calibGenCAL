REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/calFitsMerge.bat,v 1.5 2006/06/21 18:43:13 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\calFitsMerge.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: calFitsMerge: CALIBGENCALROOT must be defined

:EXIT