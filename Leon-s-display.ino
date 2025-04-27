/************************************************************************************************************************************************

/*  ============================================================================================================

NodeMCU 1.0 (ESP-12E Module)

  ============================================================================================================== */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Board_Identify.h>

#define DEBUG 
#define U8G2_WITH_UNICODE

#define VERSION         "1.1"

#define URL_HL_PREFIX   "https://api-samenmeten.rivm.nl/v1.0/Datastreams("
#define URL_HL_POSTFIX  ")/Observations?$top=1"
#define URL_SC          "https://data.sensor.community/airrohr/v1/sensor/"
#define URL_SL_PREFIX   "http://airrohr-"
#define URL_SL_POSTFIX  ".local/data.json"
#define URL_HR          "https://samenmeten.rivm.nl/hollandseluchten/sensordata.php"
#define URL_OM_PREFIX   "https://api.open-meteo.com/v1/forecast" // latitude=52.52&longitude=13.41
#define URL_OM_POSTFIX  "&current=temperature_2m,wind_speed_10m,wind_direction_10m,wind_gusts_10m"

#define HOSTNAME        "leons-display"  // this is also the mDNS responder name
#define UPDATE_PATH     "https://www.dropbox.com/scl/fo/n85gensqu8jg2jqayle35/APWvpD3LNh3kmTL0RPSl39U?rlkey=2pntmhdherfupdlqjm3lldorv&dl=0"

#define sensorType(st)  strcmp(user_settings.sensortype, st) == 0

#define DEG2RAD 0.0174532925

const String HTML_HEADER = "<!doctype html>"
                           "<html lang='en'>"
                           "<head>"
                              "<meta charset='utf-8'>"
                              "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                              "<title>Leon's display</title>"
                              "<style>"
                              "  button {width:200px;margin:4px},"
                              "  table  {cellspacing:0;cellpadding:5},"
                              "  title  {font-family:Verdana}"
                              "  h1     {font-family:Verdana}"
                              "  h2     {font-family:Verdana}"
                              "  p      {font-family:Verdana}"
                              "  td     {font-family:Verdana}"
                              "  button {font-family:Verdana}"
                              "  input  {font-family:Verdana}"
                              "</style>"
                           "</head>"
                           "<body>";
                           
const String HTML_FOOTER = "<form action='/' method='get'><button type='submit'>Home</button></form>"
                           "</body>"
                           "</html>";

const int ONE_MINUTE  = 1000 * 60;
const int TEN_MINUTES = ONE_MINUTE * 10;

const static uint8_t PROGMEM icon_nav_arrow_16x[] { 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x03, 0xc0, 0x03,
                                                    0xc0, 0x07, 0xe0, 0x07, 0xe0, 0x0f, 0xf0, 0x0f, 0xf0, 0x1f, 0xf8,
                                                    0x1f, 0xf8, 0x3e, 0x7c, 0x3c, 0x3c, 0x00, 0x00, 0x00, 0x00};

// Kleine Wemos D1 Mini USB-C bordje met losse oled display (SCL en SDA zijn al correct ingevuld)
// Gerard van Zelst heeft er zo een.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* SCL */ D1, /* SDA */ D2, /* reset */ U8X8_PIN_NONE);

// USB-C bord HW-364A met ingebouwde display
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* SCL */ D6, /* SDA */ D5, /* reset */ U8X8_PIN_NONE);

// Ideaspark Micro USB bord met ingebouwde display
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,  /* SCL */  D5,  /* SDA */  D6, /* reset */ U8X8_PIN_NONE);

ESP8266WebServer    server(80);

struct settings {
  char ssid[30];
  char password[30];
  char sensortype[3]; // can be sc (Sensor Community)/sl (Sensor Community Local)/ hl (Hollandse Luchten)/om (Open Meteo)
  char sensornr[10];
  char latitude[20];
  char longitude[20];
  char windUnit;
} user_settings = {};

enum ConnectionState { INITIAL, ACCESS_POINT, CONNECTED_TO_WIFI, CONNECTED_AND_SENSOR_NOT_SETUP, CONNECTED_AND_API_OK, CONNECTED_AND_API_NOT_OK };

ConnectionState connectionState = INITIAL;

long    millies         = 0;
double  mainResult      = -1.0; // MP2.5 or Windspeed
double  temp            = -50.0;
double  windDirection   = -1.0;
double  windGusts       = -1.0;

String  timeStamp; 
String  url;
int     refreshInterval;
String  progressString  = "";
bool    spiffsmounted   = false;

//String sensor = "84654";  // sensor.community sensor van Leon, Sietse is 36691
//String sensor = "38616";  // HL sensor 465 van Leon
//String sensor = "5813238" // Sensor.community local

