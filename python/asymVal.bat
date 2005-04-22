@echo off
setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%ROOTSYS%\bin;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\asymVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal