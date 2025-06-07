# Leon's Display
### An ESP8266 microcontroller with a small screen to display relevant data of your choice

This project started when I became involved in ['Hollandse Luchten'](https://hollandse-luchten.org), a citizen science project in the Netherlands to measure particulate matter (PM2.5) in your environment. For that purpose, every member recieved a PM2.5 sensor to measure air quality. To be able to see the current PM2.5 level, it was necessary to go to a website with current readings. I thought that if that reading was displayed permanent, the user could better couple changes in the environment (smell of smoke, wind, fog, etc.) with changes in the PM2.5 reading.

So, I figured that a small display and an ESP microcontroller could do the trick. Figuring out the API to get the readings from the sensors was not that hard. Problem is that this value that can be retrieved from the server is the mean value, measured over an hour, an hour ago. Not very precise, but things will change in the future. I also added the possibility to read out Luftdaten Sensors as published on [sensor.community](https://sensor.community). I am now adding more sensor types. The current situation is that the following data can be shown:
* PM2.5 level from Sodaq Air sensors of ['Hollandse Luchten'](https://hollandse-luchten.org) (mean value, an hour ago)
* PM2.5 level of Luftdaten sensors via the [sensor.community](https://sensor.community) server (real-time value)
* PM2.5 level of Luftdaten sensors via the local interface of the sensor (real time value)
* Wind speed, gusts, direction and temperature for a given Lat/Long location
* PM2.5 level of Scapeler sensors (Future, work in progress)

## How to build one

### What you need:
* Wemos D1 Mini V3, ESP8266 Development board. [Buy here](https://www.otronic.nl/en/wemos-d1-mini-v3-esp8266-wifi-ch340-development-bo.html)
* SSD1306 Mini OLED display 0.96 inch 128x64 I2C. [Buy here](https://www.otronic.nl/en/mini-oled-display-white-096-inch-128x64-i2c.html)
* A 3D printed case. [download here](https://www.thingiverse.com/thing:2884823)
* Some thin wire

 
Connect a USB cable to your computer and verify that the display has been connected well. Then flash the firmware to the controller. This can be done in two ways:
* download the [compiled firmware file](https://github.com/Wubbe/Leon-s-display/blob/main/Leon-s-display.ino.bin) and use a web flasher like [esp.huhn.me](https://esp.huhn.me/) or a dedicated flasher like [Nodemcu Pyflasher](https://github.com/marcelstoer/nodemcu-pyflasher) to write the firmware to the board (recommended).
* open the source file in the Arduino development environment and compile/build the file and write it to the controller (more difficult).
* Order a pre-flashed controller, including display and a 3D printed case by sending me an email. I can do the soldering also, if you prefer.

Future versions of the firmware can be updated via the interface of the controller itself. It has the possibility to upgrade OTA (over the air).

## How to use it

### Assembly

If you haven't bought the pre-soldered version, connect the display to the controller using some thin wire:
* VCC to 3V3
* SCL to D1
* SDA to D2
* GND to GND

Using some (hot)glue, put the display in the case (connector facing down, see picture).
Slide the controller board into the horizontal slot of the case, see picture.
Close the case

Done.

Of you connect the controller to power, using a USB-c cable, check if the display shows something.

### Setup WiFi

To be able to operate, the display must be connected to the internet. This is done in multiple steps. The first step is that the display appears as an access point 'Setup Leon's Display'.
Connect to this access point using the WiFi settings of your phone or laptop. Then open a browser and type the ip number that is on the display. In most cases it will be 192.168.1.4
Now the setup page of the display appears. Press the button 'Setup WiF'. Fill in your WiFi settings (ssid and password). Reset the device as is shown. Press the reset button.
If everything went well, the device now connects to your WiFi network and can be accessed by typing 'leon-s-display.local' in your browser.

### Setup Sensors

#### Hollandse Luchten
#### Sensor community (server)
#### Sensor community (local)
#### Open Meteo data (wind, temperatue)

In the 'Setup sensor' screen, select 'Open Meteo'. Now entering the lat/long coordinates of your desired location is important. The most easy way is to go to [Google Maps](https://maps.google.com) and go to the desirec location, left-mousbutton press reveals the lat/long coordinates. For example '52xxxxxxx, 4xxxxxx' for the center of Haarlem.
