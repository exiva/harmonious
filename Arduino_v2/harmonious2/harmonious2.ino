#define WEBDUINO_FAVICON_DATA "" //save 198 bytes.
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <OneWire.h>
#include <IRremote.h>

//Define Ethernet addresses
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t ip[] = { 192, 168, 1, 210 };

#define PREFIX ""
#define NAMELEN 32
#define VALUELEN 500
WebServer webserver(PREFIX, 80);
int DS18S20_Pin = 2;
OneWire ds(DS18S20_Pin);

void irCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  char name[NAMELEN];
  char value[VALUELEN];
  server.httpSuccess();
  if (type == WebServer::POST)  {
    server.readPOSTparam(name, NAMELEN, value, VALUELEN);
    if(strcmp(name,"type") == 0) {
      if(strcmp(value,"raw") == 0) {
        while (server.readPOSTparam(name, NAMELEN, value, VALUELEN)) {
          if(strcmp(name,"code") == 0) {
            server.print("Code: ");
            server.print(value);
          } else if(strcmp(name,"len") == 0) {
            server.print(" Length: ");
            server.print(value);
          }
        }
      } else if(strcmp(value,"rc6") == 0) {
        server.readPOSTparam(name, NAMELEN, value, VALUELEN);
        Serial.print(name);
        Serial.print(" ");
        Serial.print(value);
      } else if (strcmp(value,"sony") == 0) {
        server.readPOSTparam(name, NAMELEN, value, VALUELEN);
        Serial.print(name);
        Serial.print(" ");
        Serial.print(value);
      }       
    }
  }
}

void tempRead(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  server.httpSuccess();
  if (type != WebServer::HEAD) {
    server.print("{\"sensors\": [{\"backroom\":");
    server.print(getTemp());
    server.print("}]}");
  }
}

float getTemp(){
 byte data[12];
 byte addr[8];

 if ( !ds.search(addr)) {
   //no more sensors on chain, reset search
   ds.reset_search();
   return -1000;
 }

 if ( OneWire::crc8( addr, 7) != addr[7]) {
   return -1000;
 }

 if ( addr[0] != 0x10 && addr[0] != 0x28) {
   return -1001;
 }

 ds.reset();
 ds.select(addr);
 ds.write(0x44,1); // start conversion, with parasite power on at the end

 byte present = ds.reset();
 ds.select(addr);  
 ds.write(0xBE); // Read Scratchpad

 
 for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
 }
 
 ds.reset_search();
 
 byte MSB = data[1];
 byte LSB = data[0];

 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 float tempc = tempRead / 16;
 float tempf = tempc * 9 / 5 +32;
 
 return tempf;
}

void setup() {
  Ethernet.begin(mac, ip);
  webserver.addCommand("ir", &irCmd);
  webserver.addCommand("temp", &tempRead);
  webserver.begin();
  Serial.begin(115200);
  Serial.print("Server online.");
}

void loop() {
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
}
