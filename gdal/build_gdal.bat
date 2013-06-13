@echo off

set vcvars="D:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
echo %vcvars%
call %vcvars%

nmake /f makefile.vc
nmake /f makefile.vc devinstall

pause
