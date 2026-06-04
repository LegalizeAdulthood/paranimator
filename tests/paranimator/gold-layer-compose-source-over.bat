@echo off
pushd "%~dp0"
if errorlevel 1 exit /b 1
if not exist "layers" mkdir "layers"
start/wait id batch=yes overwrite=yes savename=layer-base-0001.png savedir=. librarydirs=. video=F6 @frames.par/layer-base-0001
if errorlevel 1 exit /b 1
move /y "image/layer-base-0001.png" "layers/layer-base-0001.png"
if errorlevel 1 exit /b 1
if not exist "layers" mkdir "layers"
start/wait id batch=yes overwrite=yes savename=layer-detail-0001.png savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0001
if errorlevel 1 exit /b 1
move /y "image/layer-detail-0001.png" "layers/layer-detail-0001.png"
if errorlevel 1 exit /b 1
if not exist "layers" mkdir "layers"
start/wait id batch=yes overwrite=yes savename=layer-base-0002.png savedir=. librarydirs=. video=F6 @frames.par/layer-base-0002
if errorlevel 1 exit /b 1
move /y "image/layer-base-0002.png" "layers/layer-base-0002.png"
if errorlevel 1 exit /b 1
if not exist "layers" mkdir "layers"
start/wait id batch=yes overwrite=yes savename=layer-detail-0002.png savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0002
if errorlevel 1 exit /b 1
move /y "image/layer-detail-0002.png" "layers/layer-detail-0002.png"
if errorlevel 1 exit /b 1
if not exist "layers" mkdir "layers"
start/wait id batch=yes overwrite=yes savename=layer-base-0003.png savedir=. librarydirs=. video=F6 @frames.par/layer-base-0003
if errorlevel 1 exit /b 1
move /y "image/layer-base-0003.png" "layers/layer-base-0003.png"
if errorlevel 1 exit /b 1
if not exist "layers" mkdir "layers"
start/wait id batch=yes overwrite=yes savename=layer-detail-0003.png savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0003
if errorlevel 1 exit /b 1
move /y "image/layer-detail-0003.png" "layers/layer-detail-0003.png"
if errorlevel 1 exit /b 1
popd
