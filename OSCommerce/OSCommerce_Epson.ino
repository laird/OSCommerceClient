/*
  OSCommerce client

 This sketch connects to a website using an Arduino Wiznet Ethernet shield,
 and uses a series of simple web services.

 Based on sample Arduino HTTP sketch created 18 Dec 2009 by David A. Mellis
 modified 9 Apr 2012 by Tom Igoe, based on work by Adrian McEwen

 Added OSCommerce logic Jan 2014, Laird Popkin and Bo
 */

#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Dhcp.h>
#include <Dns.h>
#include <EthernetClient.h>
//#include <EthernetServer.h>
//#include <EthernetUdp.h>
#include <util.h>
#include <LiquidCrystal.h>
#include <thermalprinter.h> 


// ----- Config Section - Only change These -----

// Server (unique for each storefront)
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:

const char server[] = "87.51.52.114";    // name address for server (using DNS)
//IPAddress server(87,51,52,114); // example, if you want to use IP address

String securityCode = "1234"; // unique for each customer. Not really secure, but better than nothing.
int waitPollForOrders = 30000; // Look for orders every X ms

// Buzzer
#define Buzzer 0
// Set to 0 if no buzzer is connected, set to 1 if we have one connected
#define Buzzerport 2
// What port do we have the buzzer on

// LCD display
#define LCD 0
// Define if we have one connected or set to 0 if we dont use LCD

#define DebugPrint 1
// Define 1 to send printer output to serial debugger

int printer_RX_Pin = 6; // port that the RX line is connected to
int printer_TX_Pin = 7; // port that the TX line is connected to

const int maxNumToPrint = 2; // Print only this many orders in a batch (because we used a fixed size array queue)

#define debugNetIn 0

// ----- Config Section END -----

// wait times, in ms

int waitEthernetOn = 1000; // give ethernet board 1s to initialize

// Buzzer stuff

#if Buzzer
const int Buzzer        =  Buzzerport;
pinMode(Buzzer, OUTPUT);
#endif

// LCD stuff

#if LCD
LiquidCrystal lcd(3,5,6,7,8,9);  // These are the pins used for the parallel LCD
#endif

// --------------------ethernet stuff--------------------

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,1,229);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// printer stuff,

Epson TM88 = Epson(printer_RX_Pin, printer_TX_Pin); // Init Epson TM-T88III Receipt printer


// ---- other logic -----

// states for parsing the return lists
#define inHeader 0
// skip header
#define inArgs 1
// parse out lists of ints
#define inStatus 2
// after list, check OK

int state = inHeader;

int matched = 0;
char matchString[] = "html"; // match this string to detect end of headers.
const int matchLen = 4;

int toPrint[maxNumToPrint];
int numToPrint = 0;

// business logic

/*
 The logic is:
 1) request the list of orders
 2) process the list until connection ends. If any are pending, add to a queue to print.
 If no orders are pending, wait 1 minute, then go back to (1). If more than 10 orders are
 pending, ignore the rest.
 3) if there are items to print, take top print request from queue and request it. Otherwise go to (0)
 4) process the return data (send to printer and display), until connection ends. Send the ‘processing’ message, and go to (3).

 All of the above are 'processState' values, handled in loop().

 */

// states for overall process

const int requestOrders = 1; // issue request for list of orders
const int processOrderList = 2; // process the return data
const int requestPrint = 3; // issue request for an order to print
const int processPrintData = 4; // process the return data (and display and print it)
const int reportProcessing = 5; // report that an order is processing
const int processReportData = 6;  // ignore returned data

int processStep = requestOrders;

int order = 0; // number of order being processed

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
    }

  // --------------------set up LCD--------------------

#if LCD
  Serial.println(F("Set up LCD"));
  lcd.begin(20,4);
#endif

  // --------------------set up printer --------------------


  Serial.println(F("Set up printer"));
  TM88.start();
  TM88.characterSet(4);

  sendCheckOrders();
  }

// Loop processing incoming data

void loop() {
  showProcessStep(); // for debugging fun
  switch(processStep) {
    case requestOrders: sendCheckOrders();
      break;
    case processOrderList: processIncoming();
      break;
    case requestPrint: sendPrintOrder();
      break;
    case processPrintData: processIncoming();
      break;
    case reportProcessing: sendProcessingOrder();
      break;
    case processReportData: processIncoming();
      break;
    }
  }

void startEthernet() {
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(waitEthernetOn);
  Serial.println("Connecting...");
  }

