/*
  OSCommerce client 2

 This sketch connects to a website using an Arduino Wiznet Ethernet shield,
 and uses a series of simple web services.
 Based on sample Arduino HTTP sketch created 18 Dec 2009 by David A. Mellis
 modified 9 Apr 2012 by Tom Igoe, based on work by Adrian McEwen
 Added OSCommerce logic Jan 2014, Laird Popkin
 */

#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
//#include <Thermal.h> // Lib for adafruit thermal printer
#include <thermalprinter.h> // Lib for epson thermal printer

String securityCode = "1234"; // unique for each customer

// wait times, in ms

int waitPollForOrders = 30000;
int waitEthernetOn = 1000;

// LCD stuff

LiquidCrystal lcd(3,5,6,7,8,9);  // These are the pins used for the parallel LCD
const int Buzzer        =  2;   // Buzzer on digital pin 2
pinMode(Buzzer, OUTPUT); 


const int haveprinter = 0; // set to 1 if there is a printer
LiquidCrystal lcd(3,5,6,7,8,9);  // These are the pins used for the parallel LCD

// --------------------printer stuff--------------------
const int printer_RX_Pin = 2;
const int printer_TX_Pin = 3;

int printer_RX_Pin = 2;
int printer_TX_Pin = 3;

Epson TM88 = Epson(printer_RX_Pin, printer_TX_Pin); // Init Epson TM-T88III Receipt printer
// Thermal printer(printer_RX_Pin, printer_TX_Pin, 19200); // Init adafruit thermal printer

// --------------------ethernet stuff--------------------

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(87,51,52,114); // OSCommerce server
const char server[] = "87.51.52.114";    // name address for server (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,1,229);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;



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
// --------------------set up LCD--------------------

  lcd.begin(20,4);

// --------------------set up printer --------------------

// printer.sleep(); // epson does not seem to use this

  sendCheckOrders();
}

void startEthernet() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(waitEthernetOn);
  Serial.println("connecting...");
}

int a[3];
int numArgs = -1;
int inNum = 0;

// states for parsing the return lists
const int inHeader = 0; // skip header
const int inArgs = 1;  // parse out lists of ints
const int inStatus = 2; // after list, check OK

int state = inHeader;

int matched = 0;
char matchString[] = "html";
const int matchLen = 4;

const int maxNumToPrint = 2; // Print only this many orders in a batch
int toPrint[maxNumToPrint];
int numToPrint = 0;

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
    if (haveprinter) printer.wake();
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

// Loop processing incoming data

void loop() {
  showProcessStep(); // for debugging fun  
  switch(processStep) {
  case requestOrders: 
    sendCheckOrders();
    break;
  case processOrderList:
    processIncoming();
    break;
  case requestPrint:
    sendPrintOrder();
    break;
  case processPrintData:
    processIncoming();
    break;
  case reportProcessing:
    sendProcessingOrder();
    break;
  case processReportData:
    processIncoming();
    break;
  }
}

// process returned data

void processIncoming() {
  Serial.println("process incoming");
  showProcessStep();

  // if there are incoming bytes available
  // from the server, read them and print them:
  // And parse/process them as appropriate for processStep.

  while (client.connected()) {
      char c = client.read();
      Serial.print(c);

      // if processing print data, just send it to the printer
      if (processStep==processPrintData) {
        if (state == 0) { // skipping header
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
          if (haveprinter) printer.print(c);
          //Serial.print(c);
        }
      }

      // if processing order list, parse out orders and queue them to print
      if (processStep==processOrderList) {

        if (state == 0) { // skipping header
          if (c == matchString[matched]) {
            ++matched;
          }
          else {
            //          if (matched>0) Serial.println("No match."); // end matching
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
    }
  //}

  // when the server's disconnected, stop the client:
    setParseState(0);

    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    showProcessStep();

    // If there are any queued to print, print one
    if (processStep==processOrderList) {
      Serial.println("*** done processing order list");
      if (numToPrint>0) sendPrintOrder();
      }

    // If we were printing, stop printing and put printer to sleep
    if (processStep==processPrintData) {
      Serial.println("*** END PRINTING ***");
      if(haveprinter) printer.sleep();
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
void PrintOrder(int order) {
    
    tone(buzzer, 2093, 1000);
    noTone(buzzer);

void setParseState(int s) {
  state = s;
  Serial.print("   Set parse state: "); 
  showProcessStep();
}
    
  
  TM88.start();
  //printer.wake(); // adafruit printer version of the above
  
  
  
  TM88.println("Hello World start");  
  //printer.println("hello");// adafruit printer version of the above

void setProcessStep(int s) {
  processStep = s; 
  state = 0;
  Serial.print("   Set process Step: "); 
  showProcessStep();
}
  TM88.doubleHeightOn();
  //printer.doubleHeightOn(); // adafruit printer version of the above
  
  TM88.boldOn();
  //printer.boldOn(); // adafruit printer version of the above
  
  TM88.println("PAYMENT METHOD");  
  
  TM88.doubleHeightOff();
  //printer.doubleHeightOFF(); // adafruit printer version of the above

int oldProcessStep = 0;
int oldState = 0;
  TM88.boldOff(); 
  //printer.boldOff(); // adafruit printer version of the above

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
  TM88.cut(); // adafruit printer does not have a knife.... hmm... might want to stick to the epson?
  
  //printer.sleep(); // only for adafruit printer
  
  }
  //Serial.print(processStep);
  Serial.print(" State: ");
  Serial.print(state);
  Serial.print(" queue: ");
  Serial.println(numToPrint);
}




