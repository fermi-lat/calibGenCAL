@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%ROOTSYS%\bin;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\pedVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: pedVal: CALIBGENCALROOT must be defined

:EXIT
