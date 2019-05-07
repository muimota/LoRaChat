/*********
 * 2019 Martin Nadal
 * with portions of:
  Rui SantosComplete project details at https://randomnerdtutorials.com  
*********/

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#include <U8x8lib.h>
#include <LoRa.h>

//LORA Parameters
#define SS      18
#define RST     14
#define DI0     26
#define BAND    915E6

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];
U8X8LOG u8x8log;

unsigned long lastTime;
int counter;
String message;

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.print(client->id());
    Serial.println(" disconnected");
 
  } else if(type == WS_EVT_DATA){
 
    u8x8log.print(client->id());
    u8x8log.print("->");
    for(int i=0; i < len; i++) {
          u8x8log.print((char) data[i]);
    }
    u8x8log.println();
    
    server->textAll(data,len);
  }
}

AsyncWebSocket ws("/msg");
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
  
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  //screen
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8x8log.setRedrawMode(1);    // 0: Update screen with newline, 1: Update screen for every char  

  u8x8log.print("LoRaChat (c)2019\n");
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  String ssid("LoRa_" + String((uint16_t) (ESP.getEfuseMac() >> 32),HEX)); 
  const char password[] = "";
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid.c_str(), password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
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
 
}
 
void loop(){
}
