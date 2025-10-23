# Leon's Display
### An ESP8266 microcontroller with a small screen to display relevant data of your choice

This project started when I became involved in ['Hollandse Luchten'](https://hollandse-luchten.org), a citizen science project in the Netherlands to measure particulate matter (PM<sub>2.5</sub>) in your environment. For that purpose, every member recieved a PM<sub>2.5</sub> sensor to measure the air quality. To be able to see the current PM<sub>2.5</sub> level, it was necessary to go to a website with current (or historic) readings. I thought that if that value was displayed permanently, the user could better couple changes in the environment (smell of smoke, wind, fog, etc.) with changes in the PM<sub>2.5</sub> value and thus in air quality.

So, I figured that a small display and an ESP microcontroller could do the trick. Figuring out the API to get the readings from the sensors was not that hard. Problem with the server of Hollandse Luchten is that the PM<sub>2.5</sub> value that can be retrieved is the mean value, measured over an hour, an hour ago. Not very precise, but things will change in the future (I hope). I also added the possibility to read out values from Luftdaten Sensors, as published on [sensor.community](https://sensor.community). I am now adding more sensor types. Also I added the possibility to show wind speed (and gusts) and temperature from a specified location on the world.

The current situation is that the following data can be shown:
* PM<sub>2.5</sub> level from Sodaq Air sensors of ['Hollandse Luchten'](https://hollandse-luchten.org) (mean value, an hour ago)
* PM<sub>2.5</sub> level of Luftdaten sensors via the [sensor.community](https://sensor.community) server (real-time value)
* PM<sub>2.5</sub> level of Luftdaten sensors via the local interface of the sensor (real time value)
* Wind speed, gusts, direction and temperature for a given Lat/Long location (real time value)
* PM<sub>2.5</sub> level of Scapeler sensors (future, work in progress)

![Leon's display showing a PM2.5 value](/leon's-display.jpg)

## How to build one

### What you need:
* Wemos D1 Mini V3 or V4, ESP8266 Development board. [Buy here](https://www.otronic.nl/en/wemos-d1-mini-v3-esp8266-wifi-ch340-development-bo.html)
* SSD1306 Mini OLED display 0.96 inch 128x64 I2C. [Buy here](https://www.otronic.nl/en/mini-oled-display-white-096-inch-128x64-i2c.html)
* A 3D printed case. [I have found this case to print, but it isn't perfect](https://www.thingiverse.com/thing:2884823). The development board does not fit perfectly in this case. I am still searching for a better case.
* Some thin wire

### Assembly

If you haven't bought the pre-soldered version, connect the display to the controller using some thin wire:
* VCC to 3V3
* SCL to D1
* SDA to D2
* GND to GND

Using some (hot)glue, put the display in the case (connector facing up, see picture). Slide the controller board into the horizontal slot of the case, see picture.
Close the case.
 
Connect a USB cable to your computer for power and verify that the display has been connected well. It should display something. If it does, you are ready for the next step.
Then flash the firmware to the controller. This can be done in two ways:
* download the [compiled firmware file](https://github.com/Wubbe/Leon-s-display/blob/main/Leon-s-display.ino.bin) and use a web flasher like [esp.huhn.me](https://esp.huhn.me/) or a dedicated flasher like [Nodemcu Pyflasher](https://github.com/marcelstoer/nodemcu-pyflasher) to write the firmware to the board (recommended).
* Order a pre-flashed controller, including display and a optionally a 3D printed case by sending me an email. I can do the soldering also, if you prefer.

Future versions of the firmware can be updated via the interface of the controller itself. It has the possibility to upgrade OTA (over the air).

## How to buy one

If you don't want to solder and flash the firmware, just email me for a pre-flashed controller and display or a soldered one. I also can build you a complete one, including the 3D printed case.

### Prices

- Flashed controller and display to solder yourselve. € 12,50 (ex. shipping)
- Flashed controller and display, soldered. € 15,- (ex. shipping)
- Flashed controller and display, soldered and assembled in a casing: not available yet. I have used the proposed case in a few devices but the case could be improved. If I have foud the perfect case, I will offer a display, including the case.

## How to use it

If the device is build and assembled well, with the right firmware, it must be configured to retrieve and show the data of your choice. This is achieved in two steps:
- Setup WiFi
- Setup Sensor

### Setup WiFi

To be able to operate, the device must be connected to the internet. This is done in multiple steps. The first step is that after the display is connected to power (a usb charger) the display appears as a WiFi access point with the name `Setup Leon's Display`. Connect to this access point using the WiFi settings of your phone or laptop. Then open a browser and type the ip number that is mentioned on the display. In most cases it will be `192.168.1.4`. Now the setup page of the display appears in your browser. Press the button 'Setup WiFi'. Fill in your WiFi settings (ssid and password) and reset the device as is shown.

If everything went well, the device connects to your WiFi network and can be accessed by typing `leon-s-display.local` in your browser. If that does not work look at the display and type the ip number that is mentioned there. Again, the Setup page of the display should appear in your browser.

Next step is to setup the sensor of your choice.

### Setup Sensors

Select the sensor of your choice in the `Setup Sensor` screen.

#### Hollandse Luchten

To be able to show data of your Hollandse Luchten sensor, it is necessary to do some investigation. Goal is to find the sensor number to be filled in on the Setup page of the display. This is done using your browser, using your sensor number. In this case, my (old) sensor is used as an example. This sensor has number 465.

First of all, retrieve all data from the sensor:
[`https://api-samenmeten.rivm.nl/v1.0/Things?$filter=contains(name,'HLL_hl_device_465')`](https://api-samenmeten.rivm.nl/v1.0/Things?$filter=contains(name,'HLL_hl_device_465')) (change 465 into the number of your sensor)

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

Now copy the link mentioned after `Datastreams`. So: [`https://api-samenmeten.rivm.nl/v1.0/Things(7270)/Datastreams`](https://api-samenmeten.rivm.nl/v1.0/Things(7270)/Datastreams)

This results in the following response containing a list of measurements the sensor can return.
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

This response contains all the data that the sensor returns. Temperature, Humidity, PM<sub>2.5</sub> (calibrated) and PM<sub>2.5</sub> (not calibrated). We take the calibrated version as it has a correction to avoid too high values that aren't accurate. Using this PM<sub>2.5</sub> value, look at he link after Observations and note the number that is mentioned there. In this case it is number 38617.
We have found it. Finally!

Now, go to the setup page of the display, choose 'Hollandse Luchten (Sodaq) as sensor and fill 38617 (fill in your number) as sensor number.

Press the 'Save' button on the setup page and do a Reset also. Now the current PM<sub>2.5</sub> value of the specified sensor should appear.

#### Sensor community (server)

Using a sensor community (Luftdaten) sensor is easy. Just select `'Sensor Community (server)'` from the list of sensors and fill in the sensor number. Every Sensor Community sensor has a 5-digit number that can be found in multiple ways, for instance using the map [`https://sensor.community`](https://sensor.community). Search for your sensor on this map and click on it. Now the number appears, together with the current PM value. This number must be entered in the corresponding field.

#### Sensor community (local)

It is also possible to retrieve the data from your own sensor directly, if it is part of the same WiFi network as the display is connected to. In this case select as sensor 'Sensor community (local)' and fill in the sensor number in the corresponding field.

#### Scapeler

In future versions, the Scapeler sensors will be supported. They are working on a new version of their API. If that is finished I will update the firmware to support these sensors too.

#### Open Meteo data (wind, temperature)

In the 'Setup sensor' screen, select 'Open Meteo'. Now entering the lat/long coordinates of your desired location is important. The most easy way is to go to [`Google Maps`](https://maps.google.com) and go to the desired location, left-mousbutton press reveals the lat/long coordinates. For example '52.38150558737995, 4.635996278654743' for the center of Haarlem (Grote Markt). Selecting the coordinates from the menu copies them. You can paste the values in the lat/long fields in the setup page.

After 'Save' and 'Reset', the display should show the current wind force, temperature, direction and gusts of wind. Also the wind force unit can be chosen between Bft, km/h, m/s or Knots.
