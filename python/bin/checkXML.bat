REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/checkXML.bat,v 1.1 2007/02/16 18:57:35 dwood Exp $
@echo off

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\checkXML.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal

