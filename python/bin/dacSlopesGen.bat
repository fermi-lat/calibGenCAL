REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/dacSlopesGen.bat,v 1.2 2006/10/11 17:22:39 dwood Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR


setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\dacSlopesGen.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: dacSlopesGen: CALIBGENCALROOT must be defined

:EXIT



