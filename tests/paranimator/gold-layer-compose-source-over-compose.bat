magick ^( "output/layers/layer-base-0001.png" -alpha set -channel A -evaluate multiply 1 +channel ^) ^( "output/layers/layer-detail-0001.png" -alpha set -channel A -evaluate multiply 0.5 +channel ^) -compose Over -composite -background "black" -alpha remove -alpha off "output/frames/frame0001.png"
if errorlevel 1 exit /b 1
magick ^( "output/layers/layer-base-0002.png" -alpha set -channel A -evaluate multiply 1 +channel ^) ^( "output/layers/layer-detail-0002.png" -alpha set -channel A -evaluate multiply 0.5 +channel ^) -compose Over -composite -background "black" -alpha remove -alpha off "output/frames/frame0002.png"
if errorlevel 1 exit /b 1
magick ^( "output/layers/layer-base-0003.png" -alpha set -channel A -evaluate multiply 1 +channel ^) ^( "output/layers/layer-detail-0003.png" -alpha set -channel A -evaluate multiply 0.5 +channel ^) -compose Over -composite -background "black" -alpha remove -alpha off "output/frames/frame0003.png"
if errorlevel 1 exit /b 1
