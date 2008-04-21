REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/dacSlopesDiff.bat,v 1.1 2008/04/21 14:37:04 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR


setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\dacSlopesDiff.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: dacSlopesDiff: CALIBGENCALROOT must be defined

:EXIT



