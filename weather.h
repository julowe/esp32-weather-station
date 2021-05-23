struct Weather {
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
  
  char iconH1[10];
  char tempH1[10];
  char feelsLikeH1[10];
  char humidityH1[6];
  char popH1[10];
  char popH4[10];

  char iconD[10];
  char tempMinD[10];
  char tempMaxD[10];
  char humidityD[6];

  char iconD1[10];
  char tempMinD1[10];
  char tempMaxD1[10];
  char humidityD1[6];

  char updated[20];
};

void fillWeatherFromJson(Weather* weather) {

  sprintf(weather->descriptionC, "%s", (const char*) jsonWeather["current"]["weather"][0]["description"]);
  sprintf(weather->descriptionShortC, "%s", (const char*) jsonWeather["current"]["weather"][0]["main"]);
  sprintf(weather->tempC, "%2i\xb0", (int) round(jsonWeather["current"]["temp"]));
  
  int timezone_offset = (int) jsonWeather["timezone_offset"];
  int sunset = (int) jsonWeather["current"]["sunset"];
  int sunset_t = sunset + timezone_offset;
  sprintf(weather->sunset, "%2d:%02d", hour(sunset_t)-12, minute(sunset_t));
  
  sprintf(weather->pressureC, "%4i", (int) jsonWeather["current"]["pressure"]);

  int humidityInt = round(jsonWeather["current"]["humidity"]);
  if (humidityInt == 100){ //is humidty ever reported as 100? do this for screen layout reasons.
    humidityInt = 99;
  }
  sprintf(weather->humidityC, "%2i%%", (int) humidityInt);
  
  sprintf(weather->uvIndexC, "%2i", (int) round(jsonWeather["current"]["uvi"]));
  sprintf(weather->uvIndexC, "%1.2f",  (double) jsonWeather["current"]["uvi"] );
  
  sprintf(weather->windSpeedC, "%2i", (int) round(jsonWeather["current"]["wind_speed"]));
//  char windDirectionC[10];
  sprintf(weather->windDirectionC, "%3i", (int) round(jsonWeather["current"]["wind_deg"]));
//  char windGustC[10];
  sprintf(weather->windGustC, "%2i", (int) round(jsonWeather["current"]["wind_gust"]));

  
  sprintf(weather->iconH1, "%s", (const char*) jsonWeather["hourly"][1]["weather"][0]["icon"]);
  sprintf(weather->tempH1, "%2i\xb0", (int) round(jsonWeather["hourly"][1]["temp"]));
  sprintf(weather->feelsLikeH1, "%2i\xb0", (int) round(jsonWeather["hourly"][1]["feels_like"]));
  sprintf(weather->humidityH1, "%3i %%", (int) jsonWeather["hourly"][1]["humidity"]);
  sprintf(weather->popH1, "%1.2f",  (double) jsonWeather["hourly"][1]["pop"] );
  sprintf(weather->popH4, "%1.2f",  (double) jsonWeather["hourly"][4]["pop"] ); //FIXME temp assignment to get a nonzero value

  sprintf(weather->iconD, "%s", (const char*) jsonWeather["daily"][0]["weather"][0]["icon"]);
  sprintf(weather->tempMinD, "%2i\xb0", (int) round(jsonWeather["daily"][0]["temp"]["min"]));
  sprintf(weather->tempMaxD, "%2i\xb0", (int) round(jsonWeather["daily"][0]["temp"]["max"]));
  sprintf(weather->humidityD, "%3i %%", (int) jsonWeather["daily"][0]["humidity"]);

  sprintf(weather->iconD1, "%s", (const char*) jsonWeather["daily"][1]["weather"][0]["icon"]);
  sprintf(weather->tempMinD1, "%2i\xb0", (int) round(jsonWeather["daily"][1]["temp"]["min"]));
  sprintf(weather->tempMaxD1, "%2i\xb0", (int) round(jsonWeather["daily"][1]["temp"]["max"]));
  sprintf(weather->humidityD1, "%3i %%", (int) jsonWeather["daily"][1]["humidity"]);

//  int timezone_offset = (int) jsonWeather["timezone_offset"]; //set above already
  int dt = (int) jsonWeather["current"]["dt"];
  int t = dt + timezone_offset;
  sprintf(weather->updated, "MAJ : %02d/%02d %02d:%02d", day(t), month(t), hour(t), minute(t));
}