void setup() {

  Serial.begin(9600);
  Serial.println();
  Serial.println("Initializing");

  Serial.print(F("Board Type: "));
  Serial.println(BoardIdentify::type); 
  Serial.print(F("Board Make: "));
  Serial.println(BoardIdentify::make); 
  Serial.print(F("Board Model: "));
  Serial.println(BoardIdentify::model); 
  Serial.print(F("Board MCU: "));
  Serial.println(BoardIdentify::mcu);

  u8g2.begin();

  drawText();

  strcpy(user_settings.latitude, "52.377114337541");
  strcpy(user_settings.longitude, "4.633415994184433");
  
  EEPROM.begin(sizeof(struct settings));
  EEPROM.get(0, user_settings);
   
  Serial.println("Connecting to AP");

  WiFi.setHostname(HOSTNAME);
  WiFi.hostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(user_settings.ssid, user_settings.password);

  //Maak een reset functie.
  //ESP.restart();
  
  byte tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (tries++ > 30) {
      Serial.println("Connecting to AP failed. Setting up access point 'Setup Leon's display'");

      WiFi.mode(WIFI_AP);
      WiFi.softAP("Setup Leon's display");
      Serial.println("Connected to AP");
      Serial.print("Listening on ip nr: ");
      Serial.println(WiFi.softAPIP().toString().c_str());
      break;
    }
  }

  server.on("/",          handlePortal);
  server.on("/setupwifi", handleSetupWifi);
  server.on("/settings",  handleSettings);
  server.on("/clear",     handleClear);
  server.on("/reset",     handleReset);
  server.on("/firmware", HTTP_GET, handleUpdateGet);
  server.on("/firmware", HTTP_POST, handlePostUpdate, handleFileUpload);

  OTAUpdateSetup();
  spiffsmounted = SPIFFS.begin();

  server.begin();

  Serial.println(WiFi.localIP());

  connectionState = getConnectionState();
  drawText();

  // Create url
  if (sensorType("sc")) {
    // sensor.community
    url = URL_SC + String(user_settings.sensornr) + "/";                                //  84654
    refreshInterval = ONE_MINUTE;
  } else if (sensorType("sl")) {
    // sensor.community
    url = URL_SL_PREFIX + String(user_settings.sensornr) + URL_SL_POSTFIX;      //  5813238
    refreshInterval = ONE_MINUTE;
  } else if (sensorType("hl")) {
    // Hollandse Luchten
    url = URL_HL_PREFIX + String(user_settings.sensornr) + URL_HL_POSTFIX;              //  38616
    refreshInterval = TEN_MINUTES;
  } else if (sensorType("hr")) {
    // Hollandse Luchten realtime
    url = URL_HR; // + String(user_settings.sensornr);
    refreshInterval = ONE_MINUTE;
  } else if (sensorType("om")) {
    url = URL_OM_PREFIX + 
    String("?latitude=") + String(user_settings.latitude) + String("&longitude=") + String(user_settings.longitude);
    switch(user_settings.windUnit) {
      case 'n':
        url += "&wind_speed_unit=kn";
        break;
      case 'm':
        url += "&wind_speed_unit=ms";
        break;
      case 'b':
        url += "&wind_speed_unit=ms";
        break;
      case 'k':
        // do nothing
        break;
      default:
        break;
    }
    url += URL_OM_POSTFIX;

    refreshInterval = TEN_MINUTES;
  }

  Serial.println();
  Serial.print("URL: ");
  Serial.println(url);

}

void loop() {

  ArduinoOTA.handle();
  yield();

  connectionState = getConnectionState();

  if (connectionState != ACCESS_POINT && connectionState != CONNECTED_AND_SENSOR_NOT_SETUP) {

    if (((millis() - millies) > refreshInterval ) || (mainResult == -1.0)) {

      if (WiFi.status() == WL_CONNECTED) {
    
        Serial.println(url);

        String json = retrieveData(url);
        Serial.println(json);
        if (json.length() > 10) {
          parseJson(json);
          if (mainResult > 0) {
            drawText();
          }
        } else if (json.equals("[]")) {
          connectionState = CONNECTED_AND_API_NOT_OK;
          Serial.println("Sensor is offline");
        } else {
          connectionState = CONNECTED_AND_API_NOT_OK;
          Serial.print("Error parsing json");
          Serial.println(json);
        }

        millies = millis();
      }
    }
  }

  server.handleClient();
  drawText();
}

String retrieveData(String url) {

  String result = "";
  bool succes = false;

  HTTPClient https;

  // wait for WiFi connection
  if (sensorType("sl")) {  

    // http
    WiFiClient client;
    succes = https.begin(client, url);
  } else {

    // https
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure(); // Note this is unsafe against MITM attacks
    succes = https.begin(*client, url);
  }

  if (succes) {  // HTTPS

    // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        Serial.println("Processing result...");
        Serial.printf("HttpCode: %d", httpCode);
        Serial.println();
        result = https.getString();
        Serial.println(result);
      } else {
        Serial.println("Error calling API...");
        Serial.printf("HttpCode: %d", httpCode);
        Serial.println();
        Serial.printf("[HTTPS] GET... failed, error: [%d] %s\n", httpCode, https.errorToString(httpCode).c_str());
        Serial.println();
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] GET... failed, error: [%d] %s\n", httpCode, https.errorToString(httpCode).c_str());
      Serial.println();
    }
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }
  return result;
}

void parseJson(String json) {

  Serial.println("Parsing json");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error && (error != DeserializationError::EmptyInput)) {
    Serial.println("Deserialization error");
    Serial.println(json);
    Serial.print(error.c_str());
    mainResult = -1.0;
  } else if (error == DeserializationError::EmptyInput) {
    Serial.println("Empty response");
    mainResult = -1.0;
  } else {

    mainResult = -1.0;
    temp = -50.0;

    if (sensorType("sc")) {

      for (JsonObject root : doc.as<JsonArray>()) {
      
        for (JsonObject sensordatavalue : root["sensordatavalues"].as<JsonArray>()) {
    
          String value      = sensordatavalue["value"];
          String value_type = sensordatavalue["value_type"];
    
          Serial.print(value_type);
          Serial.print(" : ");
          Serial.println(value);

          if (value_type.equals("P2") && mainResult == -1.0) {
            mainResult = value.toDouble();
          }
        }
        //timeStamp = root["timestamp"];
      }

    } else if (sensorType("sl")) {

      for (JsonObject sensordatavalue : doc["sensordatavalues"].as<JsonArray>()) {

        String value      = sensordatavalue["value"]; // "9.05", "1.65", "10.96", "100656.41", ...
        String value_type = sensordatavalue["value_type"]; // "SDS_P1", "SDS_P2", ...

        Serial.print(value_type);
        Serial.print(" : ");
        Serial.println(value);

        if (value_type.equals("SDS_P2") && mainResult == -1.0) {
          mainResult = value.toDouble();
        }

        if (value_type.equals("BME280_temperature") && temp == -50.0) {
          temp = value.toDouble();
        }
      }
    } else if (sensorType("hl")) {

      // Hollandse Luchten API
      JsonObject value_0 = doc["value"][0];
      mainResult = value_0["result"];
      //timeStamp = value_0["phenomenonTime"];
    } else if (sensorType("hr")) {

      // Hollandse Luchten API Real Time
      JsonObject value_0 = doc[0];

      mainResult = value_0["result"];
      //timeStamp = value_0["phenomenonTime"];
    } else if (sensorType("om")) {

      JsonObject meteo = doc["current"];

      mainResult = meteo["wind_speed_10m"];
      temp = meteo["temperature_2m"];
      windDirection = meteo["wind_direction_10m"];
      windGusts = meteo["wind_gusts_10m"];
    } else {
      Serial.print("Invalid value of sensortype: ");
      Serial.println(user_settings.sensortype);
    } 
  }  
}

