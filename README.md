# Led-matrix
See https://github.com/hzeller/rpi-rgb-led-matrix how to use the led-matrix and how to compile.
If you are able to reproduce the examples there, copy the source from this repository into the
main directory of rpi-rgb-led-matrix.

For wiring the led-matrix see 32x32_matrix_wiring.png.

For building use ./my_make.sh &lt;file&gt;, without the file ending, e.g. ./my_make.sh mandelbrot. Run it via sudo ./mandelbrot.

For pngs, png++ needs to be installed. If I remember correctly I built it from source.

Make also sure that paths are set correctly.

# Gps
## Requirements
1. Raspberry pi model B
2. NAVILOCK GPS NL-602U USB at usb-port closest to the raspi
3. Libraries: gpsd, gpsd-client, python-gps

## Description
1. First cronjob is starting a bash-script running gpsd at boot up: gps.sh
2. Second cronjob is starting a bash-script running a python-script which reads out the wanted gps-data and saves it in file: gpsspeed.sh and gpsspeed.py as well as gpstime.sh and gpstime.py. Found no more elegant way to get speed from gps.
3. Third cronjob is starting the led-binary which reads out file containing the gps-data.
4. It might take some time until the NAVILOCK finds the gps signal. Won't work in a closed room.

## Install libraries
sudo apt-get install gpsd gpsd-client python-gps

## configure gpsd so that it is not started automatically
sudo dpkg-reconfigure gpsd

This is my configuration and this step might be unnecessary.

## Edit cronjobs via
sudo crontab -e

## Add the following (change &lt;path&gt; correspondingly)
@reboot cd /home/pi/&lt;path&gt;/rpi-rgb-led-matrix; ./gps.sh &
@reboot cd /home/pi/&lt;path&gt;/rpi-rgb-led-matrix; ./gpsspeed.sh &

The last command creates and updates /home/pi/&lt;path&gt;/rpi-rgb-led-matrix/speed.txt.

## See gps.cc for an example
Compile via my_make.sh gps.

