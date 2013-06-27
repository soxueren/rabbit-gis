@echo off

cd .\swig\python
if exist build RD /S /Q build 

rem set PATH=%PATH%;E:\gdal\build\bin
rem set GDAL_DATA=E:\gdal\build\data

python setup.py build

cd ..\..\

copy /Y .\build\bin\*.dll .\swig\python\build\lib.win32-2.7\osgeo

pause
