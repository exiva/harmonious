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

P(tempstart) = "{\"sensors\": [{\"backroom\":";
P(tempend) = "}]}";

void tempRead(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  server.httpSuccess();
  if (type != WebServer::HEAD) {
    server.printP(tempstart);
    server.print(getTemp());
    server.printP(tempend);
  }
}

void codeCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  char param[32];
  char value[250];
  if (type == WebServer::POST)  {
    server.readPOSTparam(param, 32, value, 20);
    if(strcmp(param,"type") == 0) {
      if(strcmp(value,"raw") == 0) {
        char rawCode[250];
        server.readPOSTparam(param, 32, value, 250);
        if(strcmp(param,"code") == 0) {
          strncpy(rawCode,"",strlen(value)+1);
          strncpy(rawCode,value,strlen(value));
        }
        server.readPOSTparam(param, 32, value, 3);
        parseRAWCode(rawCode, atoi(value));
      } else if(strcmp(value,"rc6") == 0) {
        server.readPOSTparam(param, 32, value, 20);
        unsigned long long tmp = 1234;
        server.readPOSTparam(param, 5, value, 3);
        irsend.sendRC6(tmp,atoi(value));
      } else if(strcmp(value,"nec") == 0) {
        server.readPOSTparam(param, 32, value, 20);
        unsigned long tmp = strtol(value,NULL,16);
        server.readPOSTparam(param, 5, value, 3);
        irsend.sendNEC(tmp,atoi(value));
      } else if(strcmp(value,"sony") == 0) {
        server.readPOSTparam(param, 32, value, 20);
        unsigned long tmp = strtol(value,NULL,16);
        server.readPOSTparam(param, 5, value, 3);
        irsend.sendSony(tmp,atoi(value));
        delay(200);
        irsend.sendSony(tmp,atoi(value));
      }
    }
  }
  server.httpSuccess();
}

void parseRAWCode(char* raw, int leng) {
  int x=0;  
  int n=0;
  char f[5];
  unsigned int cde[leng];
  while(sscanf(raw, "%31[^,]%n", f, &n) == 1) {
    raw+=n;
    cde[x] = atoi(f);
    if(*raw != ',') {
      break;
    }
    raw++;
   x++;
  }
//  sendRawIR(cde, leng);
  irsend.sendRaw(cde, leng, 38);
  return;
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
 float tempout = tempRead / 16 * 9 / 5 + 32;

 return tempout;
}

void setup() {
  Ethernet.begin(mac, ip);
  webserver.addCommand("ir", &codeCmd);
  webserver.addCommand("temp.json", &tempRead);
  webserver.begin();
}

void loop() {
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
}
