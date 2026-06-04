start/wait id batch=yes librarydirs=output @frames.par/layer-base-0001
if errorlevel 1 exit /b 1
start/wait id batch=yes librarydirs=output @frames.par/layer-detail-0001
if errorlevel 1 exit /b 1
start/wait id batch=yes librarydirs=output @frames.par/layer-base-0002
if errorlevel 1 exit /b 1
start/wait id batch=yes librarydirs=output @frames.par/layer-detail-0002
if errorlevel 1 exit /b 1
start/wait id batch=yes librarydirs=output @frames.par/layer-base-0003
if errorlevel 1 exit /b 1
start/wait id batch=yes librarydirs=output @frames.par/layer-detail-0003
if errorlevel 1 exit /b 1
