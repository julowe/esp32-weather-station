# ESP32 Weather Station

That project is a weather station (getting data from openweathermap.org) displayed on a 64x32 LED hub75 display.

The ESP32 is wakes up, fetches weather data, updates the screen and sleeps for one hour.
Refresh is stopped from around midnight to 6 a.m.

## Forked
This project was thrown together from [paulgreg's E-Ink project](https://github.com/paulgreg/esp32-weather-station) and code from this [HackerBoxes isntructable](https://www.instructables.com/HackerBox-0065-Realtime/) where I got the misc parts from.

## Updates

### To Do

* add rain chance - `pop` field (probability of precipitation)
* add Hourly+1 ?
* add air pollution, pm2.5 (and/or pm10?) for smoke season https://openweathermap.org/api/air-pollution
* add covid
  * ALL days... no data logged on sundays https://api.covidactnow.org/v2/cbsa/{cbsa}.timeseries.json?apiKey={key}
  * can specify dates, smaller returned data, but need to do more math, and less documentation. also no sunday data for my sub-region https://api.covid19tracking.narrativa.com/api/country/{country-name}/region/{region-name}/sub_region/{subregion-name}?date_from=2021-04-18&date_to=2021-04-20


### To Fix
RTC module doesn't seem to always hold time or to update with new code upload.
Don't care too much, just do NTP. Battery supplies full voltage to chip pins.

## Arduino env

Using `DOIT ESP32 DEVKIT V1` package name from: https://dl.espressif.com/dl/package_esp32_index.json with Arduino IDE.

Copy `parameters.h.dist` to `parameters.h` and update it with your wifi settings and update the URL (you need to change lat/lon and set your OpenWeatherMap API token).

## Weather API

  * https://openweathermap.org/api/one-call-api
  * create an account and an API KEY

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
