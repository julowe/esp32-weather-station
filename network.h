//TODO add checkWifi function

boolean connectToWifi() {
  Serial.print("\n Attempting to connect to ");
  Serial.println(WIFI_SSID);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Already connected to Wifi SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP(0));
    return true;
  } else {
    WiFi.setHostname(ESP32_HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned int waitIntervals = 90;
    while (WiFi.status() != WL_CONNECTED && (waitIntervals-- > 0)) {
      Serial.print(".");
      delay(1000);
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWifi connection failed");
      return false;
    }
    Serial.println("");
    Serial.println("wifi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP(0));
    Serial.println("");
    return true;
  }
}

boolean disconnectFromWifi() {
  WiFi.disconnect();
}

boolean getJSON(const char* url) {
  boolean success = false;
   
  if ((WiFi.status() == WL_CONNECTED)) {

    Serial.print("Connecting to ");
    Serial.println(url);
    
    HTTPClient http;
    http.begin(url);
    
    if ( strcmp(url, URL_pollen) == 0 ) {
      Serial.println("Adding Pollen API Key to headers");
      http.addHeader("x-api-key", APIKEY_pollen);
    }
    
    int httpCode = http.GET();
//    Serial.print("HTTP code : "); //ok this isn't printing and I don't want to debug. moved to below error printing, which does work
//    Serial.println(httpCode);
    if (httpCode == 200) { //is this too broad? do we want 429/auth problems to fall through to be handled elsewhere??

//      TODO [D][HTTPClient.cpp:1260] handleHeaderResponse(): size: 18612
//      myString.reserve(size);

      jsonResult = JSON.parse(http.getString());
      
      if (JSON.typeof(jsonResult) == "undefined") { 
        //TODO improve error reporting
        Serial.print("Parsing input failed! HTTP code : ");
        Serial.println(httpCode);
      } else {
        success = true;
      }
    } else {
      Serial.print("Problem getting data! HTTP code : ");
      Serial.println(httpCode);
    }
    http.end();
  }  
  return success;
}