/************************************************* POST/GET handlers ***********************************************************/

void handlePortal() {

  String htmlString;

  if (server.method() == HTTP_POST) {

    Serial.println("Submit /");

    Serial.println();
    Serial.println("Contents of Server parameters");
    Serial.print("SSID: ");
    Serial.println(server.arg("ssid").c_str());
    Serial.print("Password: ");
    Serial.println(server.arg("password").c_str());
    Serial.print("Sensortype: ");
    Serial.println(server.arg("sensortype").c_str());
    Serial.print("SensorNr: ");
    Serial.println(server.arg("sensornr").c_str());
    Serial.print("Latitude: ");
    Serial.println(server.arg("latitude").c_str());
    Serial.print("Longitude: ");
    Serial.println(server.arg("longitude").c_str());

    Serial.println();
    Serial.println("Contents of EEPROM");
    Serial.print("SSID: ");
    Serial.println(user_settings.ssid);
    Serial.print("Password: ");
    Serial.println(user_settings.password);
    Serial.print("Sensortype: ");
    Serial.println(user_settings.sensortype);
    Serial.print("SensorNr: ");
    Serial.println(user_settings.sensornr);
    Serial.print("Latitude: ");
    Serial.println(user_settings.latitude);
    Serial.print("Longitude: ");
    Serial.println(user_settings.longitude);

    htmlString = htmlPortalPost();
  } else {

    Serial.println("Get /");

    Serial.println();
    Serial.println("Contents of EEPROM");
    Serial.print("SSID: ");
    Serial.println(user_settings.ssid);
    Serial.print("Password: ");
    Serial.println(user_settings.password);
    Serial.print("Sensortype: ");
    Serial.println(user_settings.sensortype);
    Serial.print("SensorNr: ");
    Serial.println(user_settings.sensornr);
    Serial.print("Latitude: ");
    Serial.println(user_settings.latitude);
    Serial.print("Longitude: ");
    Serial.println(user_settings.longitude);

    htmlString = htmlPortalGet();
  }
  server.send(200,   "text/html", htmlString);
}

void handleSetupWifi() {

  String htmlString;

  if (server.method() == HTTP_POST) {
    Serial.println("handleSetupWifi POST");
    Serial.print("SSID: ");
    Serial.println(server.arg("ssid").c_str());
    Serial.print("Password: ");
    Serial.println(server.arg("password").c_str());

    strncpy(user_settings.ssid,     server.arg("ssid").c_str(),     sizeof(user_settings.ssid) );
    user_settings.ssid[server.arg("ssid").length()] = '\0';

    strncpy(user_settings.password, server.arg("password").c_str(), sizeof(user_settings.password) );
    user_settings.password[server.arg("password").length()] = '\0';

    EEPROM.put(0, user_settings);
    EEPROM.commit();

    htmlString = htmlSetupWifiPost();
  } else {
    Serial.println("handleSetupWifi GET");
    htmlString = htmlSetupWifiGet();
  }
  server.send(200,   "text/html", htmlString);
}

void handleSettings() {

  String htmlString;

  if (server.method() == HTTP_POST) {
    Serial.println("handleSettings POST");
    Serial.print("Sensortype: ");
    Serial.println(server.arg("sensortype").c_str());
    Serial.print("SensorNr: ");
    Serial.println(server.arg("sensornr").c_str());
    Serial.print("Latitude: ");
    Serial.println(server.arg("latitude").c_str());
    Serial.print("Longitude: ");
    Serial.println(server.arg("longitude").c_str());
    Serial.print("Unit: ");
    Serial.println(server.arg("windUnit").c_str());

    strncpy(user_settings.sensortype, server.arg("sensortype").c_str(), sizeof(user_settings.sensortype) );
    user_settings.sensortype[server.arg("sensortype").length()] = '\0';

    strncpy(user_settings.sensornr, server.arg("sensornr").c_str(), sizeof(user_settings.sensornr) );
    user_settings.sensornr[server.arg("sensornr").length()] = '\0';

    strncpy(user_settings.latitude, server.arg("latitude").c_str(), sizeof(user_settings.latitude) );
    user_settings.latitude[server.arg("latitude").length()] = '\0';

    strncpy(user_settings.longitude, server.arg("longitude").c_str(), sizeof(user_settings.longitude) );
    user_settings.longitude[server.arg("longitude").length()] = '\0';

//    strncpy(user_settings.longitude, server.arg("windUnit").c_str(), sizeof(user_settings.windUnit) );
    
    user_settings.windUnit = server.arg("windUnit").charAt(0);

    EEPROM.put(0, user_settings);
    EEPROM.commit();

    htmlString = htmlSensorSetupPost();
  } else {
    Serial.println("handleSettings GET");
    htmlString = htmlSensorSetupGet();
  }

  server.send(200,   "text/html", htmlString);
}

