magick ^( "output/layers/layer-base-0001.png" -alpha set -channel A -evaluate multiply 1 +channel ^) ^( "output/layers/layer-detail-0001.png" -alpha set -channel A -evaluate multiply 1 +channel ^) -compose Screen -composite "output/frames/frame0001.png"
if errorlevel 1 exit /b 1
