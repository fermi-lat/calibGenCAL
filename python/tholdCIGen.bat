REM $Header$
@echo off

if not defined CALIBGENCALROOT goto :ERROR


setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\tholdCIGen.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: tholdCIGen: CALIBGENCALROOT must be defined

:EXIT