void handleClear() {

  String htmlString;

  if (server.method() == HTTP_POST) {

    Serial.println("handleClear POST");
    Serial.println();

    strncpy(user_settings.ssid, "\0",     sizeof(user_settings.ssid) );
    user_settings.ssid[0] = '\0';
    strncpy(user_settings.password, "\0", sizeof(user_settings.password) );
    user_settings.password[0] = '\0';
    strncpy(user_settings.sensortype, "\0", sizeof(user_settings.sensortype) );
    user_settings.sensortype[0] = '\0';
    strncpy(user_settings.sensornr, "\0", sizeof(user_settings.sensornr) );
    user_settings.sensornr[0] = '\0';
    strncpy(user_settings.latitude, "\0", sizeof(user_settings.latitude) );
    user_settings.latitude[0] = '\0';
    strncpy(user_settings.longitude, "\0", sizeof(user_settings.longitude) );
    user_settings.longitude[0] = '\0';
    user_settings.windUnit = '\0';

    EEPROM.put(0, user_settings);
    EEPROM.commit();

    Serial.println();
    Serial.println("Contents of EEPROM");
    Serial.print("SSID: ");
    Serial.println(user_settings.ssid);
    Serial.print("Password: ");
    Serial.println(user_settings.password);
    Serial.print("Sensortype: ");
    Serial.println(user_settings.sensortype);
    Serial.print("SensorNr: ");
    Serial.println(user_settings.sensornr);
    Serial.print("Longitude: ");
    Serial.println(user_settings.latitude);
    Serial.print("Latitude: ");
    Serial.println(user_settings.longitude);

    htmlString = htmlClearPost();

  } else {

    Serial.println("handleClear GET");

    Serial.println();
    Serial.println("Contents of EEPROM");
    Serial.print("SSID: ");
    Serial.println(user_settings.ssid);
    Serial.print("Password: ");
    Serial.println(user_settings.password);
    Serial.print("Sensortype: ");
    Serial.println(user_settings.sensortype);
    Serial.print("SensorNr: ");
    Serial.println(user_settings.sensornr);
    Serial.print("Latitude: ");
    Serial.println(user_settings.latitude);
    Serial.print("Longitude: ");
    Serial.println(user_settings.longitude);

    htmlString = htmlClearGet();
  }
  server.send(200,   "text/html", htmlString);
}

void handleReset() {

  String htmlString;

  if (server.method() == HTTP_POST) {

    htmlString = htmlPortalGet();

    server.client().setNoDelay(true);
    server.send(200,   "text/html", htmlString);
    delay(100);
    server.client().stop();
    ESP.restart();

  } else {

    Serial.println("handleReset GET");
    htmlString = htmlResetGet();
    server.send(200,   "text/html", htmlString);
  }
}

// GET '/'
String htmlPortalPost() {

  String htmlString;
  
  htmlString = HTML_HEADER;

  htmlString += "<main class='form-signin'>"
    "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlPortalPost</h2>";
  #endif

  htmlString += "<p>Your settings have been saved successfully!<br/>"
    "Please restart the device.</p>"
    "<p>SSID: ";
    htmlString+= user_settings.ssid;
    htmlString+= "</p><p>Password: ";
    htmlString+= user_settings.password;
    htmlString+= "</p><p>Sensor type: ";
    htmlString+= user_settings.sensortype;
    htmlString+= "</p><p>Sensor NR: ";
    htmlString+= user_settings.sensornr;
    htmlString+= "</p><p>Latitude: ";
    htmlString+= user_settings.latitude;
    htmlString+= "</p><p>Longitude: ";
    htmlString+= user_settings.longitude;
    htmlString+= "</p>";
    htmlString += HTML_FOOTER;

  return htmlString;
}

String htmlPortalGet() {
  String htmlString = HTML_HEADER;
  htmlString += "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlPortalGet</h2>";
  //  htmlString += "<p>Board: ";
  //  htmlString += BOARD;
  //  htmlString += "</p>";
  #endif

  String sensorType;

  if (sensorType("sc")) {
    sensorType = "Sensor Community (Luftdaten), server API";
  } else if (sensorType("sl")) {
    sensorType = "Sensor Community (Luftdaten), lokale API";
  } else if (sensorType("hl")) {
    sensorType = "Sodaq Air van Hollandse Luchten";
  } else if (sensorType("hr")) {
    sensorType = "Sodaq Air van Hollandse Luchten (Real Time)";
  } else if (sensorType("om")) {
    sensorType = "Open Meteo (Temperature/Wind/Wind direction/Gusts)";
  } else {
    sensorType = "*** unknown sensor type ***";
  }

  if (connectionState == ACCESS_POINT) {
      htmlString += "<p>Setup Wifi first. Set up your Wifi connection with the 'Set up Wifi' button</p>";
  } else if (connectionState == CONNECTED_TO_WIFI) {
      htmlString += "<p>Setup not completed. Set up your sensor with the 'Settings' button</p>";
  } else if (connectionState == CONNECTED_AND_API_OK) {
    if (mainResult == -1.0) {
      htmlString += "<p>Error. data not initialized</p>";
    } else {
      if (sensorType("om")) {
        htmlString += "<p>Source: ";
        htmlString += sensorType;
        htmlString += "</p>";
        htmlString += "<p><b>Wind Speed:</b></p>";
        htmlString += "</p><p font style='font-size:60px'>";
        htmlString += mainResult;
        htmlString +=  " <sup>km</sup>/<sub>h</sub></p>";
        htmlString +=  "<p>Temperature: ";
        htmlString +=  temp;
        htmlString +=  "</p>";
        htmlString +=  "<p>Direction: ";
        htmlString +=  degreesToCompass(windDirection);
        htmlString +=  "</p>";
        htmlString +=  "<p>Gusts: ";
        htmlString +=  windGusts;
        htmlString +=  "</p>";

      } else {
        htmlString += "<p>Sensor: ";
        htmlString += sensorType;
        htmlString += " ";
        htmlString += user_settings.sensornr;
        htmlString += "</p>";
        htmlString += "<p><b>PM 2.5 value:</b></p>";
        htmlString += "</p><p font style='font-size:60px'>";
        htmlString += mainResult;
        htmlString +=  " &micro;g/m<sup>3</sup></p>";
      }
    }
  }

  htmlString += "<br/>"
    "<form action='/setupwifi' method='get'><button type='submit'>Set up Wifi</button></form>"
    "<form action='/settings'  method='get'><button type='submit'>Set up sensor</button></form>"
    "<form action='/clear'     method='get'><button type='submit'>Clear data</button></form>"
    "<form action='/firmware'  method='get'><button type='submit'>Update firmware</button></form>"
    "<form action='/reset'     method='get'><button type='submit'>Reset</button></form>";

    htmlString += HTML_FOOTER;

  return htmlString;
}

