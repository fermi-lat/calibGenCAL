REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/genLACsettings.bat,v 1.4 2006/01/13 17:17:56 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\genLACsettings.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT


:ERROR
echo ERROR: genLACsettings: CALIBGENCALROOT must be defined

:EXIT


