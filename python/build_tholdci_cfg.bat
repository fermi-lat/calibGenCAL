REM $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/build_tholdci_cfg.bat,v 1.1 2006/03/14 22:42:43 fewtrell Exp $
@echo off

if not defined CALIBGENCALROOT goto :ERROR

setlocal
set PYTHONPATH=%CALIBGENCALROOT%\python\lib;%PYTHONPATH%;
python %CALIBGENCALROOT%\python\build_tholdci_cfg.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
goto :EXIT

:ERROR
echo ERROR: asymTXT2XML: CALIBGENCALROOT must be defined

:EXIT