// Check orders
void sendCheckOrders() {
  char b[100]="";
  startEthernet();
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected for orders");
    // Make a HTTP request:

    //sprintf(b, "GET /arduino1.php?sc=%s HTTP/1.1", securityCode);
    //client.println(b);
    //Serial.println(b);
    //sprintf(b, "Host: %s", server);
    //client.println(b);
    //Serial.println(b);

    Serial.print("GET /arduino1.php?sc=");
    Serial.print(securityCode);
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(server);
    Serial.println("Connection: close");
    Serial.println();

    client.print("GET /arduino1.php?sc=");
    client.print(securityCode);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();

    delay(100);

    setProcessStep(processOrderList);
  }
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

// Print order (if any are in queue)
//
// retrieve data (e.g. http://87.51.52.114/arduino3.php?sc=1234&o=69)
// send the result to the printer/display
// and call http://87.51.52.114/arduino4.php?sc=1234&o=69&s=processing

void sendPrintOrder() {
  char b[100]="";
  if (numToPrint<1) return;
  order = getOrderToPrint();

  startEthernet();
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("*** Print order.");
    // Make a HTTP request:
    // sprintf(b,"GET /arduino3.php?sc=1234&o=%s HTTP/1.1", order);
    // Serial.println(b);
    // client.println(b);

    Serial.print("GET /arduino3.php?sc=");
    Serial.print(securityCode);
    Serial.print("&o=");
    Serial.print(order);
    // append printer type, so server can format for the printer


    Serial.println(" HTTP/1.1");
    // sprintf(b,"Host: %s", server);
    // Serial.println(b);
    // Serial.println(b);
    Serial.print("Host: ");
    Serial.println(server);
    Serial.println("Connection: close");
    Serial.println();


    client.print("GET /arduino3.php?sc=");
    client.print(securityCode);
    client.print("&o=");
    client.print(order);
    client.println(" HTTP/1.1");
    // sprintf(b,"Host: %s", server);
    // client.println(b);
    // Serial.println(b);
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();

    delay(100);

    setProcessStep(processPrintData);

   

    loop();
  }
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    setProcessStep(requestOrders);
    loop();
  }
}


// Report processing of an order
//
// call http://87.51.52.114/arduino4.php?sc=1234&o=69&s=processing
//
// relies on global order to be the number of the order being processed

void sendProcessingOrder() {
  char b[100] = "";

  startEthernet();
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.print("Reporting order ");
    Serial.print(order);
    Serial.println(" processed.");

    Serial.println("connected for print");
    // Make a HTTP request:

    Serial.print("GET /arduino4.php?sc=1234&o=");
    Serial.print(order);
    Serial.println("&s=processing HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(server);
    // sprintf(b,"Host: %s", server);
    // Serial.println(b);
    // Serial.println(b);
    Serial.println("Connection: close");
    Serial.println();


    // sprintf(b,"GET /arduino4.php?sc=1234&o=%d&s=processing HTTP/1.1", order);
    // client.println(b);
    // Serial.println(b);
    client.print("GET /arduino4.php?sc=1234&o=");
    client.print(order);
    client.println("&s=processing HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    // sprintf(b,"Host: %s", server);
    // client.println(b);
    // Serial.println(b);
    client.println("Connection: close");
    client.println();

    delay(100);

    setProcessStep(processReportData); // so we process the return data properly
    loop();
  }
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    setProcessStep(requestOrders);
    loop();
  }
}

// process returned data

void processIncoming() {
  char c2;
  Serial.println("process incoming");
  showProcessStep();

  // if there are incoming bytes available
  // from the server, read them and print them:
  // And parse/process them as appropriate for processStep.
  // Until the connection ends.

  while (client.connected()) {
      char c = client.read();

#if debugNetIn
      Serial.print(c);  // echo all incoming data
#endif

      // if processing print data, just send it to the printer
      if (processStep==processPrintData) processPrintChar(c);

      // if processing order list, parse out orders and queue them to print
      if (processStep==processOrderList) processOrderListChar(c);
      }

  // when the server's disconnected, stop the client:
    setParseState(0);

    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    showProcessStep();

    // If there are any queued to print, print one
    if (numToPrint>0) {
        setProcessStep(requestPrint);
        sendPrintOrder();
        }

    // If we were printing, stop printing and put printer to sleep
    if (processStep==processPrintData) {
      Serial.println("*** END PRINTING ***");
      Serial.println();


      TM88.cut();

      setProcessStep(reportProcessing);
      sendProcessingOrder();  // and report that the order is being processed
    }

    // When done reporting an order processed, move on to print the next order
    if (processStep == processReportData) {
      Serial.println("*** done processing return from reporting progress");
      order = 0;
      if (numToPrint>0) {
        setProcessStep(requestPrint); // if any more to print, do one
        sendPrintOrder();
      }
    }

    // if there's nothing else to do, wait 30 seconds, then check for orders
    Serial.println("waitPollForOrders");
    delay(waitPollForOrders);
    setProcessStep(requestOrders);
    sendCheckOrders();
  }

