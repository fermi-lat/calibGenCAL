REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/tholdCIDiff.bat,v 1.1 2008/04/21 20:25:21 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR


setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\tholdCIDiff.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: tholdCIDiff: CALIBGENCALROOT must be defined

:EXIT



