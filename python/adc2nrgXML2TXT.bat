REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/adc2nrgXML2TXT.bat,v 1.2 2006/06/21 18:43:13 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\adc2nrgXML2TXT.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: adc2nrgXML2TXT: CALIBGENCALROOT must be defined

:EXIT
