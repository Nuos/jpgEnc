@echo off
SET BUILDFOLDER=build-x64

IF NOT EXIST %BUILDFOLDER%\ (
mkdir %BUILDFOLDER% && echo INFO: output folder "%BUILDFOLDER%" created
)

cd %BUILDFOLDER%
cmake -G "Visual Studio 12 Win64" ..
cd ..

pause