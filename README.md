# ESP32 Weather Station

This project display various data on a 64x32 LED Hub75 display.

The ESP32 updates displayed time and sleeps. On various intervals it also updates weather, pollution, and covid data.

## Forked
This project was thrown together from [paulgreg's E-Ink project](https://github.com/paulgreg/esp32-weather-station), code from this [HackerBoxes instructable](https://www.instructables.com/HackerBox-0065-Realtime/) where I also got the misc parts from, and other various places mentioned in the code itself.

## Updates

### To Do

* add air pollution, pm2.5 (and/or pm10?) for smoke season https://openweathermap.org/api/air-pollution
* add covid data (see parameters.h)
* better differentiate get or parse data failures? But not sure if I would do anything different with different failures...
* save data to SD card?
* sleep during display intervals? or keep arduino on?
* move led display code to display-led.h

### To Fix

* Fix out of memory httpclient.getstring() error. Or just keep letting it retrieve it twice?
  * [D][HTTPClient.cpp:947] getString(): not enough memory to reserve a string! need: 17987
  * Parsing input failed!
  * [D][HTTPClient.cpp:378] disconnect(): still data in buffer (5368), clean up.

## Arduino env

Using `DOIT ESP32 DEVKIT V1` package name from: https://dl.espressif.com/dl/package_esp32_index.json with Arduino IDE.

Copy `parameters.h.dist` to `parameters.h` and update it with your wifi settings and update the URL (you need to change lat/lon and set your OpenWeatherMap API token).

## Data APIs

  * Weather: https://openweathermap.org/api/one-call-api
    * Free Developer account required to obtain API Key
  * Pollution: http://api.openweathermap.org/data/2.5/air_pollution
    * Free Developer account required to obtain API Key
  * Covid: Undecided

## Icons

Icons are [official icons from OpenWeatherMap](https://openweathermap.org/weather-conditions#How-to-get-icon-URL).

To be able to open them correctly, [paulgreg](https://github.com/paulgreg/esp32-weather-station) converted them using [LCD Assistant](http://en.radzio.dxp.pl/bitmap_converter/) with byte orientation set to horizontal and big endian settings.
It can be opened via wine (by adding 32 bits architecture).

## Fonts

Fonts are from [Adafruit-GFX-Library-fontconvert](https://github.com/paulgreg/Adafruit-GFX-Library-fontconvert) project.

## HTTP request & JSON parsing

  * https://randomnerdtutorials.com/esp32-http-get-post-arduino/

## HTTPS request

  * https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/