// process incoming character when printing a receipt
void processPrintChar(char c) {
  char c2;

    if (state == inHeader) { // skipping header
      if (c == matchString[matched]) {
        ++matched;
      }
      else {
        matched = 0;
      }
      if (matched>=matchLen) {
        setParseState(inArgs);
        Serial.println("*** START PRINTING ***"); // start printing after header
      }
    }

    if (state == inArgs) {

      /* Formatting codes are:
        [B] bold, [b] bold off
        [U] underline, [u] underline off
        [R] reverse (inverse), [r] reverse off
        [D] double height, [d] standard height
        [F] line feed
        [C] cut (not needed - we cut between receipts)
      */

      if (c=='[') { // control codes start with '[', so execute code
        c2 = client.read();
        switch(c2) {

          case 'B': TM88.boldOn(); break;
          case 'b': TM88.boldOff(); break;
          case 'D': TM88.doubleHeightOn(); break;
          case 'd': TM88.doubleHeightOff(); break;
          case 'R': TM88.reverseOn(); break;
          case 'r': TM88.reverseOff(); break;
          case 'U': TM88.underlineOn(); break;
          case 'u': TM88.underlineOff(); break;
          case 'F': TM88.feed(); break;
          case 'C': TM88.cut(); break;

#if DebugPrint
          case 'B': Serial.print("*** boldOn"); break;
          case 'b': Serial.print("*** boldOff"); break;
          case 'D': Serial.print("*** doubleHeightOn"); break;
          case 'd': Serial.print("*** doubleHeightOff"); break;
          case 'R': Serial.print("*** reverseOn"); break;
          case 'r': Serial.print("*** reverseOff"); break;
          case 'U': Serial.print("*** underlineOn"); break;
          case 'u': Serial.print("*** underlineOff"); break;
          case 'F': Serial.print("*** feed"); break;
          case 'C': Serial.print("*** cut"); break;
#endif
          default: Serial.print("*** bad code "); Serial.println(c2);
        }  
        }
      else { // not a control code, so print it

        printer.print(c);

#if DebugPrint
        Serial.print(c);
#endif
        }
      }
    }

// process incoming character c when receiving an order list
void processOrderListChar(char c) {
  static int a[3]; // used to collect args
  static int numArgs = -1;
  static int inNum = 0;

    if (state == 0) { // skipping header
      if (c == matchString[matched]) {
        ++matched;
      }
      else {
        matched = 0;
      }
      if (matched>=matchLen) setParseState(inArgs);
    }

    if (state == inArgs) {
      if (c == '<') { // end of line (order number and status)
        if (a[1]==1) addToPrint(a[0]); // if pending, add to queue to print
        numArgs = -1;
        a[0]=0;
        a[1]=0;
      }
      if (c>='0' && c<='9') { // digits
        if (!inNum) { // first digit in number
          numArgs += 1;
          inNum = 1;
        }
        a[numArgs] = a[numArgs]*10+c-'0'; // add digit to int
      }
      else { // not a digit
        if (inNum) {
          inNum = 0;
        }
      }
    }

    if (state == 2) {
      Serial.print(c);
    }

  }

// add an order to the print queue. If maxed out, ignore it.

void addToPrint(int order) {
  if (numToPrint>= maxNumToPrint) {
    //Serial.println("*** ignoring ***");
  }
  else {
    Serial.print("*** Recorded order ");
    Serial.println(order);
    toPrint[numToPrint]=order;
    ++numToPrint;
  }
}

// return an order to print, or -1 if there are no more orders to print

int getOrderToPrint() {
  if (numToPrint <1) {
    Serial.println("*** Something from nothing ***");
    return -1;
  }
  return toPrint[--numToPrint];
}

void setParseState(int s) {
  state = s;
  Serial.println("   Set parse state.");
  showProcessStep();
}

void setProcessStep(int s) {
  processStep = s;
  state = 0;
  Serial.println("   Set process Step: ");
  showProcessStep();
}

int oldProcessStep = 0;
int oldState = 0;

void showProcessStep() {
  if (oldProcessStep==processStep && oldState==state) return; // nothing interesting to print

  oldProcessStep = processStep;
  oldState = state;

  Serial.print("   Step: ");
  switch(processStep) {
  case requestOrders:
    Serial.print("requestOrders");
    break;
  case processOrderList:
    Serial.print("processOrderList");
    break;
  case requestPrint:
    Serial.print("requestPrint");
    break;
  case processPrintData:
    Serial.print("processPrintData");
    break;
  case reportProcessing:
    Serial.print("reportProcessing");
    break;
  case processReportData:
    Serial.print("processReportData");
    break;
  }
  //Serial.print(processStep);
  Serial.print(" State: ");
  Serial.print(state);
  Serial.print(" queue: ");
  Serial.println(numToPrint);
}

