REM $Header$
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%ROOTSYS%\bin;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\biasVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: biasVal: CALIBGENCALROOT must be defined

:EXIT

