REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/asymXML2TXT.bat,v 1.1 2007/02/16 18:57:34 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\asymXML2TXT.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: asymXML2TXT: CALIBGENCALROOT must be defined

:EXIT