REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/charVal.bat,v 1.1 2006/01/25 23:28:50 dwood Exp $
@echo off

setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%ROOTSYS%\bin;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\checkXML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal

