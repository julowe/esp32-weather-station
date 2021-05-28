
void trelloRandomCard(char* cardName) {
  //start at 2
  snprintf(cardName, 30, "%s", (const char*) jsonResult[random(2,jsonResult.length())-1]["name"]);
  
}


void trelloFirstCard(char* cardName) {

  snprintf(cardName, 30, "%s", (const char*) jsonResult[0]["name"]);
  
}
