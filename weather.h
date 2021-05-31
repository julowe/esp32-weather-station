struct Weather {
  //Current
  char descriptionC[30];
  char descriptionShortC[20];
  char sunset[15];
  char tempC[10];
  char pressureC[10];
  char humidityC[10];
  char uvIndexC[10];
  char windSpeedC[10];
  char windDirectionC[10];
  char windGustC[10];

  //this hour
  char popH0[10]; //Probability Of Precipitation
  

  //Next Hour (NB: 1, not 0; so not this current hour)
  char descriptionH1[30];
  char descriptionShortH1[20];
  char iconH1[10];
  char tempH1[10];
  char feelsLikeH1[10];
  char humidityH1[6];
  char popH1[10]; //Probability Of Precipitation

  //4 hours ahead
  char popH4[10];

  //Today
  char tempDayD[10];
  char tempNightD[10];
  char tempEveD[10];
  char tempMornD[10];
  char descriptionD[30];
  char descriptionShortD[20];
  char iconD[10];
  char tempMinD[10];
  char tempMaxD[10];
  char humidityD[6];

  //Next Day
  char descriptionD1[30];
  char descriptionShortD1[20];
  char iconD1[10];
  char tempMinD1[10];
  char tempMaxD1[10];
  char humidityD1[6];

  //Current time returned in json data
//  int updated[20];
  int updated;
};

boolean fillWeatherFromJson(Weather* weather_data) {
  boolean fillWeatherSuccess = false;
        
  //current
  sprintf(weather_data->descriptionC, "%s", (const char*) jsonResult["current"]["weather"][0]["description"]);
  sprintf(weather_data->descriptionShortC, "%s", (const char*) jsonResult["current"]["weather"][0]["main"]);
  sprintf(weather_data->tempC, "%2i\xb0", (int) round(jsonResult["current"]["temp"]));
  
  int timezone_offset = (int) jsonResult["timezone_offset"];
  int sunset = (int) jsonResult["current"]["sunset"];
  int sunset_t = sunset + timezone_offset;
  sprintf(weather_data->sunset, "%2d:%02d", hour(sunset_t)-12, minute(sunset_t));
  
  sprintf(weather_data->pressureC, "%4i", (int) jsonResult["current"]["pressure"]);

  int humidityInt = round(jsonResult["current"]["humidity"]);
  if (humidityInt == 100){ //is humidty ever reported as 100? do this for screen layout reasons.
    humidityInt = 99;
  }
  sprintf(weather_data->humidityC, "%2i%%", (int) humidityInt);

  //TODO do i want a float or just the rounded int?
//  sprintf(weather_data->uvIndexC, "%1i", (int) round(jsonResult["current"]["uvi"]));
  sprintf(weather_data->uvIndexC, "%1.2f",  (double) jsonResult["current"]["uvi"] );
  
  sprintf(weather_data->windSpeedC, "%2i", (int) round(jsonResult["current"]["wind_speed"]));
  sprintf(weather_data->windDirectionC, "%3i", (int) round(jsonResult["current"]["wind_deg"]));
  sprintf(weather_data->windGustC, "%2i", (int) round(jsonResult["current"]["wind_gust"]));

  //this hour
  sprintf(weather_data->popH0, "%1.2f",  (double) jsonResult["hourly"][0]["pop"] );


  //next hour
  sprintf(weather_data->descriptionH1, "%s", (const char*) jsonResult["hourly"][1]["weather"][0]["description"]);
  sprintf(weather_data->descriptionShortH1, "%s", (const char*) jsonResult["hourly"][1]["weather"][0]["main"]);
  sprintf(weather_data->iconH1, "%s", (const char*) jsonResult["hourly"][1]["weather"][0]["icon"]);
  sprintf(weather_data->tempH1, "%2i\xb0", (int) round(jsonResult["hourly"][1]["temp"]));
  sprintf(weather_data->feelsLikeH1, "%2i\xb0", (int) round(jsonResult["hourly"][1]["feels_like"]));
  sprintf(weather_data->humidityH1, "%3i %%", (int) jsonResult["hourly"][1]["humidity"]);
  sprintf(weather_data->popH1, "%1.2f",  (double) jsonResult["hourly"][1]["pop"] );
  sprintf(weather_data->popH4, "%1.2f",  (double) jsonResult["hourly"][4]["pop"] ); //FIXME temp assignment to get a nonzero value

  //today
  sprintf(weather_data->tempDayD, "%2i\xb0", (int) round(jsonResult["daily"][0]["temp"]["day"]));
  sprintf(weather_data->tempNightD, "%2i\xb0", (int) round(jsonResult["daily"][0]["temp"]["night"]));
  sprintf(weather_data->tempEveD, "%2i\xb0", (int) round(jsonResult["daily"][0]["temp"]["eve"]));
  sprintf(weather_data->tempMornD, "%2i\xb0", (int) round(jsonResult["daily"][0]["temp"]["morn"]));

  sprintf(weather_data->descriptionD, "%s", (const char*) jsonResult["daily"][0]["weather"][0]["description"]);
  sprintf(weather_data->descriptionShortD, "%s", (const char*) jsonResult["daily"][0]["weather"][0]["main"]);
  sprintf(weather_data->iconD, "%s", (const char*) jsonResult["daily"][0]["weather"][0]["icon"]);
  sprintf(weather_data->tempMinD, "%2i\xb0", (int) round(jsonResult["daily"][0]["temp"]["min"]));
  sprintf(weather_data->tempMaxD, "%2i\xb0", (int) round(jsonResult["daily"][0]["temp"]["max"]));
  sprintf(weather_data->humidityD, "%3i %%", (int) jsonResult["daily"][0]["humidity"]);

  //next day
  sprintf(weather_data->descriptionD1, "%s", (const char*) jsonResult["daily"][1]["weather"][0]["description"]);
  sprintf(weather_data->descriptionShortD1, "%s", (const char*) jsonResult["daily"][1]["weather"][0]["main"]);
  sprintf(weather_data->iconD1, "%s", (const char*) jsonResult["daily"][1]["weather"][0]["icon"]);
  sprintf(weather_data->tempMinD1, "%2i\xb0", (int) round(jsonResult["daily"][1]["temp"]["min"]));
  sprintf(weather_data->tempMaxD1, "%2i\xb0", (int) round(jsonResult["daily"][1]["temp"]["max"]));
  sprintf(weather_data->humidityD1, "%3i %%", (int) jsonResult["daily"][1]["humidity"]);

//  int timezone_offset = (int) jsonResult["timezone_offset"]; //set above already
  int dt = (int) jsonResult["current"]["dt"];
  int t = dt + timezone_offset;
//  sprintf(weather_data->updated, "MAJ : %02d/%02d %02d:%02d", day(t), month(t), hour(t), minute(t));
  weather_data->updated = t ;

  fillWeatherSuccess = true;

  return fillWeatherSuccess;
  
}


void printWeatherDebug(Weather* weather_data) {

  //current
  Serial.print("descriptionC: ");
  Serial.println(weather_data->descriptionC);
  Serial.print("descriptionShortC: ");
  Serial.println(weather_data->descriptionShortC);
  Serial.print("sunset: ");
  Serial.println(weather_data->sunset);
  Serial.print("tempC: ");
  Serial.println(weather_data->tempC);
  Serial.print("pressureC: ");
  Serial.println(weather_data->pressureC);
  Serial.print("humidityC: ");
  Serial.println(weather_data->humidityC);
  Serial.print("uvIndexC: ");
  Serial.println(weather_data->uvIndexC);  
  Serial.print("windSpeedC: ");
  Serial.println(weather_data->windSpeedC);
  Serial.print("windDirectionC: ");
  Serial.println(weather_data->windDirectionC);
  Serial.print("windGustC: ");
  Serial.println(weather_data->windGustC);
  
  //next hour
  Serial.print("descriptionH1: ");
  Serial.println(weather_data->descriptionH1);
  Serial.print("descriptionShortH1: ");
  Serial.println(weather_data->descriptionShortH1);
  Serial.print("iconH1: ");
  Serial.println(weather_data->iconH1);
  Serial.print("Hourly High Temp: ");
  Serial.println(weather_data->tempH1);
  Serial.print("Hourly Feels like: ");
  Serial.println(weather_data->feelsLikeH1); //prints ⸮ instead of degree sign \b0
  Serial.print("Hourly Humidity: ");
  Serial.println(weather_data->humidityH1);
  Serial.print("Hourly PoP: ");
  Serial.println(weather_data->popH1);

  //4 hours from now
  Serial.print("Hourly PoP4: ");
  Serial.println(weather_data->popH4);
  
  //today
  Serial.print("tempDayD: ");
  Serial.println(weather_data->tempDayD);
  Serial.print("tempNightD: ");
  Serial.println(weather_data->tempNightD);
  Serial.print("tempEveD: ");
  Serial.println(weather_data->tempEveD);
  Serial.print("tempMornD: ");
  Serial.println(weather_data->tempMornD); 
  Serial.print("descriptionD: ");
  Serial.println(weather_data->descriptionD);
  Serial.print("descriptionShortD: ");
  Serial.println(weather_data->descriptionShortD);
  Serial.print("iconD: ");
  Serial.println(weather_data->iconD);
  Serial.print("Today Min Temp: ");
  Serial.println(weather_data->tempMinD);
  Serial.print("Today Max Temp: ");
  Serial.println(weather_data->tempMaxD);
  Serial.print("Today Humidity: ");
  Serial.println(weather_data->humidityD);

  //next day
  Serial.print("descriptionD1: ");
  Serial.println(weather_data->descriptionD1);
  Serial.print("descriptionShortD1: ");
  Serial.println(weather_data->descriptionShortD1);
  Serial.print("iconD1: ");
  Serial.println(weather_data->iconD1);
  Serial.print("Tomorrow Min Temp: ");
  Serial.println(weather_data->tempMinD1);
  Serial.print("Tomorrow Max Temp: ");
  Serial.println(weather_data->tempMaxD1);
  Serial.print("Tomorrow Humidity: ");
  Serial.println(weather_data->humidityD1);

  //current time from json data
  Serial.print("Updated at: ");
//  Serial.println(weather_data->updated);
  Serial.print(year(weather_data->updated));
  Serial.print("-");
  Serial.print(month(weather_data->updated));
  Serial.print("-");
  Serial.print(day(weather_data->updated));
  Serial.print(" ");
  Serial.print(hour(weather_data->updated));
  Serial.print(":");
  Serial.println(minute(weather_data->updated));

}
