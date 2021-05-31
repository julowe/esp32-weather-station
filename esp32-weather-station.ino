// Display things on a Hub75 display with ESP32
// uses ESP32 DevKit v1 board, and `DOIT ESP32 DEVKIT V1` package name from: https://dl.espressif.com/dl/package_esp32_index.json
// 2021-04-25 jkl

/* includes much code from: 
 *  HackerBox 0065 Clock Demo for 64x32 LED Array, https://www.instructables.com/HackerBox-0065-Realtime/
 *  Adapted from SmartMatrix example file
 *  in boolean RTC_DS1307::begin(void):
 *  Change Wire.begin() to Wire.begin(14, 13)
 *  
 *  NTP from : https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
*/

/*
 * forked from: 
 * https://github.com/paulgreg/esp32-weather-station
*/

//TODO
//todo add covid data (see parameters.h)
//TODO better differentiate get or parse data failures? But not sure if I would do anything different with different failures...
//TODO save data to SD card?
//TODO sleep during display intervals? or keep arduino on?
//TODO move led display code to display-led.h
//TODO hourly temp doesnt give much more info. use daily-timeframe temps instead?
//TODO buttons to display diff data - random trello card(1), tomorrow weather. covid? pollution?
//TODO button for new random trello card - first display random card scroll text, then usual first, random text (so less waiting for new data to display)
/*TODO 
 * x- display pollution and allergy levels as 15x1-2 pixel bars at bottom of screen. 
 * x- red yellow green bars as according to data/apis. 
 * - a physical button would then pop up a 15 x 8 pixel box all same color as data color with black text to show number, disappear after.... 30 seconds?
 */


/* TODO hmm well crap OpenWeatherMap is often wrong in the ~2 days of on and (mostly) off testing I've done. No it is not cloudy and raining, its sunny...
 *  
 *  so, other sources?
 *  https://www.reddit.com/r/webdev/comments/8tjavu/now_that_the_free_wunderground_api_has_been/ which provides this ranking page:
 *  https://www.forecastadvisor.com/
 *  Here are results for my area:
 *  
Last Month (April) 
The Weather Channel     91.67% - ibm call for code - works unti nov 2021 :-/  api key in my param file
Weather Underground     91.11% - not free - maybe this key works: https://github.com/katzwebservices/Wunderground/blob/master/wunderground.php
Foreca/Vaisala    88.06% - free for 12 months(?) https://developer.foreca.com/trial
AccuWeather     87.50%  - Free api, 50 calls/day limit https://developer.accuweather.com/packages
AerisWeather    86.39%
NWS Digital Forecast    83.06% - assume free as gov, https://www.weather.gov/documentation/services-web-api
World Weather Online    78.61%
Wetter    77.50%
OpenWeather     76.11%
Persistence     60.00%
Weather News    57.78%
 *  
 */

   
//Data retrieval stuff
#include <Time.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h> //QUESTION: any benefit to using ArduinoJson library here instead?
//#include "cjson/cJSON.h"

JSONVar jsonResult;

#include "parameters.h"

//#include "display.h" //TODO - use weather icons from this file
//#include "display-led.h"
#include "network.h"

const uint64_t SECOND = 1000;
const uint64_t MINUTE = 60 * SECOND;
const uint64_t HOUR = 60 * MINUTE;
const uint64_t MICRO_SEC_TO_MILLI_SEC_FACTOR = 1000;
uint64_t sleepTime = MINUTE;


//weather stuff
#include "weather.h"
//declare struct for weather data to be put in
Weather weather_data;


//pollution stuff
#include "pollution.h";
Pollution pollution_data;
int barWidthPollution;


//trello stuff
#include "trello.h";
char trelloCardNameFirst[30];
char trelloCardNameRandom[30];//ugh, ugly hack


//LED Matrix & RTC stuff
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
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer1, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer2, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer3, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayerAQI, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions); //color rectangles for pollution (over scrolling text)
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayerPM25, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions); //color rectangles for pollution (over scrolling text)
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayerPM10, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions); //color rectangles for pollution (over scrolling text)

SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer5, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions); //layer for pollution numbers