String htmlSetupWifiPost() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString += "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlSetupWifiPost</h2>";
  #endif

  htmlString += "<form action='/reset' method='get'>"
    "<p>WiFi settings have been changed, reset the device</p>"
    "<table><tr><td>SSID:</td><td>";
  htmlString += user_settings.ssid;
  htmlString += "</td></tr><tr><td>Password:</td><td>";
  htmlString += user_settings.password;
  htmlString += "</td></tr></table>"
    "<br/>"
    "<button type='submit'>Reset</button>"
    "</form>";
  htmlString += "<form action='/' method='get'><button type='submit'>Home</button></form>"
      "</body>"
      "</html>";

  Serial.println(htmlString);

  return htmlString;
}

String htmlSetupWifiGet() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString +=
    "<main class='form-signin'>"
    "<form action='/setupwifi' method='post'>"
    "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlSetupWifiGet</h2>";
  #endif

  htmlString += "<table><tr><td>SSID</td><td>"
    "<input type = 'text' name='ssid' id='ssid' value = '";
  htmlString += user_settings.ssid;
  htmlString += "'></td></tr>"
    "<tr><td><label>Password</label></td><td><input type='text' name='password' value='";
  htmlString += user_settings.password;
  htmlString +=  "'></td></tr>"
    "</table>"
    "<br/><br/>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<form action='/' method='get'><button type='submit'>Home</button></form>"
    "</body>"
    "</html>";

  return htmlString;
}

String htmlSensorSetupPost() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString +=
    "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlSensorSetupPost</h2>";
  #endif

  htmlString += "<table><tr><td>Sensor Type:</td><td>";
  htmlString +=  user_settings.sensortype;
  htmlString += "</td></tr><tr><td>Sensor nr.</td><td>";
  htmlString += user_settings.sensornr;
  htmlString += "</td></tr></table>";
  htmlString += "<p>Reset the device to enable the new settings</p>"
    "<form action='/reset' method='get'><button type='submit'>Reset</button></form>"
    "<form action='/' method='get'><button type='submit'>Home</button></form>"
    "</body>"
    "</html>";

  return htmlString;
}

String htmlSensorSetupGet() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString +=
    "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlSensorSetupGet</h2>";
  #endif

  htmlString += "<form action='/settings' method='POST'>"
    "<table><tr><td>"
    "<label>Sensor Type</label></td><td>";

  htmlString += "<input type='radio' id='hl' name='sensortype' value='hl' ";
  if (sensorType("hl")) {
    htmlString += "checked >";
  } else {
    htmlString += ">";
  }
  htmlString +=  "<label for='hl'>Hollandse Luchten (Sodaq)</label><br>";

  htmlString += "<input type='radio' id='hr' name='sensortype' value='hr' ";
  if (sensorType("hr")) {
    htmlString += "checked >";
  } else {
    htmlString += ">";
  }
  htmlString +=  "<label for='hr'>Hollandse Luchten (Realtime)</label><br>";

  htmlString +=  "<input type='radio' id='sc' name='sensortype' value='sc'";
  if (sensorType("sc")) {
    htmlString += "checked >";
  } else {
    htmlString += ">";
  }
  htmlString += "<label for='sc'>Sensor Community</label><br>";

  htmlString +=  "<input type='radio' id='sl' name='sensortype' value='sl'";
  if (sensorType("sl")) {
    htmlString += "checked >";
  } else {
    htmlString += ">";
  }
  htmlString += "<label for='sl'>Sensor Community (Local)</label><br>";

  htmlString +=  "<input type='radio' id='om' name='sensortype' value='om'";
  if (sensorType("om")) {
    htmlString += "checked >";
  } else {
    htmlString += ">";
  }
  htmlString += "<label for='om'>Open Meteo Temperature and Wind</label><br>";

  htmlString += "</td></tr>"
    "<tr><td><label>Sensor name/number</label></td>"
    "<td><input type='text' name='sensornr' value='";
  htmlString += user_settings.sensornr;
  htmlString += "'";
  htmlString += "></td></tr>";

  htmlString += "<tr><td><label>Latitude</label></td>"
    "<td><input type='text' name='latitude' value='";
  htmlString += user_settings.latitude;
  htmlString += "'></td></tr>"
    "<tr><td><label>Longitude</label></td>"
    "<td><input type='text' name='longitude' value='";
  htmlString += user_settings.longitude;
  htmlString += "'></td></tr><tr>";

  htmlString += "<tr><td><label>Wind speed unit</label></td><td>";
  htmlString += "<input type ='radio' id='k' name='windUnit' value='k'";
  if (user_settings.windUnit == 'k') {
    htmlString += " checked ";
  }
  htmlString += ">";
  htmlString += "<label for='k'><sup>km</sup>/<sub>h</sub></label>";

  htmlString += "&nbsp;<input type ='radio' id='m' name='windUnit' value='m'";
  if (user_settings.windUnit == 'm') {
    htmlString += " checked ";
  }
  htmlString += ">";
  htmlString += "<label for='m'><sup>m</sup>/<sub>s</sub></label>";

  htmlString += "&nbsp;<input type ='radio' id='b' name='windUnit' value='b'";
  if (user_settings.windUnit == 'b') {
    htmlString += " checked ";
  }
  htmlString += ">";
  htmlString += "<label for='b'>Bft</label>";
  
  htmlString += "&nbsp;<input type ='radio' id='n' name='windUnit' value='n'";
  if (user_settings.windUnit == 'n') {
    htmlString += " checked ";
  }
  htmlString += ">";
  htmlString += "<label for='n'>Knots</label>";
  
  htmlString += "</td></tr>"
  "</table>"
    "<br/><br/>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<form action='/' method='get'><button type='submit'>Home</button></form>"
    "</body>"
    "</html>";

  return htmlString;
}

String htmlClearPost() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString += "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlClearPost</h2>";
  #endif

  htmlString += "<br/>"
    "<p>Your settings have been cleared successfully!<br/>"
    "Please reset the device.</p>"
    "<form action='/reset' method='get'><button type='submit'>Reset</button></form>"
    "<form action='/' method='get'><button type='submit'>Home</button></form>"
    "</body>"
    "</html>";

  return htmlString;
}

