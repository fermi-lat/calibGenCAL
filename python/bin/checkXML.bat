REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/checkXML.bat,v 1.2 2006/06/21 18:43:12 dwood Exp $
@echo off

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\checkXML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal

