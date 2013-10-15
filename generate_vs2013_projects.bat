@echo off
SET BUILDFOLDER=build

IF NOT EXIST %BUILDFOLDER%\ (
mkdir %BUILDFOLDER% && echo INFO: output folder "%BUILDFOLDER%" created
)

cd %BUILDFOLDER%
cmake -G "Visual Studio 12" ..
cd ..

pause