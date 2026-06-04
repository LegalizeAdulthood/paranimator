@echo off
pushd "%~dp0"
if errorlevel 1 exit /b 1
magick ^( "layers/layer-base-0001.png" -alpha set -channel A -evaluate multiply 1 +channel ^) ^( "layers/layer-detail-0001.png" -alpha set -channel A -evaluate multiply 1 +channel ^) -compose Screen -composite "frames/frame0001.png"
if errorlevel 1 exit /b 1
popd
