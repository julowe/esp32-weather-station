// Display things on a Hub75 display with ESP32
// uses ESP32 DevKit v1 board, and `DOIT ESP32 DEVKIT V1` package name from: https://dl.espressif.com/dl/package_esp32_index.json
// 2021-04-25 jkl

/* includes much code from: 
 *  HackerBox 0065 Clock Demo for 64x32 LED Array, https://www.instructables.com/HackerBox-0065-Realtime/
 *  Adapted from SmartMatrix example file
 *  in boolean RTC_DS1307::begin(void):
 *  Change Wire.begin() to Wire.begin(14, 13)
*/

/*
 * forked from: 
 * https://github.com/paulgreg/esp32-weather-station
*/

//TODO
//todo add air pollution, pm2.5 (or10?) for smoke season https://openweathermap.org/api/air-pollution
//todo add covid data (see parameters.h)
//todo use NTP?
//TODO better differentiate get or parse data failures? But not sure if I would do anything different with different failures...
//TODO save data to SD card?
//TODO sleep during display intervals? or keep arduino on?
//TODO move led display code to display-led.h

/* TODO Fix out of memory httpclient.getstring() error. Or just keep letting it retrieve it twice?
 *  [D][HTTPClient.cpp:947] getString(): not enough memory to reserve a string! need: 17987
 *  Parsing input failed!
 *  [D][HTTPClient.cpp:378] disconnect(): still data in buffer (5368), clean up.
*/

   
//Data retrieval stuff
#include <Time.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h> //QUESTION: any benefit to using ArduinoJson library here instead?
JSONVar jsonWeather;

#include "parameters.h"
#include "weather.h"
//#include "display.h" //TODO - use weather icons from this file
//#include "display-led.h"
#include "network.h"

const uint64_t SECOND = 1000;
const uint64_t MINUTE = 60 * SECOND;
const uint64_t HOUR = 60 * MINUTE;
const uint64_t MICRO_SEC_TO_MILLI_SEC_FACTOR = 1000;
uint64_t sleepTime = MINUTE;

//declare struct for weather data to be put in
Weather weather_data;

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

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer1, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer2, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer3, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

RTC_DS1307 rtc;
const int defaultBrightness = (35*255)/100;     // dim: 35% brightness
//TODO use time lib instead of below?
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char monthsOfTheYr[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


//debug/admin stuff
const bool debug = true;
const bool debugSerial = true;

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

  displayClock();

  indexedLayer3.fillScreen(0);
  
  indexedLayer3.setFont(font3x5);
  indexedLayer3.setIndexedColor(1,{0x00, 0x00, 0xff});
  indexedLayer3.drawString(0, 25, 1, "Initializing");
  indexedLayer3.swapBuffers();


  //get data at startup
  bool dataWrapperSuccess = getDataWrapper("weather", 5, 2); //try connecting to wifi 5 times, getting data twice

//  disconnectFromWifi(); //TODO do we want to explicitly disconnect from wifi between updates? chip heat savings?

  //Display Data
  if (dataWrapperSuccess) {
    //function to display data on led matrix TODO
    displayWeather(&weather_data);
  } else {
    indexedLayer2.fillScreen(0);
    indexedLayer3.fillScreen(0);
  
    indexedLayer3.setFont(font3x5);
    indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
    indexedLayer3.drawString(0, 25, 1, "Failed to Initialize");
    indexedLayer3.swapBuffers();
  }
  
}

void loop() {
  bool dataWrapperSuccess = false;
  if (debugSerial) {
    Serial.println("---- Starting loop() ----");
  }


  DateTime now = rtc.now();
  
  if (now.minute() % weatherUpdateInterval == 0){ // update every `weatherUpdateInterval`-th mins
    if (debugSerial) {
      Serial.print("Updating weather because it is a ");
      Serial.print(weatherUpdateInterval);
      Serial.print(" minute mark, time is: ");
      Serial.print(now.hour());
      Serial.print(":");
      print2digitsLn(now.minute());
//      Serial.println(now.minute());
    }

    dataWrapperSuccess = getDataWrapper();
    
    if (debugSerial && dataWrapperSuccess) {
      Serial.println("Succesfully got weather data");
      printWeatherDebug(&weather_data);
    } else if (debugSerial && !dataWrapperSuccess) {
      Serial.println("ERROR: Did not retrieve weather data.");
    }       
    
    //Display Data
    if (dataWrapperSuccess) {
      //TODO make 'last updated' it's own layer
      displayWeather(&weather_data);
    } else {
      if (debugSerial) {
        Serial.println("Updating matrix with failure message");
      }
      //TODO display anything different here? or just leave old data up? update 'last updated' layer only?

      indexedLayer2.fillScreen(0);
      indexedLayer2.swapBuffers();
      indexedLayer3.fillScreen(0);
    
      indexedLayer3.setFont(font3x5);
      indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
      indexedLayer3.drawString(0, 25, 1, "Failed to Update");
      indexedLayer3.swapBuffers();
    }
  
  } else {
    if (debugSerial) {
      Serial.print("NOT updating weather because it is NOT ");
      Serial.print(weatherUpdateInterval);
      Serial.print(" minute mark, minute = ");
      print2digitsLn(now.minute());
//      Serial.println(now.minute());
    }
  }


//  if (now.hour() % covidUpdateInterval == 0 && now.minute() == 0){ // update every Xth hours
  if (now.hour() % covidUpdateInterval == 0){ // update every Xth hours
    if (debugSerial) {
      Serial.print("Updating covid data because it is a ");
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


    dataWrapperSuccess = getDataWrapper("covid");
    
    if (debugSerial && dataWrapperSuccess) {
      Serial.println("Succesfully got covid data");
//      printWeatherDebug(&weather_data); //TODO make covid data display debug?
    } else if (debugSerial && !dataWrapperSuccess) {
      Serial.println("ERROR: Did not retrieve covid data.");
    }       
    
    //Display Data
    if (dataWrapperSuccess) {
      //TODO make 'last updated' it's own layer
//      displayWeather(&weather_data); //TODO make covid data display function
    } else {
      if (debugSerial) {
//        Serial.println("Updating matrix with failure message");
        Serial.println("Not changing led display as displaying covid data is a work in progress.");
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
      Serial.print("NOT updating covid data because it is NOT ");
      Serial.print(covidUpdateInterval);
      Serial.print(" hour mark, time is: ");
      Serial.print(now.hour());
      Serial.print(":");
      print2digitsLn(now.minute());
//      Serial.println(now.minute());
    }
  }



//////////////////////////////////////////////
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
    Serial.print("Sleeping for ");
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

void displayClock() {
  DateTime now = rtc.now();
  
  //TODO update display with actual data
  char txtBuffer[20];

  // clear screen before writing new text
  indexedLayer1.fillScreen(0);

  //drawString of all date data at once left large gaps (5?6? pixels), so I split up draw commands to only leave 2 pixel gaps
  indexedLayer1.setFont(font3x5);
  indexedLayer1.setIndexedColor(1,{0x00, 0x00, 0xff});
//  indexedLayer1.setIndexedColor(2,{0xff, 0x00, 0x00});
  
  //Weekday
//  sprintf(txtBuffer, "%s %02d %s %02d:%02d", daysOfTheWeek[now.dayOfTheWeek()], now.day(), monthsOfTheYr[(now.month()-1)], now.hour(), now.minute());
  sprintf(txtBuffer, "%s", daysOfTheWeek[now.dayOfTheWeek()]);
  indexedLayer1.drawString(0, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();

  //Day date
  sprintf(txtBuffer, "%02d", now.day());
//  indexedLayer1.setFont(font3x5);
  indexedLayer1.drawString(14, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();

  //month
  sprintf(txtBuffer, "%s", monthsOfTheYr[(now.month()-1)]);
//  indexedLayer1.setFont(font3x5);
  indexedLayer1.drawString(24, 0, 1, txtBuffer);
  indexedLayer1.swapBuffers();

  //time
  sprintf(txtBuffer, "%d:%02d", now.hour(), now.minute());
//  indexedLayer1.setFont(font3x5);
  indexedLayer1.drawString(38, 0, 1, txtBuffer);
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
  indexedLayer2.fillScreen(0);
  indexedLayer3.fillScreen(0);
  
//  indexedLayer2.setFont(font8x13);
  indexedLayer2.setFont(font5x7);
  indexedLayer2.setIndexedColor(1,{0x00, 0xff, 0x00});
  indexedLayer2.drawString(0, 6, 1, weather_data->descriptionC);
  indexedLayer2.swapBuffers();

  int pop1Temp = atof(weather_data->popH1)*100;
//  sprintf(txtBuffer, "%s %s", weather_data->tempC, weather_data->tempH1);
  sprintf(txtBuffer, "%s %s %i%%", weather_data->tempC, weather_data->tempH1, pop1Temp);
//  sprintf(txtBuffer, "%s %s %3i%% %3i%%", weather_data->tempC, weather_data->tempH1, atof(weather_data->popH1)*100, atof(weather_data->popH4)*100);
  indexedLayer3.setFont(font5x7);
  indexedLayer3.setIndexedColor(1,{0xff, 0x00, 0x00});
  indexedLayer3.drawString(0, 14, 1, txtBuffer);
  indexedLayer3.swapBuffers();
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

  while (getDataTrialNumber <= getDataTries && !dataSuccess) {
    Serial.print("Attempt #");
    Serial.print(getDataTrialNumber);
    Serial.print("/");
    Serial.print(getDataTries);
    Serial.print(" for ");
    
    Serial.println("retrieving data...");
    
    //connect to wifi
    if (WiFi.status() == WL_CONNECTED) {

      if ( data_source == "weather") {
        if (debugSerial) {
          Serial.println("weather if");
        }
        dataSuccess = getJSON(URL_weather);
      } else if ( data_source == "pollution") {
        if (debugSerial) {
          Serial.println("pollution if");
        }
        dataSuccess = getJSON(URL_pollution);
      } else if ( data_source == "covid") {
        if (debugSerial) {
          Serial.println("covid if");
        }
        Serial.println("Covid data retrieval not yet implemented.");
//        dataSuccess = getJSON(URL_covid_base);
      } else {
        if (debugSerial) {
          Serial.println("default else");
        }
        dataSuccess = getJSON(URL_weather);
      }
      
      if (dataSuccess) {

        if ( data_source == "weather") {
          if (debugSerial) {
            Serial.println("dataSuccess weather if");
          }
          fillWeatherFromJson(&weather_data); //weather.h
        } else if ( data_source == "pollution") {
          if (debugSerial) {
            Serial.println("dataSuccess pollution if");
          }
          Serial.println("Pollution data storage not yet implemented.");
//          fillPollutionFromJson(&pollution_data); //weather.h
        } else if ( data_source == "covid") {
          if (debugSerial) {
            Serial.println("dataSuccess covid if");
          }
          Serial.println("Covid data storage not yet implemented.");
//          fillCovidDataFromJson(&covid_data); //weather.h
        } else {
          if (debugSerial) {
            Serial.println("dataSuccess default else");
          }
          fillWeatherFromJson(&weather_data); //weather.h
        }
        
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
