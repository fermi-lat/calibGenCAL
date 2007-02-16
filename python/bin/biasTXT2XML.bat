REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/biasTXT2XML.bat,v 1.1 2006/07/09 16:42:57 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\biasTXT2XML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: biasTXT2XML: CALIBGENCALROOT must be defined

:EXIT
