# LED-matrix
See https://github.com/hzeller/rpi-rgb-led-matrix for how to use the LED-matrix and how to compile.
If you are able to reproduce the examples there, copy the source from this repository into the main
directory of rpi-rgb-led-matrix.

For wiring the LED-matrix see 32x32_matrix_wiring.png.

For building use `./my_make.sh <file>`, without the file ending, e.g. `./my_make.sh mandelbrot`. Run it via `sudo ./mandelbrot`.

For pngs, png++ needs to be installed. If I remember correctly I built it from source.

Make also sure that paths are set correctly.

## Default setup
Requirements: A powerbank with two usb-ports, one for the raspi, the other for the LED-matrix, similar to [this](https://www.adafruit.com/product/1566).
The thing on the raspi usb-port (gps receiver) is not required for the default setup.

![Alt text](images/setup.on.back.jpg?raw=true "Setup") 

![Alt text](images/setup.on.front.jpg?raw=true "Setup turned on")

Futurama's hypnotoad consists of three pngs with varying eyes. Bikers passing by from behind will obey.

![Alt text](images/hypnotoad.jpg?raw=true "Hypnotoad with speed")

# Gps
## Requirements
1. Raspberry pi model B
2. NAVILOCK GPS NL-602U USB at usb-port closest to the raspi
3. Libraries: gpsd, gpsd-client, python-gps

## Description
1. First cronjob starts a bash-script which starts gpsd at boot up: **gps.sh**.
2. Second cronjob starts a bash-script which runs a python-script readíng out the wanted gps-data and saves it to file:
   1. **gpsspeed.sh** and **gpsspeed.py** save in **speed.txt**;
   2. **gpstime.sh** and **gpstime.py** save in **time.txt**.
3. Optional: Third cronjob starts the led-binary which reads out file containing the gps-data.
4. It might take some time until the NAVILOCK finds the gps signal. Won't work well in a closed room.

Found no other, more elegant way to get speed or time from gps yet. Clearly, speed needs not to be shown on the LED-matrix,
it can also be used for controlling the speed of the shown programs, e.g., pong runs faster for higher gps-speeds. Moreover,
via gpxlogger (included in gpsd or gps-client) it is possible to record the covered distance.

## Install libraries
`sudo apt-get install gpsd gpsd-client python-gps`

## Configure gpsd so that it is not started automatically
`sudo dpkg-reconfigure gpsd`

This is my configuration and might be unnecessary together with the corresponding cronjob (see below).

## Edit cronjobs via
`sudo crontab -e`

## Add the following lines to cronjobs (change &lt;path&gt; correspondingly)
`@reboot cd /home/pi/<path>/rpi-rgb-led-matrix; ./gps.sh &`

`@reboot cd /home/pi/<path>/rpi-rgb-led-matrix; ./gpsspeed.sh &`

The last command creates and updates /home/pi/&lt;path&gt;/rpi-rgb-led-matrix/speed.txt. Then reboot:

`sudo shutdown -r now`

## See gps.cc for an example
Compile via `./my_make.sh gps`.

## Running setup
Requirements: A rucksack. Tested with Salomon Trailrunning rucksack 12L or 20L. I recommend the 20L rucksack
because of higher volume. If you plan to use the setup during rainy weather, make sure to seal the contacts
via a plastic bag or similar.

![Alt text](images/setup.off.rucksack.outside.jpg?raw=true "Running setup (outside)")

![Alt text](images/setup.off.rucksack.inside.jpg?raw=true "Running setup (inside)")

The battery goes into the inside, the raspi and the gps receiver are stored in the mesh pocket at the left or right
outside of the rucksack. Zipper is closed where the cables of the battery leave the rucksack.

![Alt text](images/setup.on.rucksack.jpg?raw=true "Running setup turned on")
