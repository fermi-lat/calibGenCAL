REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/intNonlinVal.bat,v 1.5 2006/01/13 17:17:57 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\intNonlinVal.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: intNonlinVal: CALIBGENCALROOT must be defined

:EXIT
