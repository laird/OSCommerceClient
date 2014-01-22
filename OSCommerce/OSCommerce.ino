#include <LiquidCrystal.h>

//#include <Dhcp.h>
//#include <Dns.h>
//#include <Ethernet.h>
//#include <EthernetClient.h>
//#include <EthernetServer.h>
//#include <EthernetUdp.h>
//#include <util.h>

/*
   Web client sketch for IDE v1.0.1 and w5100/w5200
 Uses GET method.
 Posted October 2012 by SurferTim
 Last modified September 15, 2013
 */

#include <SPI.h>
#include <Ethernet.h>

// this must be unique
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// change to your network settings
IPAddress ip(192,168,1, 229);
//IPAddress gateway(192, 168, 1,1);
//IPAddress subnet(255, 255, 255, 0);

// change to your server
IPAddress server(87,51,52,114); // OSCommerce server

// request http://87.51.52.114/arduino1.php?sc=1234

//Change to your domain name for virtual servers
char serverName[] = "87.51.52.114";

//char serverName[] = "www.google.com";
// If no domain name, use the ip address above
// char serverName[] = "74.125.227.16";

// change to your server's port
int serverPort = 80;

EthernetClient client;
int totalCount = 0;
char pageAdd[64];

// set this to the number of milliseconds delay
// this is 30 seconds
#define delayMillis 30000UL

unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);

  // disable SD SPI
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  Serial.print(F("Starting ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("failed"));
  else Serial.println(Ethernet.localIP());

  delay(2000);
  Serial.println(F("Ready"));
}

void loop()
{
  thisMillis = millis();

  if(thisMillis - lastMillis > delayMillis)
  {
    lastMillis = thisMillis;

    // Modify next line to load different page
    // or pass values to server
    sprintf(pageAdd,"/arduino1.php?sc=1234",totalCount); // get list of orders

    // sprintf(pageAdd,"/arduino.php?test=%u",totalCount);

    if(!getPage(server,serverPort,pageAdd)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    totalCount++;
    Serial.println(totalCount,DEC);
  }    
}

byte getPage(IPAddress ipBuf,int thisPort, char *page)
{
  int c;
  char outBuf[128];

  Serial.print(F("connecting..."));

  if(client.connect(ipBuf,thisPort))
  {
    Serial.println(F("connected"));

    sprintf(outBuf,"GET %s HTTP/1.1",page);
    client.println(outBuf);
    Serial.println(outBuf);
    sprintf(outBuf,"Host: %s",serverName);
    client.println(outBuf);
    Serial.println(outBuf);
    client.println(F("Connection: close, , "));
  } 
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  // connectLoop controls the hardware fail timeout
  int connectLoop = 0;

  //int arg[3];
  //int numArgs = -1;
  //int inNum = 0;

  //int state = 0;

  //  const int inHeader = 0;
  //  const int inArgs = 1;
  //  const int inStatus = 2;

  //  int matched = 0;
  //  String matchString = "Content-Type: text/html";
  //  const int matchLen = 23;

  Serial.println(F("Waiting for input."));

  while(client.connected())
  {
    while(client.available())
    {
      c = client.read(); // get a char
      Serial.write(c); // echo to debugger


      //      if (state == 0) { // skipping header
      //        if (inChar == matchString[matched]) {
      //          ++matched;
      //          Serial.write('*');
      //          }
      //        else matched = 0;
      //        if (matched==matchLen) state=inArgs;
      //      }
      //      if (state == 1) {
      //        if (inChar == '<') { // end of line (order number and status)
      //          Serial.print(F("Processing order "));
      //          Serial.print(arg[0]);
      //          Serial.print(" status ");
      //          Serial.println(arg[1]);
      //          }
      //        if (inChar>='0' && inChar<='9') { // digits
      //          if (!inNum) { // first digit in number
      //            numArgs += 1;
      //            inNum = 1;
      //            }
      //          arg[numArgs] = arg[numArgs]*10+inChar-'0'; // add digit to int
      //          }
      //        else {
      //          if (inNum) {
      //            inNum = 0;
      //            Serial.print(F("arg "));
      //            Serial.print(numArgs);
      //            Serial.print(F(" is "));
      //            Serial.print(arg[numArgs]);
      //            }
      //          }
      //        }
      //      
      //      if (state == 2) {
      //      }

      // set connectLoop to zero if a packet arrives
      connectLoop = 0;
    }

    connectLoop++;

    // if more than 10000 milliseconds since the last packet
    if(connectLoop > 10000)
    {
      // then close the connection from this end.
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
    // this is a delay for the connectLoop timing
    delay(1);
  }

  Serial.println();

  Serial.println(F("disconnecting."));
  // close client end
  client.stop();

  return 1;
}

