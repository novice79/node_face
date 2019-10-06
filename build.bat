@echo off
REM call node-gyp rebuild --msvs_version=2015 -C .\src --python=C:\Python27\python.exe
REM call node-gyp rebuild --msvs_version=2015 --python=C:\Python27\python.exe
call node-gyp rebuild --msvs_version=2019
REM cmake-js rebuild -G"Visual Studio 14 Win64" --CDVCPKG_TARGET_TRIPLET=x64-windows-static
REM cmake-js build "--CDVCPKG_TARGET_TRIPLET=x64-windows-static"
REM npm i --python=C:\Python27\python.exe
REM npm config set cmake_VCPKG_TARGET_TRIPLET "x64-windows-static" --global
REM VCPKG_DEFAULT_TRIPLET=x64-windows
REM REM call node-gyp rebuild --msvs_version=2015
REM pkg -t node10-win-x64 app.js
call copy /y build\Release\addon.node dist\addon.node
REM Copy-Item -Path build\Release\addon.node -Destination dist\addon.node