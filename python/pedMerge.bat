REM $Header$
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\pedMerge.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: pedMerge: CALIBGENCALROOT must be defined

:EXIT
