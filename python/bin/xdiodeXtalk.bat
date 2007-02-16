REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/xdiodeXtalk.bat,v 1.1 2006/10/03 21:12:24 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\xdiodeXtalk.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: xdiodeXtalk: CALIBGENCALROOT must be defined

:EXIT
