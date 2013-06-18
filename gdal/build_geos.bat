@echo off

set vcvars="D:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
echo %vcvars%
call %vcvars%

call autogen.bat
if /I '%1'=='' (
goto release
)

if /I '%1'=='r' (
goto release
)

if /I '%1'=='c' (
nmake /f makefile.vc clean
goto end
)

if /I '%1'=='d' (
nmake /f makefile.vc MSVC_VER=1500 BUILD_DEBUG==YES
goto end
)

:release
nmake /f makefile.vc MSVC_VER=1500
copy /Y .\src\geos_c.dll ..\gdal\build\bin

:end
pause
