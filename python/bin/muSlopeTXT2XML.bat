REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/muSlopeTXT2XML.bat,v 1.1 2006/06/28 22:08:02 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\muSlopeTXT2XML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: pedTXT2XML: CALIBGENCALROOT must be defined

:EXIT