RTC_DS1307 rtc;
const int defaultBrightness = (35*255)/100;     // dim: 35% brightness
//TODO use time lib instead of below?
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char monthsOfTheYr[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//NTP stuff
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8*3600;
//const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;
struct tm ntpTimeInfo;

//debug/admin stuff
const bool debug = true;
const bool debugSerial = true;
bool updateRTC = false; //don't really need this until we stop using RTC and don't just end the sketch if RTC not working

//TODO fix this/make it work another way
//#if debug == 1

  //only 1 get data attempt will likely always fail, so every other loop() cycle will fail because of httpclient.getstring() memory error
  boolean getDataWrapper(String data_source = "weather", int connectWifiTries = 1, int getDataTries = 1); 
  const int weatherUpdateInterval = 1; //used as minutes
  const int covidUpdateInterval = 1; //used as hours
  
//#else

//  boolean getDataWrapper(String data_source = "weather", int connectWifiTries = 5, int getDataTries = 5);
//  const int weatherUpdateInterval = 15; //used as minutes
//  const int covidUpdateInterval = 4; //used as hours

//#endif
  
const int connectWifiDelay = 5000; //5 seconds for now
const int getDataDelay = 2000; //2 seconds for now


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

  print_wakeup_reason();

  // setup matrix
  matrix.addLayer(&indexedLayer1);
  matrix.addLayer(&indexedLayer2);
  matrix.addLayer(&indexedLayer3);
  matrix.addLayer(&scrollingLayer);
  matrix.addLayer(&indexedLayerAQI);
  matrix.addLayer(&indexedLayerPM25);
  matrix.addLayer(&indexedLayerPM10);
  matrix.addLayer(&indexedLayer5);
  
  matrix.begin();
  
  matrix.setBrightness(defaultBrightness);

  //Get time message
  // clear screen before writing new text
  indexedLayer1.fillScreen(0);

  indexedLayer1.setFont(font3x5);
  indexedLayer1.setIndexedColor(1,{0x00, 0x00, 0xff});
  indexedLayer1.drawString(0, 0, 1, "Checking Time...");
  indexedLayer1.swapBuffers();

  //Weather Intialize message
  indexedLayer3.fillScreen(0);
  
  indexedLayer3.setFont(font3x5);
  indexedLayer3.setIndexedColor(1,{0x00, 0x00, 0xff});
  indexedLayer3.drawString(0, 16, 1, "Getting Weather...");
  indexedLayer3.swapBuffers();


  //Pollution Intialize message
  indexedLayer5.fillScreen(0);
  
  indexedLayer5.setFont(font3x5);
  indexedLayer5.setIndexedColor(1,{0x00, 0x00, 0xff});
  indexedLayer5.drawString(0, 25, 1, "Getting Air Qual...");
  indexedLayer5.swapBuffers();

  
  Serial.println("DS1307RTC Test");
  if (rtc.begin()) {
    updateRTC = true;
  } else {
    Serial.println("ERROR: Couldn't find RTC");
    //updateRTC == false; //false is default
    Serial.flush();
    abort();
  }


  //set up NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (rtc.isrunning()) {
    if (debugSerial) {
      Serial.println("RTC IS running, let's check the time!");
    }
    //rtc is running, but check time on start up
    bool timesMatched = verifyTime();
  } else {
    if (debugSerial) {
      Serial.println("RTC is NOT running, let's set the time!");
    }
    
    if ( getNTP() ) {
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));    
      updateRTCtoNTP();
    } else {
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }

  displayClock();


  //get weather data at startup
  bool dataWrapperSuccess = getDataWrapper("weather", 5, 3); //try connecting to wifi 5 times, getting data thrice

  if (debugSerial && dataWrapperSuccess) {
    Serial.println("Succesfully got weather data");
  } else if (debugSerial && !dataWrapperSuccess) {
    Serial.println("ERROR: Did not retrieve weather data.");
  }

 
  if (dataWrapperSuccess) {
    dataWrapperSuccess = parseDataWrapper("weather");
  }
  if (debugSerial && dataWrapperSuccess) {
    Serial.println("Succesfully parsed weather data");
    printWeatherDebug(&weather_data);
  } else if (debugSerial && !dataWrapperSuccess) {
    Serial.println("ERROR: Did not parse weather data.");
  }
  

  //Display Weather Data
  if (dataWrapperSuccess) {
    displayWeather(&weather_data);
  } else {
    indexedLayer2.fillScreen(0);
    indexedLayer3.fillScreen(0);
  
    indexedLayer3.setFont(font3x5);
    indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
    indexedLayer3.drawString(0, 13, 1, "Failed to Initialize");
    indexedLayer3.swapBuffers();
  }


  //get Pollution data at startup
  bool dataWrapperPollutionSuccess = getDataWrapper("pollution", 5, 3); //try connecting to wifi 5 times, getting data thrice
  if (debugSerial && dataWrapperPollutionSuccess) {
    Serial.println("Succesfully got pollution data");
  } else if (debugSerial && !dataWrapperPollutionSuccess) {
    Serial.println("ERROR: Did not retrieve pollution data.");
  }

 
  if (dataWrapperPollutionSuccess) {
    dataWrapperPollutionSuccess = parseDataWrapper("pollution");
  }
  if (debugSerial && dataWrapperPollutionSuccess) {
    Serial.println("Succesfully parsed pollution data");
      printPollutionDebug(&pollution_data);
  } else if (debugSerial && !dataWrapperPollutionSuccess) {
    Serial.println("ERROR: Did not parse pollution data.");
  }

  //Display pollution Data
  if (dataWrapperPollutionSuccess) {
    barWidthPollution = displayPollution(&pollution_data, true);
    if (debugSerial) {
      Serial.print("Pollution bars width = ");
      Serial.println(barWidthPollution);
    }
  } else {
    //maybe don't display error here, as then logic is complicated for erasing or keeping after trello data gotten
//    indexedLayer5.fillScreen(0);
//    
//    indexedLayer5.setFont(font3x5);
//    indexedLayer5.setIndexedColor(1,{0x00, 0x00, 0xff});
//    indexedLayer5.drawString(0, 25, 1, "Failed to get pollution");
//    indexedLayer5.swapBuffers();
  }


  //get Trello data at startup
  bool dataWrapperTrelloSuccess = getDataWrapper("trello_cards", 5, 3); //try connecting to wifi 5 times, getting data thrice
  if (debugSerial && dataWrapperTrelloSuccess) {
    Serial.println("Succesfully got trello data");
  } else if (debugSerial && !dataWrapperTrelloSuccess) {
    Serial.println("ERROR: Did not retrieve trello data.");
  }

 
  if (dataWrapperTrelloSuccess) {
    dataWrapperTrelloSuccess = parseDataWrapper("trello_cards");
  }
  if (debugSerial && dataWrapperTrelloSuccess) {
    Serial.println("Succesfully parsed trello_cards data");
  } else if (debugSerial && !dataWrapperTrelloSuccess) {
    Serial.println("ERROR: Did not parse trello_cards data.");
  }

  //Display Trello Data
  if (dataWrapperTrelloSuccess) {
    displayTrelloCards();
  }

  //wait 20 seconds
  delay(20*1000);
  //then "minimize" pollution data
  if (debugSerial) {
    Serial.println("DEBUG: Minimizing pollution bars");
  }

  if ( dataWrapperPollutionSuccess ) {
    barWidthPollution = displayPollution(&pollution_data, false);
  }
  
