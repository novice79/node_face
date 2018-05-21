REM call node-gyp rebuild --msvs_version=2015 -C .\src --python=C:\Python27\python.exe
call node-gyp rebuild --msvs_version=2015 --python=C:\Python27\python.exe
REM cmake-js rebuild -G"Visual Studio 14 Win64" --CDVCPKG_TARGET_TRIPLET=x64-windows-static
REM cmake-js build "--CDVCPKG_TARGET_TRIPLET=x64-windows-static"
REM npm i --python=C:\Python27\python.exe
REM npm config set cmake_VCPKG_TARGET_TRIPLET "x64-windows-static" --global
REM VCPKG_DEFAULT_TRIPLET=x64-windows
REM REM call node-gyp rebuild --msvs_version=2015