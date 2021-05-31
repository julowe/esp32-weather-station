struct Pollution {
  //Current
  char aqi[10];
  char co[10];
  char no[10];
  char no2[10];
  char o3[10];
  char so2[10];
  char pm2_5[10];
  char pm10[10];
  char nh3[10];
  
  //Current time returned in json data
  int updated;
};

boolean fillPollutionFromJson(Pollution* pollution_data) {
  boolean fillPollutionSuccess = false;
  sprintf(pollution_data->aqi, "%1i",  (int) jsonResult["list"][0]["main"]["aqi"] ); //shoudl always be 1, but in case
  sprintf(pollution_data->co, "%.2f",  (double) jsonResult["list"][0]["components"]["co"] );
  sprintf(pollution_data->no, "%.2f",  (double) jsonResult["list"][0]["components"]["no"] );
  sprintf(pollution_data->no2, "%.2f",  (double) jsonResult["list"][0]["components"]["no2"] );
  sprintf(pollution_data->o3, "%.2f",  (double) jsonResult["list"][0]["components"]["o3"] );
  sprintf(pollution_data->so2, "%.2f",  (double) jsonResult["list"][0]["components"]["so2"] );
  sprintf(pollution_data->pm2_5, "%.2f",  (double) jsonResult["list"][0]["components"]["pm2_5"] );
  sprintf(pollution_data->pm10, "%.2f",  (double) jsonResult["list"][0]["components"]["pm10"] );
  sprintf(pollution_data->nh3, "%.2f",  (double) jsonResult["list"][0]["components"]["nh3"] );
  

  int timezone_offset = -25200; //not provided in pollution API, set yourself  
  int dt = (int) jsonResult["list"][0]["dt"];
  int t = dt + timezone_offset;
  pollution_data->updated = t ;

  fillPollutionSuccess = true; //ok yeah this doesnt do anything

  return fillPollutionSuccess;

}


void printPollutionDebug(Pollution* pollution_data) {
  
  //data
  Serial.print("aqi: ");
  Serial.println(pollution_data->aqi);
  Serial.print("co: ");
  Serial.println(pollution_data->co);
  Serial.print("no: ");
  Serial.println(pollution_data->no);
  Serial.print("no2: ");
  Serial.println(pollution_data->no2);
  Serial.print("o3: ");
  Serial.println(pollution_data->o3);
  Serial.print("so2: ");
  Serial.println(pollution_data->so2);
  Serial.print("pm2_5: ");
  Serial.println(pollution_data->pm2_5);
  Serial.print("pm10: ");
  Serial.println(pollution_data->pm10);
  Serial.print("nh3: ");
  Serial.println(pollution_data->nh3);

  //updated time from json data
  Serial.print("Updated at: ");
  Serial.print(year(pollution_data->updated));
  Serial.print("-");
  Serial.print(month(pollution_data->updated));
  Serial.print("-");
  Serial.print(day(pollution_data->updated));
  Serial.print(" ");
  Serial.print(hour(pollution_data->updated));
  Serial.print(":");
  Serial.println(minute(pollution_data->updated));
}
