REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/checkXML.bat,v 1.1 2006/04/03 17:14:26 dwood Exp $
@echo off

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\checkXML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal

