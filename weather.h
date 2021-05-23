struct Weather {
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
  sprintf(weather->iconH1, "%s", (const char*) jsonWeather["hourly"][1]["weather"][0]["icon"]);
  sprintf(weather->tempH1, "%2i\xb0", (int) round(jsonWeather["hourly"][1]["temp"]));
  sprintf(weather->feelsLikeH1, "%2i\xb0", (int) round(jsonWeather["hourly"][1]["feels_like"]));
  sprintf(weather->humidityH1, "%3i %%", (int) jsonWeather["hourly"][1]["humidity"]);
  sprintf(weather->popH1, "%2.2f",  (double) jsonWeather["hourly"][1]["pop"] );
  sprintf(weather->popH4, "%2.2f",  (double) jsonWeather["hourly"][4]["pop"] ); //FIXME temp assignment to get a nonzero value

  sprintf(weather->iconD, "%s", (const char*) jsonWeather["daily"][0]["weather"][0]["icon"]);
  sprintf(weather->tempMinD, "%2i\xb0", (int) round(jsonWeather["daily"][0]["temp"]["min"]));
  sprintf(weather->tempMaxD, "%2i\xb0", (int) round(jsonWeather["daily"][0]["temp"]["max"]));
  sprintf(weather->humidityD, "%3i %%", (int) jsonWeather["daily"][0]["humidity"]);

  sprintf(weather->iconD1, "%s", (const char*) jsonWeather["daily"][1]["weather"][0]["icon"]);
  sprintf(weather->tempMinD1, "%2i\xb0", (int) round(jsonWeather["daily"][1]["temp"]["min"]));
  sprintf(weather->tempMaxD1, "%2i\xb0", (int) round(jsonWeather["daily"][1]["temp"]["max"]));
  sprintf(weather->humidityD1, "%3i %%", (int) jsonWeather["daily"][1]["humidity"]);

  int timezone_offset = (int) jsonWeather["timezone_offset"];
  int dt = (int) jsonWeather["current"]["dt"];
  int t = dt + timezone_offset;
  sprintf(weather->updated, "MAJ : %02d/%02d %02d:%02d", day(t), month(t), hour(t), minute(t));
}