String htmlClearGet() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString +=
    "<form action='/clear' method='post'>"
    "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlClearGet</h2>";
  #endif

  htmlString += "<table><tr><td>SSID</td><td>";
  htmlString += user_settings.ssid;
  htmlString += "</td></tr>";
  htmlString += "<tr><td>Sensor Type</td><td>";
  htmlString += user_settings.sensortype;
  htmlString += "</td></tr><tr><td>Sensor name/number</td><td halign='left'>";
  htmlString += user_settings.sensornr;
  htmlString += "<tr><td>Latitude</td><td>";
  htmlString += user_settings.latitude;
  htmlString += "</td></tr>";
  htmlString += "<tr><td>Longitude</td><td>";
  htmlString += user_settings.longitude;
  htmlString += "</td></tr></table><br/><br/>"
    "<button type='submit'>Clear All Settings</button>"
    "</form>";
  htmlString += HTML_FOOTER;

  return htmlString;
}

/*
String htmlResetPost() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString += "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlResetPost</h2>";
  #endif

  htmlString += "<br/>"
    "<p>Your settings have been cleared successfully!<br/>"
    "Please restart the device.</p>";
    htmlString += HTML_FOOTER;

  return htmlString;
}
*/

String htmlResetGet() {

  String htmlString;

  htmlString = HTML_HEADER;

  htmlString +=
    "<form action='/reset' method='post'>"
    "<h1>Leon's display Setup</h1>";

  #ifdef DEBUG
    htmlString += "<h2>htmlResetGet</h2>";
  #endif

  htmlString += "<p>Pressing 'Reset' will reset the device. Wait until the display shows data again.</p>"
    "<br/><br/>"
    "<button type='submit'>Reset</button>"
    "</form>"
    "<form action='/' method='get'><button type='submit'>Home</button></form>"
    "</body>"
    "</html>";

  return htmlString;
}

// send available WiFi networks to client
String getWiFiNetworks() {
 
  String response = "";
 
  for (int i = 0; i < WiFi.scanNetworks(); ++i) {
    response += "<option value='" + WiFi.SSID(i) + "'";
    if (strcmp(user_settings.ssid, WiFi.SSID(i).c_str()) == 0) {
      response += " selected ";
    }
    response += ">" + WiFi.SSID(i) + "</option>";
  }
  return response;
}

ConnectionState getConnectionState() {

  if (WiFi.getMode() == WIFI_AP) {
    return ACCESS_POINT;
  } else if ((WiFi.getMode() == WIFI_STA) && (WiFi.status() == WL_CONNECTED)) {
    if (strlen(user_settings.sensortype) > 0 && strlen(user_settings.sensornr) > 0) {
      return CONNECTED_AND_API_OK;
    } else {
      return CONNECTED_AND_SENSOR_NOT_SETUP;
    }
  }

  return INITIAL;
}

void htmlReserve() {

  String htmlString;

  htmlString =
    "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>Leon's display</title>"
    "</head>"
    "<body>"
    "<h1>Leon's display Setup</h1>"
    "<br/>"
    "<label for 'ssid'>WiFi network</label>"
    "<select name='ssid' id='ssid'>";

  htmlString += getWiFiNetworks();
  htmlString += "</select>"
    "<br/>"
    "<label>Password</label><input type='text' name='password' value='";
  htmlString += user_settings.password;
  htmlString +=  "'>"
    "<br/><br/>"
    "<label>Sensor Type</label><br/>";
  htmlString +=  "<input type='radio' id='hl' name='sensortype' value='hl' ";
  if (sensorType("hl")) {
    htmlString += "checked >";
  } else {
    htmlString += ">";
  }
  htmlString +=  "<label for='hl'>Hollandse Luchten (Sodaq)</label><br>";
  htmlString +=  "<input type='radio' id='sc' name='sensortype' value='sc'";
  if (sensorType("sc")) {
    htmlString += "checked >";
  } else {
    htmlString += ">";
  }
  htmlString +=  "<label for='sc'>Sensor Community</label><br>"
    "<br/>"
    "<label>Sensor name/number</label>"
    "<input type='text' name='sensornr' value='";
  htmlString += user_settings.sensornr;
  htmlString += "'>"
    "<br/><br/>"
    "<button type='submit'>Save</button>"
    "</form>";
  htmlString += HTML_FOOTER;
}