//  disconnectFromWifi(); //TODO do we want to explicitly disconnect from wifi between updates for any reason? chip heat savings?
}


void loop() {
  bool dataWrapperSuccess = false;
  if (debugSerial) {
    Serial.println("---- Starting loop() ----");
  }


  DateTime now = rtc.now();
  
  if (now.minute() % weatherUpdateInterval == 0){ // update every `weatherUpdateInterval`-th mins
    if (debugSerial) {
      Serial.print("Debug: Updating weather because it is a ");
      Serial.print(weatherUpdateInterval);
      Serial.print(" minute mark, time is: ");
      Serial.print(now.hour());
      Serial.print(":");
      print2digitsLn(now.minute());
//      Serial.println(now.minute());
    }

    //get weather data (default function value)
    dataWrapperSuccess = getDataWrapper();
    
    if (debugSerial && dataWrapperSuccess) {
      Serial.println("Debug: Succesfully got weather data");
    } else if (debugSerial && !dataWrapperSuccess) {
      Serial.println("ERROR: Did not retrieve weather data.");
    }

 
    if (dataWrapperSuccess) {
      dataWrapperSuccess = parseDataWrapper("weather");
    }
    if (debugSerial && dataWrapperSuccess) {
      Serial.println("Succesfully parsed weather data");
      printWeatherDebug(&weather_data);
    } else if (debugSerial && !dataWrapperSuccess) {
      Serial.println("ERROR: Did not parse weather data.");
    }
    
    //Display Data
    if (dataWrapperSuccess) {
      //TODO make 'last updated' it's own layer
      displayWeather(&weather_data);
    } else {
      if (debugSerial) {
        Serial.println("Debug: Updating matrix with failure message");
      }
      //TODO display anything different here? or just leave old data up? update 'last updated' layer only?

      indexedLayer2.fillScreen(0);
      indexedLayer2.swapBuffers();
      indexedLayer3.fillScreen(0);
    
      indexedLayer3.setFont(font3x5);
      indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
      indexedLayer3.drawString(0, 13, 1, "Failed to Update");
      indexedLayer3.swapBuffers();
    }

    //update time - later maybe break out to own interval?
    bool timesMatched = verifyTime();


    //get Pollution data
    bool dataWrapperPollutionSuccess = getDataWrapper("pollution");
    if (debugSerial && dataWrapperPollutionSuccess) {
      Serial.println("Succesfully got pollution data");
    } else if (debugSerial && !dataWrapperPollutionSuccess) {
      Serial.println("ERROR: Did not retrieve pollution data.");
    }

 
    if (dataWrapperPollutionSuccess) {
      dataWrapperPollutionSuccess = parseDataWrapper("pollution");
    }
    if (debugSerial && dataWrapperPollutionSuccess) {
      Serial.println("Succesfully parsed pollution data");
        printPollutionDebug(&pollution_data);
    } else if (debugSerial && !dataWrapperPollutionSuccess) {
      Serial.println("ERROR: Did not parse pollution data.");
    }
  
    //Display pollution Data
    if (dataWrapperPollutionSuccess) {
      barWidthPollution = displayPollution(&pollution_data, false);
    }

    //get trello data
    bool dataWrapperTrelloSuccess = getDataWrapper("trello_cards");
    if (debugSerial && dataWrapperTrelloSuccess) {
      Serial.println("Debug: Succesfully got trello_cards data");
    } else if (debugSerial && !dataWrapperTrelloSuccess) {
      Serial.println("ERROR: Did not retrieve trello_cards data.");
    }

 
    if (dataWrapperTrelloSuccess) {
      dataWrapperTrelloSuccess = parseDataWrapper("trello_cards");
    }
    if (debugSerial && dataWrapperTrelloSuccess) {
      Serial.println("Succesfully parsed trello_cards data");
    } else if (debugSerial && !dataWrapperTrelloSuccess) {
      Serial.println("ERROR: Did not parse trello_cards data.");
    }
  
    //Display Trello Data
    if (dataWrapperTrelloSuccess) {
      displayTrelloCards();
    }
  
  
  } else {
    if (debugSerial) {
      Serial.print("Debug: NOT updating weather because it is NOT ");
      Serial.print(weatherUpdateInterval);
      Serial.print(" minute mark, minute = ");
      print2digitsLn(now.minute());
//      Serial.println(now.minute());
    }
  }


  if (now.hour() % covidUpdateInterval == 0 && now.minute() == 0){ // update every Xth hours
//  if (now.hour() % covidUpdateInterval == 0){ // update every Xth hours and any minute - for debugging!
    if (debugSerial) {
      Serial.print("Debug: Updating covid data because it is a ");
      Serial.print(covidUpdateInterval);
      Serial.print(" hour mark, time is: ");
      Serial.print(now.hour());
      Serial.print(":");
      print2digitsLn(now.minute());
//      Serial.println(now.minute());
    }
    
    /*TODO
     * get json data
     * update covid data struct
     * print new data if in serial debug mode
     */
    bool dataWrapperCovidSuccess = getDataWrapper("covid");
    
    if (debugSerial && dataWrapperCovidSuccess) {
      Serial.println("Debug: Succesfully got covid data");
    } else if (debugSerial && !dataWrapperCovidSuccess) {
      Serial.println("ERROR: Did not retrieve covid data.");
    }

 
    if (dataWrapperCovidSuccess) {
//      dataWrapperCovidSuccess = parseDataWrapper("covid"); //TODO make this
      dataWrapperCovidSuccess = false; //make false until make parsing function
    }
    if (debugSerial && dataWrapperCovidSuccess) {
      Serial.println("Succesfully parsed covid data");
//      printCovidDebug(&covid_data); //TODO make covid data display debug
    } else if (debugSerial && !dataWrapperCovidSuccess) {
      Serial.println("ERROR: Did not parse covid data.");
    }      
    
    //Display Data
    if (dataWrapperCovidSuccess) {
      //TODO make 'last updated' it's own layer
//      displayCovidData(&covid_data); //TODO make covid data display function
    } else {
      if (debugSerial) {
//        Serial.println("Updating matrix with failure message");
        Serial.println("Debug: Not changing led display as displaying covid data is a work in progress.");
      }
      //TODO display anything different here? or just leave old data up? update 'last updated' layer only?
      
//      indexedLayer2.fillScreen(0);
//      indexedLayer3.fillScreen(0);
//    
//      indexedLayer3.setFont(font3x5);
//      indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
//      indexedLayer3.drawString(0, 25, 1, "Failed to Update");
//      indexedLayer3.swapBuffers();
    }
     
  } else {
    if (debugSerial) {
      Serial.print("Debug: NOT updating covid data because it is NOT ");
      Serial.print(covidUpdateInterval);
      Serial.print(" hour mark, time is: ");
      Serial.print(now.hour());
      Serial.print(":");
      print2digitsLn(now.minute());
//      Serial.println(now.minute());
    }
  }

  displayClock();

  //delay updates/actions
  if (! debug) {
    sleep(sleepTime);
    Serial.println("After sleep, that line should never be printed"); //"this" line?
    delay(sleepTime);
  } else {
    if (debugSerial) {
      Serial.print("Debug: Sleeping for ");
      Serial.print(MINUTE/1000);
      Serial.println(" seconds.");
    }
    delay(MINUTE);
  }
}


