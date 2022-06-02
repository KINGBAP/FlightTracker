#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

WiFiManager wifiManager;

String complete;
String input;
String output;
String temp;

//const char* ssid = "DECANAT.STUDENT";
//const char* password = "studentfiir";
//const char* ssid = "Vodafone-B49C";
//const char* password = "znRC75KxsnukkjEb";

//Your Domain name with URL path or IP address with path
//const char* serverName = "http://aviation-edge.com/v2/public/flights?key=457ebb-a5208b&flightIata=AH1011";
String serverName = "https://aviation-edge.com/v2/public/flights?key=457ebb-a5208b&flightIata=";
//String serverName2 = "https://aviation-edge.com/v2/public/nearby?key=457ebb-a5208b&lat=44.4&lng=26.1&distance=100";
String airLabs = "https://airlabs.co/api/v9/airports?iata_code=";
String api_key = "&api_key=17fe0e12-1e56-4477-93f5-0d53df330ddb";
String flightNumber = "AH1005";
int error =0;

String sensorReadings;
String aeroport1, aeroport2;

const byte numChars = 400;
char receivedChars[numChars];
boolean newData = false;



void setup() {
  /*Serial.begin(9600);

    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    Serial.println("");*/
  Serial.begin(9600);
  wifiManager.autoConnect();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}


void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    recvWithEndMarker();
    showNewData();
    complete=serverName+flightNumber;
    complete.replace(" ","");
    complete=complete.substring(0,complete.length()-2);
    sensorReadings = httpGETRequest(complete);
    // Allocate JsonBuffer
    // Use arduinojson.org/assistant to compute the capacity.
    const size_t capacity = JSON_ARRAY_SIZE(1) + 4 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(9);
    DynamicJsonBuffer jsonBuffer(capacity);

    // Parse JSON object
    JsonObject& root = jsonBuffer.parseObject(sensorReadings);

    if (!root.success()) {
      if (error == 0) {
        error = 2;
      }
      Serial.println(F("Parsing failed!"));
      
      delay(2000);
      return;
    }

    String depart = root["departure"]["iataCode"].as<String>() ;
    String arrivee = root["arrival"]["iataCode"].as<String>() ;
    float lat3 = root["geography"]["latitude"].as<float>() ;
    float lon3 = root["geography"]["longitude"].as<float>() ;
    aeroport1 = httpGETRequest(airLabs + depart + api_key);

    const size_t capacity2 = JSON_ARRAY_SIZE(1) + 2 * JSON_OBJECT_SIZE(0) + JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 2 * JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(12);
    DynamicJsonBuffer jsonBuffer2(capacity2);

    // Parse JSON object
    JsonObject& root2 = jsonBuffer2.parseObject(aeroport1);

    if (!root2.success()) {
      if (error == 0 || error == 2) {
        error = 3;
      }
     Serial.println(F("Parsing failed!"));
      delay(2000);
      return;
    }

    float lat1 = root2["response"]["lat"].as<float>() ;
    float lon1 = root2["response"]["lng"].as<float>() ;

    aeroport2 = httpGETRequest(airLabs + arrivee + api_key);
    JsonObject& root3 = jsonBuffer2.parseObject(aeroport2);

    float lat2 = root3["response"]["lat"].as<float>() ;
    float lon2 = root3["response"]["lng"].as<float>() ;
    String city= root3["request"]["client"]["geo"]["city"].as<String>() ;


    float distance_tot = HaverSine(lat1, lon1, lat2, lon2);
    float distance_res = HaverSine(lat3, lon3, lat2, lon2);


    float altitude = root["geography"]["altitude"].as<float>() ;
    float speedH = root["speed"]["horizontal"].as<float>();

    displaySerial(distance_tot, distance_res,arrivee,depart,speedH,altitude,flightNumber,error,city);
  }

  else {
    Serial.println("WiFi Disconnected");
  }
  delay(10000);
}



String httpGETRequest(String url) {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  // Your IP address with path or Domain name with URL path
  http.begin(client, url);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    error = 0;
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
    //if (payload[0] = '[') {
    payload.replace("[", "");
    payload.replace("]", "");
    // }
    //Serial.println(payload);
  }
  else {
    error = 1;
    //Serial.print("Error code: ");
    //Serial.println(httpResponseCode);
    
    
  }
  // Free resources
   Serial.println(url);
  http.end();

  return payload;
 
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  // if (Serial.available() > 0) {
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
      
    }
  }
}

void showNewData() {
  if (newData == true) {
    //serverName = "https://aviation-edge.com/v2/public/flights?key=457ebb-a5208b&flightIata=";
    //serverName = serverName + receivedChars;
    //serverName = serverName.substring(0, serverName.length() - 1);
    Serial.print("received :");
    flightNumber=receivedChars;
    //flightNumber=flightNumber.substring(0,flightNumber.length()-2);
    Serial.println(flightNumber);
   // flightNumber.replace(" ","");
    
    
    
    newData = false;
  }
}

float HaverSine(float lat1, float lon1, float lat2, float lon2)
{
  float ToRad = PI / 180.0;
  float R = 6371;   // radius earth in Km

  float dLat = (lat2 - lat1) * ToRad;
  float dLon = (lon2 - lon1) * ToRad;

  float a = sin(dLat / 2) * sin(dLat / 2) +
            cos(lat1 * ToRad) * cos(lat2 * ToRad) *
            sin(dLon / 2) * sin(dLon / 2);

  float c = 2 * atan2(sqrt(a), sqrt(1 - a));

  float d = R * c;
  return d;
}

void displaySerial(float dt,float dr, String arr, String dep, float speedH, float alt, String flightNumber, int error,String city) {
   float pourcent = (dr / dt) * 100;
  //temp = "G,"+String(pourcent)+","+dep+","+arr+","+String(speedH)+","+String(alt)+","+flightNumber+","+city;

  if (error == 0 ) {
   
   output = "G,"+String(pourcent)+","+dep+","+arr+","+String(speedH)+","+String(alt)+","+flightNumber+","+city;
   Serial.println(output);
  }
  else if (error !=0) {
    Serial.println(error);
  }

   
  
  }
