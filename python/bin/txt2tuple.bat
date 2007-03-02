REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/txt2tuple.bat,v 1.1 2007/01/12 18:49:11 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\txt2tuple.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: txt2tuple: CALIBGENCALROOT must be defined

:EXIT