/******************************************** Display support *******************************************************************/
void drawText() {
  
  u8g2.clearBuffer();

  char header[] = "Leon's display v.";
  strcat(header, VERSION);

  if (connectionState == INITIAL) {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawUTF8(0, 10, header);
    u8g2.drawStr(20, 30, "Initializing...");
  } else if (connectionState == ACCESS_POINT) {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawUTF8(0, 10, header);
    u8g2.drawStr(30, 30, "Not connected");
    u8g2.drawStr(0,43, "Connect to AP and ip:");
    u8g2.drawStr(20,60, WiFi.softAPIP().toString().c_str());
  } else if (connectionState == CONNECTED_AND_SENSOR_NOT_SETUP) {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawUTF8(0, 10, header);
    u8g2.drawStr( 0,30, "Connected to network");
    u8g2.drawStr( 0,43, "Complete sensor setup on:");
    u8g2.drawStr(20,60, WiFi.localIP().toString().c_str());
  } else if ((connectionState == CONNECTED_AND_API_OK) || (connectionState == CONNECTED_AND_API_NOT_OK)) {
    if (mainResult > 0.0) {
      if (!sensorType("om")) {
        u8g2.setFont(u8g2_font_6x12_tf);
        u8g2.drawUTF8(0, 10, "PM2.5");

        String textPm = String(mainResult, 1);
        if (mainResult < 10) {
          textPm = String(mainResult, 2);  
        }
        if (mainResult >= 100) {
          textPm = String(mainResult, 0);
        }

        u8g2.setFont(u8g2_font_fub35_tf);
        u8g2.drawUTF8(10,60, textPm.c_str());

        u8g2.setFont(u8g2_font_6x12_tf);
        if (temp > -50.0) {
          String textTemp = String(temp, 1) + "°C";
          if (temp < 10) {
            textTemp = String(temp,2) + "°C";  
          }
          u8g2.drawUTF8(80,10, textTemp.c_str());
        } else {
          // No temperature
          u8g2.drawUTF8(80, 10, "[µg/m³]"); // (String(user_settings.sensortype) + "-" + String(user_settings.sensornr)).c_str());
        }

        if (CONNECTED_AND_API_OK) {
          u8g2.drawUTF8(25,0, "ᯤ");
        } else {
          u8g2.drawUTF8(25,0, "❌");
        }
      } else {
        // Windrichting en sterkte
        u8g2.setFont(u8g2_font_6x12_tf);
        switch (user_settings.windUnit) {
          case 'k': u8g2.drawStr(0, 10, "Wind [km/h]");
            break;
          case 'm': u8g2.drawStr(0, 10, "Wind [m/s]");
            break;
          case 'b': u8g2.drawStr(0, 10, "Wind [Bft]");
            break;
          default: u8g2.drawStr(0, 10, "Wind [km/h]");
            break;
        }
        if (temp > -50.0) {
          String textTemp = String(temp, 1) + "°C";
          if (temp < 10) {
            textTemp = String(temp,2) + "°C";  
          }
          u8g2.drawUTF8(80,10, textTemp.c_str());
        } else {
          u8g2.drawStr( 0,25, "Retrieving data for:");
        }
        u8g2.setFont(u8g2_font_fub25_tf);
        if (user_settings.windUnit == 'b') {
          u8g2.drawStr( 15,45, String(msToBft(mainResult)).c_str());
        } else {
          u8g2.drawStr( 15,45, String(mainResult, 0).c_str());
        }
        u8g2.setFont(u8g2_font_6x12_tf);
        if (user_settings.windUnit == 'b') {
          u8g2.drawStr( 0,64, ("Gusts: " + String(msToBft(windGusts))).c_str());
        } else {
          u8g2.drawStr( 0,64, ("Gusts: " + String(windGusts, 0)).c_str());
        }
        #ifdef DEBUG
          u8g2.drawUTF8(65,64, (String(windDirection, 0) + "°").c_str());
        #endif
        u8g2.drawStr(97, 64, degreesToCompass(windDirection).c_str());

        int centerX = 100;
        int centerY =  32;
        int arc     =  16;

/*
        float x1=sin(windDirection * M_PI/180.0);
        float y1=cos(windDirection * M_PI/180.0);

        u8g2_uint_t fromX = centerX - arc * x1;
        u8g2_uint_t fromY = centerY + arc * y1;
        u8g2_uint_t toX   = centerX + arc * x1;
        u8g2_uint_t toY   = centerY - arc * y1;

        u8g2.drawLine(fromX, fromY, toX, toY);
*/

        float xs = sin(windDirection * M_PI/180.0);
        float ys = cos(windDirection * M_PI/180.0);

        u8g2_uint_t fromX = centerX - arc * xs;
        u8g2_uint_t fromY = centerY + arc * ys;

        float xe = sin(windDirection * M_PI/180.0);
        float ye = cos(windDirection * M_PI/180.0);

        u8g2_uint_t toX   = centerX + arc * xe;
        u8g2_uint_t toY   = centerY - arc * ye;

        float x2 = sin((windDirection + 40) * M_PI/180.0);
        float y2 = cos((windDirection + 40) * M_PI/180.0);

        u8g2_uint_t toX1   = centerX + (arc/3) * x2;
        u8g2_uint_t toY1   = centerY - (arc/3) * y2;

        float x3 = sin((windDirection - 40) * M_PI/180.0);
        float y3 = cos((windDirection - 40) * M_PI/180.0);

        u8g2_uint_t toX2   = centerX + (arc/3) * x3;
        u8g2_uint_t toY2   = centerY - (arc/3) * y3;

        u8g2.drawTriangle(fromX, fromY, toX1, toY1, toX2, toY2);
        u8g2.drawLine(fromX, fromY, toX, toY);
      }
    } else {
      if (sensorType("om")) {
        u8g2.drawStr(35,40, "Open Meteo data");
      } else {
        u8g2.drawStr(35,40, (String(user_settings.sensortype) + "-" + String(user_settings.sensornr)).c_str());
      }
      u8g2.drawStr(10,55, ("ip: " + WiFi.localIP().toString()).c_str());
      progressString += ".";
      if (progressString.length() > 21) {
        progressString = ".";
      }
      u8g2.drawStr(0,65, progressString.c_str());
      //u8g2.drawLine(0,65, progressString.c_str());

    }
  }
  u8g2.sendBuffer();
}

void drawRotatedBitmap(int16_t x, int16_t y, 
                       const uint8_t *bitmap, uint16_t angle)
{
  uint8_t w = 16;
  uint8_t h = 16;

  int16_t newx, newy;
  uint8_t data = 0;
  float cosa = cos(angle * DEG2RAD);
  float sina = sin(angle * DEG2RAD);

  x = x - ((w * cosa / 2) - (h * sina / 2));
  y = y - ((h * cosa / 2) + (w * sina / 2));
  for (int16_t j = 0; j < h; j++) {
    for (int16_t i = 0; i < w; i++) {
      if ((j * w + i) & 7) {
        data <<= 1;
      } else {
        data = pgm_read_byte(bitmap++);
      }

      newx = 0.5 + x + ((i * cosa) - (j * sina));
      newy = 0.5 + y + ((j * cosa) + (i * sina));
      if (data & 0x80) {
//        u8g2.display()->setPixel(newx, newy);
        u8g2.drawPixel(newx, newy);
      } else {
//        u8g2.drawPixel(newx, newy, 0);
      }
    }
  }
}

/********************************************** OTA Setup *********************************************************************************/

