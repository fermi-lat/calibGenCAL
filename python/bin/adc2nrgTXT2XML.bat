REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/adc2nrgTXT2XML.bat,v 1.1 2007/02/16 18:57:34 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\adc2nrgTXT2XML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto EXIT

:ERROR
echo ERROR: adc2nrgTXT2XML: CALIBGENCALROOT must be defined

:EXIT