void sleep(uint64_t sleepTime) {
  if (debugSerial) {
    Serial.print("Debug: Sleeping for ");
    Serial.print(sleepTime/1000);
    Serial.println(" seconds.");
  }
  
  Serial.flush();
  if (! debug) {
    //display.powerOff();
    esp_sleep_enable_timer_wakeup((uint64_t) sleepTime * MICRO_SEC_TO_MILLI_SEC_FACTOR);
    esp_deep_sleep_start();
  }

  delay(sleepTime/60);
}


/*************************
 *                       *
 *    Display Functions  *
 *                       *
 *************************/

 
void displayClock() {
  DateTime now = rtc.now();
  char txtBuffer[20];

  // clear screen before writing new text
  indexedLayer1.fillScreen(0);

  indexedLayer1.setFont(font5x7);
  indexedLayer1.setIndexedColor(1,{0x00, 0x00, 0xff});
  
  //Weekday
  sprintf(txtBuffer, "%s", daysOfTheWeek[now.dayOfTheWeek()]);
  indexedLayer1.drawString(0, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();

  //Day date
  sprintf(txtBuffer, "%02d", now.day());
  indexedLayer1.drawString(16, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();

  //month
  sprintf(txtBuffer, "%s", monthsOfTheYr[(now.month()-1)]);
  indexedLayer1.drawString(27, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();

  //time
  //nicely align time
  int nowHour = now.hour();
  if ( nowHour > 12 ) {
    nowHour -= 12;
  }
  int timePos1;
  int timePos2;
  int timePos3;
  if ( nowHour < 10 ) {
    timePos1 = 44;
    timePos2 = 48;
    timePos3 = 52;
  } else {
    timePos1 = 42;
    timePos2 = 51;
    timePos3 = 55;    
  }
  
  //display time
  sprintf(txtBuffer, "%d", nowHour);
  indexedLayer1.drawString(timePos1, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();
  
  indexedLayer1.drawString(timePos2, 0, 1, ":");
  indexedLayer1.swapBuffers();
  
  sprintf(txtBuffer, "%02d", now.minute());
  indexedLayer1.drawString(timePos3, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();

  //TODO AM/PM indicator? 24h? meh?
}


void displayWeather(Weather* weather_data) {
  /* Weather Display Ideas
   *  
   *  Humidity bar graph on right side?
   *  
   *  temp now, temp 1 hour out (or 2?), rain chance
   *  
   *  word weather description
   */
  char txtBuffer[20];
  //weather description in words
  indexedLayer2.fillScreen(0);

  indexedLayer2.setFont(font5x7);
  indexedLayer2.setIndexedColor(1,{0x00, 0xff, 0x00});
  indexedLayer2.drawString(0, 8, 1, weather_data->descriptionC);
  indexedLayer2.swapBuffers();

  //weather in numbers
  indexedLayer3.fillScreen(0);
  int pop0Temp = atof(weather_data->popH0)*100;
//  int pop1Temp = atof(weather_data->popH1)*100;
//  sprintf(txtBuffer, "%s %s", weather_data->tempC, weather_data->tempH1);
  sprintf(txtBuffer, "%s %s %i%%", weather_data->tempC, weather_data->tempH1, pop1Temp);
//  sprintf(txtBuffer, "%s %s %3i%% %3i%%", weather_data->tempC, weather_data->tempH1, atof(weather_data->popH1)*100, atof(weather_data->popH4)*100);
  indexedLayer3.setFont(font5x7);
  indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
  indexedLayer3.drawString(0, 16, 1, txtBuffer);
  indexedLayer3.swapBuffers();
}


int displayPollution(Pollution* pollution_data, bool showValue) {
  int bars_width = 0;

  indexedLayerAQI.fillScreen(0);
  indexedLayerPM25.fillScreen(0);
  indexedLayerPM10.fillScreen(0);
  indexedLayer5.fillScreen(0);
  indexedLayer5.setIndexedColor(1,{0, 0, 0});

  // Air Quality Index. Possible values: 1, 2, 3, 4, 5. Where 1 = Good, 2 = Fair, 3 = Moderate, 4 = Poor, 5 = Very Poor.
  //rgb24 is of course in {red, blue, green} order
  if ( atoi(pollution_data->aqi) == 1 ) {
    if (debugSerial) {
      Serial.println("Debug: AQI good");
    }
    indexedLayerAQI.setIndexedColor(1, {0, 0, 0xff}); //green
  } else if ( atoi(pollution_data->aqi) == 2 ) {
    if (debugSerial) {
      Serial.println("Debug: AQI fair");
    }
    indexedLayerAQI.setIndexedColor(1, {0, 0, 0x80}); //dull green
  } else if ( atoi(pollution_data->aqi) == 3 ) {
    if (debugSerial) {
      Serial.println("Debug: AQI moderate");
    }
    indexedLayerAQI.setIndexedColor(1, {0xff, 0x66, 0xff}); //yellow
  } else if ( atoi(pollution_data->aqi) == 4 ) {
    if (debugSerial) {
      Serial.println("Debug: AQI poor");
    }
    indexedLayerAQI.setIndexedColor(1, {0x80, 0, 0}); //dull red   
  } else {
    if (debugSerial) {
      Serial.println("Debug: AQI very poor");
    }
    indexedLayerAQI.setIndexedColor(1, {0xff, 0, 0}); //bright red
  }
  
//  int xStartAQI = 0;
  int xStartAQI = 1; //don't start at edge of screen, 1 or 2 pixel padding?
  int xWidthAQI = 10; //default to just small color bar
  bars_width = xStartAQI + xWidthAQI; //AQI is always a one digit number, so bar width does not change
  int yStartAQI = 30; //default to just small color bar
  
  if ( showValue ) {
    yStartAQI = 24;
  } 
//  else {
//    int yStart = 30;
//  }

  for ( int x = xStartAQI; x < xStartAQI+xWidthAQI; x++ ) {
    for ( int y = yStartAQI; y < 32; y++ ) {
      indexedLayerAQI.drawPixel(x, y, 1);
    }
  }
  indexedLayerAQI.swapBuffers();

  if ( showValue ) {
    indexedLayer5.setFont(font5x7);
    indexedLayer5.drawString(xStartAQI+3, 25, 1, pollution_data->aqi);
    indexedLayer5.swapBuffers();
  }

  //pm2_5
  //good 15, moderate 40, unhealth sensitive 65, unhealthy 150, very unhealthy 250, hazardous 500 https://aqicn.org/faq/2013-09-09/revised-pm25-aqi-breakpoints/
  //or https://www.airnow.gov/sites/default/files/2020-05/aqi-technical-assistance-document-sept2018.pdf
  //good 12, moderate 35.5, unhealth sensitive 55.5, unhealthy 150.5, very unhealthy 250.5, hazardous 500.5


  //TEST TEST
//  sprintf(pollution_data->pm2_5, "%.2f",  (double) 55 );

  //rgb24 is of course in {red, blue, green} order
  if ( atoi(pollution_data->pm2_5) < 4 ) {
    if (debugSerial) {
      Serial.println("Debug: PM25 good");
    }
    indexedLayerPM25.setIndexedColor(1, {0, 0, 0xff}); //green
  } else if ( atoi(pollution_data->pm2_5) < 40 ) {
    if (debugSerial) {
      Serial.println("Debug: PM25 moderate");
    }
    indexedLayerPM25.setIndexedColor(1, {0, 0, 0x80}); //dull green
  } else if ( atoi(pollution_data->pm2_5) < 65 ) {
    if (debugSerial) {
      Serial.println("Debug: PM25 unhealthy for sensitive");
    }
    indexedLayerPM25.setIndexedColor(1, {0xff, 0x66, 0xff}); //yellow
  } else if ( atoi(pollution_data->pm2_5) < 150 ) {
    if (debugSerial) {
      Serial.println("Debug: PM25 unhealthy");
    }
    indexedLayerPM25.setIndexedColor(1, {0xff, 0x66, 0xff}); //yellow
  } else if ( atoi(pollution_data->pm2_5) < 250 ) {
    if (debugSerial) {
      Serial.println("Debug: PM25 very unhealthy");
    }
    indexedLayerPM25.setIndexedColor(1, {0x80, 0, 0}); //dull red   
  } else { //< 500
    if (debugSerial) {
      Serial.println("Debug: PM25 hazardous");
    }
    indexedLayerPM25.setIndexedColor(1, {0xff, 0, 0}); //bright red
  }
  
  int xStartPM25 = xStartAQI + xWidthAQI + 2; 
  int xWidthPM25 = 10; //default to just small color bar
  int yStartPM25 = 30; //default to just small color bar
  
  if ( showValue ) {
    yStartPM25 = 24;

    //only expand width of bar if showing value
    if ( atoi(pollution_data->pm2_5) < 10 ) {
      //don't change bar width
    } else if ( atoi(pollution_data->pm2_5) < 100 ) {
      xWidthPM25 = 15; //2*5char + 2 on each side + 1
    } else if ( atoi(pollution_data->pm2_5) < 1000 ) {
      xWidthPM25 = 21; //3*5char + 2 on each side
    } else {
      //huh?
    }
  } 
//  else {
//    int yStart = 30;
//  }
  bars_width = xStartPM25 + xWidthPM25;

  for ( int x = xStartPM25; x < xStartPM25+xWidthPM25; x++ ) {
    for ( int y = yStartPM25; y < 32; y++ ) {
      indexedLayerPM25.drawPixel(x, y, 1);
    }
  }
  indexedLayerPM25.swapBuffers();

  if ( showValue ) {
    char pm25Temp[4];
    sprintf(pm25Temp, "%i", atoi(pollution_data->pm2_5));

    indexedLayer5.setFont(font5x7);
    indexedLayer5.drawString(xStartPM25+3, 25, 1, pm25Temp);
    indexedLayer5.swapBuffers();
  }

  //pm10
  //good 54, moderate 154, unhealth sensitive 254, unhealthy 354, very unhealthy 424, hazardous 604

  //TEST TEST
//  sprintf(pollution_data->pm2_5, "%.2f",  (double) 55 );

  //rgb24 is of course in {red, blue, green} order
  if ( atoi(pollution_data->pm10) < 54 ) {
    if (debugSerial) {
      Serial.println("Debug: PM10 good");
    }
    indexedLayerPM10.setIndexedColor(1, {0, 0, 0xff}); //green
  } else if ( atoi(pollution_data->pm10) < 154 ) {
    if (debugSerial) {
      Serial.println("Debug: PM10 moderate");
    }
    indexedLayerPM10.setIndexedColor(1, {0, 0, 0x80}); //dull green
  } else if ( atoi(pollution_data->pm10) < 254 ) {
    if (debugSerial) {
      Serial.println("Debug: PM10 unhealthy for sensitive");
    }
    indexedLayerPM10.setIndexedColor(1, {0xff, 0x66, 0xff}); //yellow
  } else if ( atoi(pollution_data->pm10) < 354 ) {
    if (debugSerial) {
      Serial.println("Debug: PM10 unhealthy");
    }
    indexedLayerPM10.setIndexedColor(1, {0xff, 0x66, 0xff}); //yellow
  } else if ( atoi(pollution_data->pm10) < 424 ) {
    if (debugSerial) {
      Serial.println("Debug: PM10 very unhealthy");
    }
    indexedLayerPM10.setIndexedColor(1, {0x80, 0, 0}); //dull red   
  } else { //< 500
    if (debugSerial) {
      Serial.println("Debug: PM10 hazardous");
    }
    indexedLayerPM10.setIndexedColor(1, {0xff, 0, 0}); //bright red
  }
  
  int xStartPM10 = xStartAQI + xWidthAQI + 2 + xWidthPM25 + 2;
  int xWidthPM10 = 10; //default to just small color bar
  int yStartPM10 = 30; //default to just small color bar
  
  if ( showValue ) {
    yStartPM10 = 24;

    //only expand width of bar if showing value
    if ( atoi(pollution_data->pm10) < 10 ) {
      //don't change bar width
    } else if ( atoi(pollution_data->pm10) < 100 ) {
      xWidthPM10 = 15; //2*5char + 2 on each side + 1
    } else if ( atoi(pollution_data->pm10) < 1000 ) {
      xWidthPM10 = 21; //3*5char + 2 on each side
    } else {
      //huh?
    }
  } 
//  else {
//    int yStart = 30;
//  }
  bars_width = xStartPM10 + xWidthPM10;

  for ( int x = xStartPM10; x < xStartPM10+xWidthPM10; x++ ) {
    for ( int y = yStartPM10; y < 32; y++ ) {
      indexedLayerPM10.drawPixel(x, y, 1);
    }
  }
  indexedLayerPM10.swapBuffers();

  if ( showValue ) {
    char PM10Temp[4];
    sprintf(PM10Temp, "%i", atoi(pollution_data->pm10));

    indexedLayer5.setFont(font5x7);
    indexedLayer5.drawString(xStartPM10+3, 25, 1, PM10Temp);
    indexedLayer5.swapBuffers();
  }

  indexedLayer5.swapBuffers();

  return bars_width;
}
void displayTrelloCards() {
  
    scrollingLayer.setMode(wrapForward);
    scrollingLayer.setOffsetFromTop(23);
//    scrollingLayer.setSpeed(10);
    scrollingLayer.setSpeed(16);
    scrollingLayer.setFont(font5x7);
    scrollingLayer.setColor({0xff,0xff,0xff});
//    String message = "First: " + trelloCardNameFirst;// + "& Random: " + trelloCardNameRandom;
    char message[80];
//    strcpy(message, "First: ");
    strcpy(message, trelloCardNameFirst);
//    strcat(message, trelloCardNameFirst);
//    strcat(message, ", Random: ");
    strcat(message, ", ");
    strcat(message, trelloCardNameRandom);
//    = "First: " + trelloCardNameFirst;// + "& Random: " + trelloCardNameRandom;

  
    scrollingLayer.start(message, -1);

}

void displayTestScroll() {

    scrollingLayer.setMode(wrapForward);
    scrollingLayer.setOffsetFromTop(21);
    scrollingLayer.setSpeed(10);
    scrollingLayer.setFont(font5x7);
    scrollingLayer.setColor({0xff,0xff,0xff});
    scrollingLayer.start("Wrap message 4 times", 4);
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


void print2digitsLn(int number) {
  if (number >= 0 && number < 10) {
    Serial.print('0');
  }
  Serial.println(number);
}



/**************************
 *                        *
 *    Get Data Functions  *
 *                        *
 **************************/
 
 
boolean getDataWrapper(String data_source, int connectWifiTries, int getDataTries) {
  bool dataSuccess = false;
  int connectWifiTrialNumber = 1;
  int getDataTrialNumber = 1;
  if (debugSerial) {
    Serial.print("---- Starting getDataWrapper(");
    Serial.print(data_source);
    Serial.print(", ");
    Serial.print(connectWifiTries);
    Serial.print(", ");
    Serial.print(getDataTries);
    Serial.println(") ----");
  }

  //TODO move this logic into connectToWifi(), passing trial number
  while (connectWifiTrialNumber <= connectWifiTries && WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempt #");
    Serial.print(connectWifiTrialNumber);
    Serial.print("/");
    Serial.print(connectWifiTries);
    Serial.print(" for ");
    Serial.println("connecting to wifi...");
    
    connectToWifi();
    connectWifiTrialNumber++;
    delay(connectWifiDelay);
  }

  if (connectWifiTrialNumber == 1 && WiFi.status() == WL_CONNECTED) {
    if (debugSerial) {
      Serial.println("Debug: Already connected to wifi.");
    }
  }
  //end TODO above

  //TODO move while logic into getJSON() ?
  while (getDataTrialNumber <= getDataTries && !dataSuccess) {
    Serial.print("Attempt #");
    Serial.print(getDataTrialNumber);
    Serial.print("/");
    Serial.print(getDataTries);
    Serial.print(" for retrieving ");
    Serial.print(data_source);    
    Serial.println(" data...");
    
    //connect to wifi
    if (WiFi.status() == WL_CONNECTED) {

      if ( data_source == "weather") {
        if (debugSerial) {
          Serial.println("Debug: weather if");
        }
        //TODO split this into two get calls (hourly & daily) if memory becomes an issue
        dataSuccess = getJSON(URL_weather);
      } else if ( data_source == "pollution") {
        if (debugSerial) {
          Serial.println("Debug: pollution if");
        }
        dataSuccess = getJSON(URL_pollution);
      } else if ( data_source == "covid") {
        if (debugSerial) {
          Serial.println("Debug: covid if");
        }
        Serial.println("Covid data retrieval not yet implemented.");
//        dataSuccess = getJSON(URL_covid_base);
      } else if ( data_source == "trello_cards") {
        if (debugSerial) {
          Serial.println("Debug: trello if");
        }
        dataSuccess = getJSON(URL_trello_cards);
      } else {
        if (debugSerial) {
          Serial.println("Debug: default else");
        }
        dataSuccess = getJSON(URL_weather);
      }
      
      if (dataSuccess) {
        
      } else {
        if (debugSerial) {
          //printing retained here to report each time an attempt fails
          Serial.print("ERROR: Did not get ");
          Serial.print(data_source);
          Serial.println(" data");
        }
      } //endif dataSuccess

    } else {    
      if (debugSerial) {
        Serial.println("ERROR: Not connected to wifi, attempting to reconnect");
      }
      connectToWifi();
      
    } //endif wifi connected
    getDataTrialNumber++;
    delay(getDataDelay);
    
  } //end while

  return dataSuccess;
}


boolean parseDataWrapper(String data_source) {
  bool parseSuccess = false;
  if (debugSerial) {
    Serial.print("---- Starting parseDataWrapper(");
    Serial.print(data_source);
    Serial.println(") ----");
  }

  if ( data_source == "weather") {
    if (debugSerial) {
      Serial.println("dataSuccess weather if");
    }
    parseSuccess = fillWeatherFromJson(&weather_data); //weather.h
    jsonResult = JSON.parse("{}");
  } else if ( data_source == "pollution") {
    if (debugSerial) {
      Serial.println("Debug: dataSuccess pollution if");
    }
    parseSuccess = fillPollutionFromJson(&pollution_data); //pollution.h
    jsonResult = JSON.parse("{}");
  } else if ( data_source == "covid") {
    if (debugSerial) {
      Serial.println("Debug: dataSuccess covid if");
    }
    Serial.println("Covid data storage not yet implemented.");
//          parseSuccess = fillCovidDataFromJson(&covid_data); //weather.h
  } else if ( data_source == "trello_cards") {
    if (debugSerial) {
      Serial.println("Debug: dataSuccess trello if");
    }       
    parseSuccess = trelloFirstCard(trelloCardNameFirst);
    
    if ( parseSuccess ) {
      parseSuccess = trelloRandomCard(trelloCardNameRandom);
    }
    
    if (debugSerial) {
      Serial.print("Debug: First card name: ");
      Serial.print(trelloCardNameFirst);
      Serial.print(", Second card name: ");
      Serial.println(trelloCardNameRandom);
    }
    jsonResult = JSON.parse("{}");        
  } else {
    if (debugSerial) {
      Serial.println("Debug: dataSuccess default else");
    }
    parseSuccess = fillWeatherFromJson(&weather_data); //weather.h
    jsonResult = JSON.parse("{}");
  }

  return parseSuccess;
}


boolean getNTP(){
  bool getNTPSuccess = false;

//  bool wifiConnected = connectToWifi(); //do we need this result again?
  
  if ( connectToWifi() ) {
    if(getLocalTime(&ntpTimeInfo)){
      getNTPSuccess = true;
    } else {
      if (debugSerial) {
        Serial.println("ERROR: Failed to obtain NTP time");
      }
      return getNTPSuccess;
    }
    
    if (debugSerial) {
      Serial.print("Info: NTP retrieved time is: ");
      
      Serial.println(&ntpTimeInfo, "%A, %B %d %Y %H:%M:%S");
      
//      Serial.print("Day of week: ");
//      Serial.println(&ntpTimeInfo, "%A");
//      Serial.print("Month: ");
//      Serial.println(&ntpTimeInfo, "%B");
//      Serial.print("Day of Month: ");
//      Serial.println(&ntpTimeInfo, "%d");
//      Serial.print("Year: ");
//      Serial.println(&ntpTimeInfo, "%Y");
//      Serial.print("Hour: ");
//      Serial.println(&ntpTimeInfo, "%H");
//      Serial.print("Hour (12 hour format): ");
//      Serial.println(&ntpTimeInfo, "%I");
//      Serial.print("Minute: ");
//      Serial.println(&ntpTimeInfo, "%M");
//      Serial.print("Second: ");
//      Serial.println(&ntpTimeInfo, "%S");
    
//      Serial.println("Time variables");
//      char timeHour[3];
//      strftime(timeHour,3, "%H", &ntpTimeInfo);
//      Serial.println(timeHour);
//      char timeWeekDay[10];
//      strftime(timeWeekDay,10, "%A", &ntpTimeInfo);
//      Serial.println(timeWeekDay);
//      Serial.println();
    }
  }
  return getNTPSuccess;
  
}


void updateRTCtoNTP() {
      rtc.adjust(DateTime(ntpTimeInfo.tm_year+1900, ntpTimeInfo.tm_mon+1, ntpTimeInfo.tm_mday, ntpTimeInfo.tm_hour, ntpTimeInfo.tm_min, ntpTimeInfo.tm_sec));
}


bool verifyTime() {
  //compare RTC time to NTP time struct, return false if RTC time doe snot match NTP, or if can't get NTP
  bool timesMatch = false;

  DateTime rtc_now = rtc.now();

  if ( getNTP() ) {
    if ( rtc_now.year() ==  ntpTimeInfo.tm_year+1900 && rtc_now.month() ==  ntpTimeInfo.tm_mon+1 && rtc_now.day() ==  ntpTimeInfo.tm_mday && rtc_now.hour() ==  ntpTimeInfo.tm_hour && rtc_now.minute() ==  ntpTimeInfo.tm_min) {
      timesMatch = true;
      if (debugSerial) {
        Serial.println("Info: Times Match.");
      }
    } else {
      if (debugSerial) {
        Serial.print("WARN: RTC Time: ");
        Serial.print(rtc_now.year());
        Serial.print("-");
        Serial.print(rtc_now.month());
        Serial.print("-");
        Serial.print(rtc_now.day());
        Serial.print("T");
        Serial.print(rtc_now.hour());
        Serial.print(":");
        Serial.print(rtc_now.minute());
        Serial.print(" does not match NTP Time: ");
        Serial.print(ntpTimeInfo.tm_year+1900);
        Serial.print("-");
        Serial.print(ntpTimeInfo.tm_mon+1);
        Serial.print("-");
        Serial.print(ntpTimeInfo.tm_mday);
        Serial.print("T");
        Serial.print(ntpTimeInfo.tm_hour);
        Serial.print(":");
        Serial.println(ntpTimeInfo.tm_min);
        
        Serial.println("WARN: Updated RTC time to NTP time.");
      }
      updateRTCtoNTP();
    }
  } else {
    if (debugSerial) {
      Serial.println("WARN: Could not get NTP time, so could not verify RTC time.");
    }
  }
  return timesMatch;
}
