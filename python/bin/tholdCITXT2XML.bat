REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/tholdCITXT2XML.bat,v 1.1 2008/02/02 23:25:39 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\tholdCITXT2XML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: tholdCITXT2XML: CALIBGENCALROOT must be defined

:EXIT
