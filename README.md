# Leon's Display
### An ESP8266 microcontroller with a small screen to display relevant data of your choice

This project started when I became involved in ['Hollandse Luchten'](https://hollandse-luchten.org), a citizen science project in the Netherlands to measure particulate matter (PM2.5) in your environment. For that purpose, every member recieved a PM2.5 sensor to measure air quality. To be able to see the current PM2.5 level, it was necessary to go to a website with current readings. I thought that if that reading was displayed permanent, the user could better couple changes in the environment (smell of smoke, wind, fog, etc.) with changes in the PM2.5 reading.

So, I figured that a small display and an ESP microcontroller could do the trick. Figuring out the API to get the readings from the sensors was not that hard. Problem is that this value that can be retrieved from the server is the mean value, measured over an hour, an hour ago. Not very precise, but things will change in the future. I also added the possibility to read out Luftdaten Sensors as published on [sensor.community](https://sensor.community). I am now adding more sensor types. The current situation is that the following data can be shown:
* PM2.5 level from Sodaq Air sensors of ['Hollandse Luchten'](https://hollandse-luchten.org) (mean value, an hour ago)
* PM2.5 level of Luftdaten sensors via the [sensor.community](https://sensor.community) server (real-time value)
* PM2.5 level of Luftdaten sensors via the local interface of the sensor (real time value)
* Wind speed, gusts, direction and temperature for a given Lat/Long location
* PM2.5 level of Scapeler sensors (Future, work in progress)

![Leon's display showing a PM2.5 value](/leon's-display.jpg)

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

### Assembly

If you haven't bought the pre-soldered version, connect the display to the controller using some thin wire:
* VCC to 3V3
* SCL to D1
* SDA to D2
* GND to GND

Using some (hot)glue, put the display in the case (connector facing down, see picture). Slide the controller board into the horizontal slot of the case, see picture.
Close the case

Done.

If you connect the controller to power, using a USB-C cable, check if the display shows something.

## How to buy one

If you don't want to solder and flash the firmware, just email me for a pre-flashed controller and display or a soldered one. I also can build you a complete one, including the 3D printed casing.

### Prices

- Flashed controller and display to solder yourselve
- Flashed controller and display, soldered
- Flashed controller and display, soldered and assembled in a casing.

## How to use it

### Setup WiFi

To be able to operate, the display must be connected to the internet. This is done in multiple steps. The first step is that the display appears as an access point 'Setup Leon's Display'. Connect to this access point using the WiFi settings of your phone or laptop. Then open a browser and type the ip number that is on the display. In most cases it will be 192.168.1.4
Now the setup page of the display appears. Press the button 'Setup WiFi'. Fill in your WiFi settings (ssid and password). Reset the device as is shown. Press the reset button.
If everything went well, the device now connects to your WiFi network and can be accessed by typing `leon-s-display.local` in your browser.

Next step is to setup the sensor of your choice.

### Setup Sensors

#### Hollandse Luchten

To be able to show data of your Hollandse Luchten sensor, it is necessary to do some investigation. Goal is to find the sensor number to be filled in in the Setup pages of the display. This is done using your browser, using your sensor number. In this case, my sensor is used as an example. This sensor has number 465.

First of all, retrieve all data from the sensor. : `(https://api-samenmeten.rivm.nl/v1.0/Things?$filter=contains(name,‘HLL_hl_device_465’))`

This wil return the following response:
```
{
  "value": [
    {
      "@iot.id": 7270,
      "@iot.selfLink": "https://api-samenmeten.rivm.nl/v1.0/Things(7270)",
      "name": "HLL_hl_device_465",
      "description": "HLL_hl_device_465",
      "properties": {
        "codegemeente": "392",
        "knmicode": "knmi_06225",
        "nh3closecode": "NL10444",
        "nh3regiocode": "NL10444",
        "nh3stadcode": null,
        "no2closecode": "NL10550",
        "no2regiocode": "NL49564",
        "no2stadcode": "NL10550",
        "owner": "Hollandse Luchten",
        "pm10closecode": "NL10550",
        "pm10regiocode": "NL49564",
        "pm10stadcode": "NL10550",
        "pm25closecode": "NL49551",
        "pm25regiocode": "NL49703",
        "pm25stadcode": "NL49007",
        "project": "Hollandse Luchten"
      },
      "Locations@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Things(7270)/Locations",
      "Datastreams@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Things(7270)/Datastreams",
      "HistoricalLocations@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Things(7270)/HistoricalLocations"
    }
  ]
}
```

Now copy the link mentioned after `Datastreams`. So: `(https://api-samenmeten.rivm.nl/v1.0/Things(7270)/Datastreams)`

This results in the following response.
```
{
  "value": [
    {
      "@iot.id": 38619,
      "@iot.selfLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38619)",
      "name": "HLL_hl_device_465-6-rh",
      "description": "HLL_hl_device_465-6-rh",
      "unitOfMeasurement":
      {
        "definition": "https://qudt.org/vocab/unit/PERCENT",
        "symbol": "%"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "Thing@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38619)/Thing",
      "Sensor@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38619)/Sensor",
      "Observations@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38619)/Observations",
      "ObservedProperty@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38619)/ObservedProperty"
    },
    {
      "@iot.id": 38618,
      "@iot.selfLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38618)",
      "name": "HLL_hl_device_465-6-temp",
      "description": "HLL_hl_device_465-6-temp",
      "unitOfMeasurement":
      {
        "definition": "https://qudt.org/vocab/unit/DEG_C",
        "symbol": "C"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "Thing@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38618)/Thing",
      "Sensor@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38618)/Sensor",
      "Observations@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38618)/Observations",
      "ObservedProperty@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38618)/ObservedProperty"
    },
    {
      "@iot.id": 38617,
      "@iot.selfLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38617)",
      "name": "HLL_hl_device_465-12-pm25_kal",
      "description": "HLL_hl_device_465-12-pm25_kal",
      "unitOfMeasurement":
      {
        "definition": "https://qudt.org/vocab/unit/MicroGM-PER-M3",
        "symbol": "ug/m3"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "Thing@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38617)/Thing",
      "Sensor@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38617)/Sensor",
      "Observations@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38617)/Observations",
      "ObservedProperty@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38617)/ObservedProperty"
    },
    {
      "@iot.id": 38616,
      "@iot.selfLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38616)",
      "name": "HLL_hl_device_465-12-pm25",
      "description": "HLL_hl_device_465-12-pm25",
      "unitOfMeasurement":
      {
        "definition": "https://qudt.org/vocab/unit/MicroGM-PER-M3",
        "symbol": "ug/m3"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "Thing@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38616)/Thing",
      "Sensor@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38616)/Sensor",
      "Observations@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38616)/Observations",
      "ObservedProperty@iot.navigationLink": "https://api-samenmeten.rivm.nl/v1.0/Datastreams(38616)/ObservedProperty"
    }
  ]
}
```

This response contains all the data that the sensor returns. Temperature, Humidity, PM2.5 (calibrated) and PM2.5 (not calibrate). We take the calibrated version as it has received a correction to avoid too high values that aren't accurate. Using this PM2.5 value, look at he link after Observations and note the number that is mentioned there.

In this case it is number 38617. We have found it. Finally!

Now, go to the setup page of the display, choose 'Hollandse Luchten (Sodaq) as sensor and fill 38617 (fill in your number) as sensor number.

Done.

#### Sensor community (server)
#### Sensor community (local)
#### Open Meteo data (wind, temperatue)

In the 'Setup sensor' screen, select 'Open Meteo'. Now entering the lat/long coordinates of your desired location is important. The most easy way is to go to [Google Maps](https://maps.google.com) and go to the desired location, left-mousbutton press reveals the lat/long coordinates. For example '52xxxxxxx, 4xxxxxx' for the center of Haarlem.
