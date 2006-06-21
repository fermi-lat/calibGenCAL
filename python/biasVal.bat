REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/biasVal.bat,v 1.2 2006/01/13 17:17:55 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\biasVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: biasVal: CALIBGENCALROOT must be defined

:EXIT

