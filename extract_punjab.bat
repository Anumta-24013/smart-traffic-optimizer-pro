@echo off
echo Extracting Punjab region from Pakistan data...
echo.

REM Punjab bounding box coordinates
REM North: 32.9 (top)
REM South: 28.0 (bottom)  
REM West: 69.3 (left)
REM East: 75.5 (right)

C:\osmosis\osmosis-0.49.2\bin\osmosis.bat ^
  --read-pbf file="pakistan-latest.osm.pbf" ^
  --bounding-box top=32.9 left=69.3 bottom=28.0 right=75.5 ^
  --write-xml file="data\punjab.osm"

echo.
echo Done! Punjab data saved to: data\punjab.osm
pause
