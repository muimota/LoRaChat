/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

// Replace with your network credentials
const char* ssid = "LoRaChat";
const char* password = "";

unsigned long lastTime;
int counter;
String message;

// Set LED GPIO
const int ledPin = 2;
// Stores LED state
String ledState;

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "MESSAGE"){
    return message;
  }
  return String();
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.print(client->id());
    Serial.println(" disconnected");
 
  } else if(type == WS_EVT_DATA){
 
    Serial.print(client->id());
    Serial.print("->");
    for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
    }
    Serial.println();
    server->textAll(data,len);
  }
}

AsyncWebSocket ws("/msg");
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to javascript file
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  });
  // Route to javascript file
  server.on("/frontend.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/frontend.js", "text/javascript");
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  
  // Start server
  server.begin();
  
  
  lastTime = millis();
}
 
void loop(){
}
