/*********
 * 2019 Martin Nadal 
 * with portions of:
  Rui SantosComplete https://randomnerdtutorials.com/esp32-web-server-spiffs-spi-flash-file-system/ 
  Martin Sikora https://medium.com/@martin.sikora/node-js-websocket-simple-chat-tutorial-2def3a841b61
  
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

int counter;
String message;

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.print(client->id());
    Serial.println(" disconnected");
 
  } else if(type == WS_EVT_DATA){
 
    Serial.print(client->id());
    Serial.print("->");
    char message[255];
    for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
          message[i] = data[i];
    }
    message[len] = 0;
    
    Serial.println();
    //send text to all connected devices
    server->textAll(data,len);
    LoRa.beginPacket();
    //relay msg to LoRa
    LoRa.print(message); 
    LoRa.endPacket();
    u8x8log.println(message);
    
  }
}

//create websocket
AsyncWebSocket ws("/msg");
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  char message[255];
  int index = 0;
  while (LoRa.available()) {            // can't use readString() in callback, so
    message[index] = LoRa.read();
    index ++;
  }
  message[index] = 0; //end string
  Serial.println(message);
  ws.textAll(message); //send to clients the msg received
  u8x8log.println(message);
    
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
  
  
  //screen
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8x8log.setRedrawMode(0);    // 0: Update screen with newline, 1: Update screen for every char  

  u8x8log.print("LoRaChat (c)2019\n");
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //generate a unique SSID per device based 
  String ssid("LoRa_" + String((uint16_t) (ESP.getEfuseMac() >> 32),HEX)); 
  const char password[] = "";
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid.c_str(), password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  u8x8log.print("SSID: ");
  u8x8log.println(ssid.c_str());
  u8x8log.println("192.168.4.1");
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());


  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  //https://github.com/me-no-dev/ESPAsyncWebServer#serving-files-in-directory
  server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");;
  

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  // Start server
  server.begin();
 
}
 
void loop(){
   // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}
