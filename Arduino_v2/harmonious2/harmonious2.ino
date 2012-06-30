#define WEBDUINO_FAVICON_DATA "" //save 198 bytes.
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <IRremote.h>
#include <OneWire.h>

//config ehternet
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t ip[] = { 192, 168, 1, 210 };

WebServer webserver("", 80);
int DS18S20_Pin = 2;
OneWire ds(DS18S20_Pin);
IRsend irsend;

void tempRead(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  server.httpSuccess();
  if (type != WebServer::HEAD) {
    server.print("{\"sensors\": [{\"backroom\":");
    server.print(getTemp());
    server.print("}]}");
  }
}

void codeCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  char param[32];
  char value[300];
  char rawCode[250];
  
//  server.httpSuccess();

  if (type == WebServer::POST)  {
    server.readPOSTparam(param, 32, value, 250);
    if(strcmp(param,"type") == 0) {
      if(strcmp(value,"raw") == 0) {
        server.readPOSTparam(param, 32, value, 250);
        if(strcmp(param,"code") == 0) {
          strncpy(rawCode,"",strlen(value)+1);
          strncpy(rawCode,value,strlen(value));
//          rawCode=temp; //The rawCode pointer can now point to the new array space, rather than the old one.
        }
        server.readPOSTparam(param, 32, value, 250);
      }
      parseData(rawCode, atoi(value));
    }
  }
  server.httpSuccess();
}

void parseData(char* raw, int leng) {
  int x=0;  
  int n=0;
  char f[5];
  unsigned int cde[50];
  while(sscanf(raw, "%31[^,]%n", f, &n) == 1) {
    raw+=n;
    cde[x] = atoi(f);
    if(*raw != ',') {
      break;
    }
    raw++;
   x++;
  }
  sendRawIR(cde, leng);
}

void sendRawIR(unsigned int *code, int len) {
  irsend.sendRaw(code, len, 38);
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
  webserver.addCommand("ir", &codeCmd);
  webserver.addCommand("temp", &tempRead);
  webserver.begin();
}

void loop() {
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
}
