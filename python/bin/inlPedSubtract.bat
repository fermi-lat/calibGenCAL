REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/inlPedSubtract.bat,v 1.1 2006/09/12 19:34:25 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\inlPedSubtract.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: inlPedSubtract: CALIBGENCALROOT must be defined

:EXIT
