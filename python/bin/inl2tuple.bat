REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/inl2tuple.bat,v 1.1 2007/01/16 17:22:14 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%ROOTSYS%\bin;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\inl2tuple.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: inl2tuple: CALIBGENCALROOT must be defined

:EXIT
