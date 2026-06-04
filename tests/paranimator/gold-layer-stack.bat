@echo off
pushd "%~dp0"
if errorlevel 1 exit /b 1
start/wait id batch=yes overwrite=yes savename=layer-base-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0001
if errorlevel 1 exit /b 1
start/wait id batch=yes overwrite=yes savename=layer-detail-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0001
if errorlevel 1 exit /b 1
start/wait id batch=yes overwrite=yes savename=layer-base-0002.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0002
if errorlevel 1 exit /b 1
start/wait id batch=yes overwrite=yes savename=layer-detail-0002.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0002
if errorlevel 1 exit /b 1
start/wait id batch=yes overwrite=yes savename=layer-base-0003.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0003
if errorlevel 1 exit /b 1
start/wait id batch=yes overwrite=yes savename=layer-detail-0003.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0003
if errorlevel 1 exit /b 1
popd
