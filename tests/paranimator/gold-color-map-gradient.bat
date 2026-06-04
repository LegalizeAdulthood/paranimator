@echo off
pushd "%~dp0"
if errorlevel 1 exit /b 1
start/wait id batch=yes overwrite=yes savename=frame-0001.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0001
if errorlevel 1 exit /b 1
popd
