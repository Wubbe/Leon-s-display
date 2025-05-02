# Leon's Display
An ESP8266 microcontroller with a small screen to display relevant data

This project started when I became involved in 'Hollandse Luchten', a citizen science project to measure particulate matter (PM2.5) in your environment. For that purpose, every member recieved a PM2.5 sensor to measure air quality. To be able to see the current PM2.5 level, it was necessary to go to a website with current readings. I thought that if that reading was displayed permanent, the user could better couple changes in the environment (smell of smoke, wind, fog, etc.) with changes in the PM2.5 reading.

So, I figured that a small display and an ESP microcontroller could do the trick. Figuring out the API to get the readings from the sensors was not hard. Problem is that this value is the mean value, measured over an hour, an hour ago. Not very precise, but things will change in the future. I also added the possibility to read out Luftdaten Sensors as published on https://sensor.community. Later I added more. The current situation is that the following data can be shown:
* PM2.5 level from Sodaq Air sensors of Hollendse Luchten (mean value, an hour ago)
* PM2.5 level of Luftdaten sensors via the sensor.community server (real-time value)
* PM2.5 level of Luftdaten sensors via the local interface of the sensor (real time value)
* Wind speed, gusts, direction and temperature for a given Lat/Long location
* PM2.5 level of Scapeler sensors (Future)

# How to build one

What you need:
* Wemos D1 Mini V3, ESP8266 Development board. [Buy here](https://www.otronic.nl/en/wemos-d1-mini-v3-esp8266-wifi-ch340-development-bo.html)
* SSD1306 Mini OLED display 0.96 inch 128x64 I2C. [Buy here](https://www.otronic.nl/en/mini-oled-display-white-096-inch-128x64-i2c.html)
* A 3D printed case. [download here](https://www.thingiverse.com/thing:2884823)
* Some thin wire

Connect the display to the controller:
* VCC to 3V3
* SCL to D1
* SDA to D2
* GND to GND

[Firmware for the controller](https://github.com/Wubbe/Leon-s-display/blob/main/Leon-s-display.ino.bin)
