// uses ESP32 DevKit v1 board, and `DOIT ESP32 DEVKIT V1` package name from: https://dl.espressif.com/dl/package_esp32_index.json

//TODO
//TODO add Hourly+1 ?
//todo add air pollution, pm2.5 (or10?) for smoke season https://openweathermap.org/api/air-pollution
//todo add covid (see parameters.h)
//todo add rain chance - `pop` field (probability of precipitation)
//todo use NTP?

//Data retrieval stuff
#include <Time.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h> //QUESTION: any benefit to using ArduinoJson library here instead?
JSONVar json;

#include "parameters.h"
#include "weather.h"
//#include "display.h" //todo - use weather icons
#include "network.h"



const uint64_t SECOND = 1000;
const uint64_t MINUTE = 60 * SECOND;
const uint64_t HOUR = 60 * MINUTE;
const uint64_t MICRO_SEC_TO_MILLI_SEC_FACTOR = 1000;
uint64_t sleepTime = MINUTE;

//Display stuff
#include "RTClib.h"
#include <MatrixHardware_ESP32_V0.h>
#include <SmartMatrix.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 64;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer1, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer2, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer3, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

RTC_DS1307 rtc;
const int defaultBrightness = (35*255)/100;     // dim: 35% brightness
//TODO use time lib instead of below?
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char monthsOfTheYr[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JLY", "AUG", "SPT", "OCT", "NOV", "DEC"};

//debug/admin stuff
const bool debug = true;
const bool debugSerial = true;
//const int weatherUpdateInterval = 15;
const int weatherUpdateInterval = 5; //used as minutes
const int covidUpdateInterval = 4; //used as hours


void setup() {
  Serial.begin(115200);
  delay(1000);

  if (debug) {
    Serial.println("Debug Mode: ON");
  }
  if (debugSerial) {
    Serial.println("Serial Debug Mode: ON");
  }

  if (debug) {
    Serial.print("Sleeptime set to: ");
    Serial.println(sleepTime);
  }


  //todo remove below, orig eink code
//  display.init(115200);
  // *** special handling for Waveshare ESP32 Driver board *** //
//  SPI.end(); // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  //SPI: void begin(int8_t sck=-1, int8_t miso=-1, int8_t mosi=-1, int8_t ss=-1);
//  SPI.begin(13, 12, 14, 15); // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
  // *** end of special handling for Waveshare ESP32 Driver board *** //
// /remove

  print_wakeup_reason();

//  display.setRotation(1);


  Serial.println("DS1307RTC Test");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // setup matrix
  matrix.addLayer(&indexedLayer1);
  matrix.addLayer(&indexedLayer2);
  matrix.addLayer(&indexedLayer3);
  matrix.begin();

  matrix.setBrightness(defaultBrightness);
}

void loop() {
  if (debugSerial) {
    Serial.println("---- Starting loop() ----");
  }

  /*TODO
   * update time on display every minute,
   * check weather every 15(?) minutes,
   * check covid every 4(?) hours
   *
   * disconnect wifi between weather check intervals? or keep on?
   *
   * sleep during display intervals? or keep on?
   */


  DateTime now = rtc.now();

  if (now.minute() % weatherUpdateInterval == 0){ // update every  mins
//  if (1 == 1) {
    Serial.print("Updating weather because it is a ");
    Serial.print(weatherUpdateInterval);
    Serial.print(" minute mark, time is: ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.println(now.minute());

    //TODO add checkWifi function to network.h
    //QUESTION connect once & check, or always connect and then disconnect as done now?
    if (!connectToWifi()) {
  //    displayError("Error : WIFI");
        if (debugSerial) {
          Serial.println("ERROR: Failed to join (or find?) WiFi");
        }
    } else {
      unsigned int retries = 5;
      boolean jsonParsed = false;

      while(!jsonParsed && (retries-- > 0)) {
        delay(1000);
        jsonParsed = getJSON(URL);

        /* TODO, maybe, fix error:
         *  [D][HTTPClient.cpp:947] getString(): not enough memory to reserve a string! need: 17926
         *  Parsing input failed!
         *  [D][HTTPClient.cpp:378] disconnect(): still data in buffer (5368), clean up.
         */
      }

      if (!jsonParsed) {
  //      displayError("Error : JSON");
        if (debugSerial) {
          Serial.println("ERROR: Parsing JSON weather date failed");
        }
      } else {
        Weather weather;
        fillWeatherFromJson(&weather); //weather.h
  //      displayWeather(&weather); //display.h

        //only for debugging, print to serial
        if (debugSerial) {
          displayWeatherDebug(&weather);

          Serial.print("Hourly PoP: ");
          Serial.println(json["hourly"][1]["pop"]);
        }

        //if (weather.updated[0] == '0' && weather.updated[1] == '0') sleepTime = HOUR * 6; // sleep for the night
      }

      disconnectFromWifi(); //QUESTION: If on wall power do we even want to disconnect? or sleep at all? sleep for heat?
    }
  } else {
    if (debugSerial) {
      Serial.print("NOT updating weather because it is NOT ");
      Serial.print(weatherUpdateInterval);
      Serial.print(" minute mark, minute = ");
      Serial.println(now.minute());
    }
  }


  if (1 == 2){ //check if time to update covid data
//  if (now.hours() % covidUpdateInterval == 0 && now.minute() == 0){ // update every X hours
    /*TODO
     * check wifi, connect if not
     * get json data
     * update covid data struct
     * print new data if in serial debug mode
     */
  }

  //TODO update display with actual data
  char txtBuffer[12];

  // clear screen before writing new text
  indexedLayer1.fillScreen(0);
  indexedLayer2.fillScreen(0);
  indexedLayer3.fillScreen(0);

  sprintf(txtBuffer, "%02d:%02d", now.hour(), now.minute());
  indexedLayer1.setFont(font3x5);
  indexedLayer1.setIndexedColor(1,{0x00, 0x00, 0xff});
  indexedLayer1.drawString(0, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();
  indexedLayer2.setFont(font8x13);
  indexedLayer2.setIndexedColor(1,{0x00, 0xff, 0x00});
  indexedLayer2.drawString(0, 11, 1, daysOfTheWeek[now.dayOfTheWeek()]);
  indexedLayer2.swapBuffers();
  sprintf(txtBuffer, "%02d %s %04d", now.day(), monthsOfTheYr[(now.month()-1)], now.year());
  indexedLayer3.setFont(font5x7);
  indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
  indexedLayer3.drawString(0, 25, 1, txtBuffer);
  indexedLayer3.swapBuffers();

  //delay updates/actions
  if (! debug) {
    sleep(sleepTime);
    Serial.println("After sleep, that line should never be printed"); //"this" line?
    delay(sleepTime);
  } else {
    delay(MINUTE);
  }
}

void sleep(uint64_t sleepTime) {
  Serial.flush();
  if (! debug) {
    //display.powerOff();
    esp_sleep_enable_timer_wakeup((uint64_t) sleepTime * MICRO_SEC_TO_MILLI_SEC_FACTOR);
    esp_deep_sleep_start();
  }

  delay(sleepTime/60);
}

void displayWeatherDebug(Weather* weather) {
    Serial.print("Hourly Feels like: ");
    Serial.println(weather->feelsLikeH1); //prints ⸮ instead of degree sign \b0
    Serial.print("Hourly High Temp: ");
    Serial.println(weather->tempH1);
    Serial.print("Hourly Huidity: ");
    Serial.println(weather->humidityH1);
    Serial.print("Today Min Temp: ");
    Serial.println(weather->tempMinD);
    Serial.print("Today Max Temp: ");
    Serial.println(weather->tempMaxD);
    Serial.print("Today Humidity: ");
    Serial.println(weather->humidityD);
    Serial.print("Tomorrow Min Temp: ");
    Serial.println(weather->tempMinD1);
    Serial.print("Tomorrow Max Temp: ");
    Serial.println(weather->tempMaxD1);
    Serial.print("Tomorrow Humidity: ");
    Serial.println(weather->humidityD1);
    Serial.print("Updated at: ");
    Serial.println(weather->updated);
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : {
      Serial.print("Wakeup caused by touchpad pin ");
      Serial.println(esp_sleep_get_touchpad_wakeup_status());
      break;
    }

    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}
