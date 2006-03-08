REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/mpdXML2TXT.bat,v 1.2 2006/01/13 17:17:55 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\mpdXML2TXT.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: mpdXML2TXT: CALIBGENCALROOT must be defined

:EXIT