void OTAUpdateSetup() {
  
  Serial.print("MDNS responder started, type ");
  Serial.print (HOSTNAME);
  Serial.println(".local/ in your browser"); 
                                  
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {

    } else { // U_SPIFFS
      type = "filesystem";
      if (spiffsmounted) {
        SPIFFS.end();
        spiffsmounted = false;
      }
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    //  Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    if (!spiffsmounted) spiffsmounted = SPIFFS.begin();
    delay(1000);
    // Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //  Serial.printf("Progress: % u % % \r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[ % u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //  Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void handleUpdateGet() {

  String htmlString = HTML_HEADER;
  htmlString += "<h1>OTA Firmware Update</h1>";
  #ifdef DEBUG
    htmlString += "<h2>handleUpdateGet</h2>";
  #endif

  htmlString += "<p>Current version ";
  htmlString += VERSION;
  htmlString += "</p>";

  htmlString += "<p><a href='";
  htmlString += UPDATE_PATH;
  htmlString += "' target = '_blank'>";
  htmlString += "Look for updates</a></p>";

  htmlString += "<pre><form method='post' action='\' enctype='multipart/form-data'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'></form></pre>"
    "<form action='/' method='get'><button type='submit'>Home</button></form>";

  server.send(200, "text/html", htmlString);
}

void handlePostUpdate() {

  if (Update.hasError()) {
    StreamString str;
    Update.printError(str);
    str;
    String s = HTML_HEADER;
    s += "<h1>Update Error </h1>";
    s += str;
    s += HTML_FOOTER;
    server.send(200, "text / html", s);
  } else {
    String s = "";
    s += HTML_HEADER;
    s += "<META http-equiv=\"refresh\" content=\"30;URL=/\">Update Success ! Rebooting...\n";
    s += HTML_FOOTER;
    server.client().setNoDelay(true);
    server.send(200, "text / html", s);
    delay(100);
    server.client().stop();
    ESP.restart();
  }
}

void handleFileUpload() {

  HTTPUpload& upload = server.upload();
  String updateerror = "";
  if (upload.status == UPLOAD_FILE_START) {
    WiFiUDP::stopAll();
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) { //start with max available size
      StreamString str;
      Update.printError(str);
      updateerror = str;
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      StreamString str;
      Update.printError(str);
      updateerror = str;
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { //true to set the size to the current progress
      StreamString str;
      Update.printError(str);
      updateerror = str;
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
      Update.end();
    }
    yield();
  }
}

/*
Windstreek	Graden o.o.	Graden w.o.
 (standaard)	 (standaard)	 (afwijkend)
 N	 0 of 360
 NNO 22,5
 NO	 45
 ONO 67,5
 O	 90
 OZO 112,5
 ZO	 135
 ZZO 157,5
 Z	 180
 ZZW 202,5
 ZW	 225
 WZW 247,5
 W	 270
 WNW 292,5
 NW	 315
 NNW 337,5 
*/

String degreesToCompass(double degrees) {

  String compass[] = {"N  ", "NNE", "NE ", "ENE", "E  ", "ESE", "SE ", "SSE", "S  ", "SSW", "SW ", "WSW", "W  ", "WNW", "NW ", "NNW" }; 
  return compass[ int((int(degrees + 11.25) % 360)  / 22.5) ];
}

int msToBft(double metersPerSecond) {
	if (metersPerSecond <= 0.2) {
		return 0;
	} else if (metersPerSecond <= 1.5) {
		return 1;
	} else if (metersPerSecond <= 3.3) {
		return 2;
	} else if (metersPerSecond <= 5.4) {
		return 3;
	} else if (metersPerSecond <= 7.9) {
		return 4;
	} else if (metersPerSecond <= 10.7) {
		return 5;
	} else if (metersPerSecond <= 13.8) {
		return 6;
	} else if (metersPerSecond <= 17.1) {
		return 7;
	} else if (metersPerSecond <= 20.7) {
		return 8;
	} else if (metersPerSecond <= 24,4) {
		return 9;
	} else if (metersPerSecond <= 28.4) {
		return 10;
	} else if (metersPerSecond <= 32.6) {
		return 11;
	} else {
		return 12;
	}

	return -1;
}


/**************************


<!DOCTYPE html>
<html lang='EN'>
<head><meta charset='utf-8'/>
<title>Current data</title>
<meta name='viewport' content='width=device-width'/>
<link rel='stylesheet' href='/EN_s1?r=css'>
</style>
</head>
<body>
<div class='canvas'>
<a class='b' href='/' style='background:none;display:inline'>
<img src='/EN_s1?r=logo' alt='Back to home page' style='float:left;margin:16px' width='100' height='89'/>
</a>
<h3 style='margin:0 10px'>Particulate matter sensor</h3>
<br/>
<small style='color:#fff;font-weight:700'>ID: 5813238 (08f9e058b3f6)<br/>
Firmware version: NRZ-2020-133/EN&nbsp;(Nov 29 2020)<br/></small>
</div><div class='content'>
<h4>Home &raquo; Current data</h4>
<b>34 seconds since last measurement.</b>
<br/><br/><table cellspacing='0' cellpadding='5' class='v'>
<thead>
  <tr><th>Sensor</th><th> Parameter</th><th>Value</th></tr>
</thead>
<tr><td>SDS011</td><td>PM2.5</td><td class='r'>1.3&nbsp;µg/m³</td></tr>
<tr><td>SDS011</td><td>PM10</td><td class='r'>6.8&nbsp;µg/m³</td></tr>
<tr><td colspan='3'>&nbsp;</td></tr>
<tr><td>WiFi</td><td>signal strength</td><td class='r'>-65&nbsp;dBm</td></tr>
<tr><td>WiFi</td><td>signal quality</td><td class='r'>70&nbsp;%</td></tr>
</table>
<br/>
<br/>
<br/>
<a class='b' href='/' style='display:inline;'>Back to home page</a>
<br/><br/><br/></div>
<footer class='footer'><div style='padding:16px'>
<a href='https://codefor.de/stuttgart/' target='_blank' rel='noreferrer' style='color:#3ba;'>&copy; Open Knowledge Lab Stuttgart a.o. (Code for Germany)</a>&nbsp;&nbsp(<a href='https://github.com/opendata-stuttgart/sensors-software/labels/bug' target='_blank' rel='noreferrer'>Report an issue</a>)
</div>
</footer>
</body>
</html>

*/