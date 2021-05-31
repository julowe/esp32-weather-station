 /* current - 1 query
  *  {
    *  "message":"Success",
    *  "lat":47.654133,
    *  "lng":-122.348329,
    *  "data":[
      *  {"Count":
        *  {"grass_pollen":4,
        *  "tree_pollen":20,
        *  "weed_pollen":2},
      *  "Risk":
        *  {"grass_pollen":"Low",
        *  "tree_pollen":"Low",
        *  "weed_pollen":"Low"},
      *  "updatedAt":"2021-05-30T16:43:02.000Z"}
    *  ]
  *  }
  *  
  *  RESPONSE PARAMETERS
  *  no info on values - integers??
  *  
Grass Pollen Pollen grains from grasses. Measuring unit is pollen particles/m3
Tree Pollen Pollen from trees such as Birch and Oak. Measuring unit is pollen particles/m3
Risk  Provides a risk evaluation with levels
Low - Mild risk to those with severe respiratory issues. No risk for the general public
Moderate - Risky for those with severe respiratory problems. Mild risk for the general public
High - Risky for all groups of people
Very High - Highly risky for all groups of people
  */

struct Pollen {
  //Current
  char grass_num[10];
  char tree_num[10];
  char weed_num[10];
  
  char grass_desc[10];
  char tree_desc[10];
  char weed_desc[10];
  
  //data updated time returned in json data
  char updated[30]; //iso, should always be 24. NB: ZULU TIME!
};

boolean fillPollenFromJson(Pollen* pollen_data) {

  //error checking - lazy try/catch, or check format of json, or some both?
  //failng because going over daily api call limit...

  //TODO move float to char conversion to display function? if we want to do math etc with actual floats...
  sprintf(pollen_data->grass_num, "%i",  (int) jsonResult["data"][0]["Count"]["grass_pollen"] );
  sprintf(pollen_data->tree_num, "%i",  (int) jsonResult["data"][0]["Count"]["tree_pollen"] );
  sprintf(pollen_data->weed_num, "%i",  (int) jsonResult["data"][0]["Count"]["weed_pollen"] );
  
  sprintf(pollen_data->grass_desc, "%.9s",  (const char*) jsonResult["data"][0]["Risk"]["grass_pollen"] );
  sprintf(pollen_data->tree_desc, "%.9s",  (const char*) jsonResult["data"][0]["Risk"]["tree_pollen"] );
  sprintf(pollen_data->weed_desc, "%.9s",  (const char*) jsonResult["data"][0]["Risk"]["weed_pollen"] );

  //NOTE: Remember this is Zulu time
  sprintf(pollen_data->updated, "%.29s",  (const char*)  jsonResult["data"][0]["updatedAt"] );

}


void printPollenDebug(Pollen* pollen_data) {
  
  //data
  Serial.print("Grass count: ");
  Serial.println(pollen_data->grass_num);
//  Serial.println(jsonResult["data"][0]["Count"]["grass_pollen"]);
  Serial.print("Grass desc: ");
  Serial.println(pollen_data->grass_desc);
//  Serial.println(jsonResult["data"][0]["Risk"]["grass_pollen"]);
  
  Serial.print("Tree count: ");
  Serial.println(pollen_data->tree_num);
//  Serial.println(jsonResult["data"][0]["Count"]["tree_pollen"]);
  Serial.print("Tree desc: ");
  Serial.println(pollen_data->tree_desc);
//  Serial.println(jsonResult["data"][0]["Risk"]["tree_pollen"]);
  
  Serial.print("Weed count: ");
  Serial.println(pollen_data->weed_num);
//  Serial.println(jsonResult["data"][0]["Count"]["weed_pollen"]);
  Serial.print("Weed desc: ");
  Serial.println(pollen_data->weed_desc);
//  Serial.println(jsonResult["data"][0]["Risk"]["weed_pollen"]);
  
  Serial.print("Updated at: ");
  Serial.println(pollen_data->updated);
//  Serial.println(jsonResult["data"][0]["updatedAt"]);

}

  
