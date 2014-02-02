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

// --------------------Buttons--------------------

//const int Up              =  A0; // Up/Prev key on analog 0
//const int Select         =  A1; // Select key on analog 1
//const int Down          =  A2; // Down key on analog 2

//pinMode(Up, INPUT);
//pinMode(Select, INPUT);
//pinMode(Down, INPUT);

// --------------------Buzzer--------------------

int Buzzer        =  2;   // Buzzer on digital pin 2
pinMode(Buzzer, OUTPUT); 

// --------------------LCD stuff--------------------

LiquidCrystal lcd(3,5,6,7,8,9);  // These are the pins used for the parallel LCD

// --------------------printer stuff--------------------

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
char server[] = "87.51.52.114";    // name address for server (using DNS)

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
    If no orders are pending, wait 1 minute, then go back to (1).
 3) if there are items to print, take top print request from queue and request it. Otherwise go to (0)
 4) process the return data (send to printer and display), until connection ends. Send the ‘processing’ message, and go to (3).

 All processed in the loop.

*/

// states for overall process

const int requestOrders = 1; // issue request for list of orders
const int processOrderList = 2; // process the return data
const int requestPrint = 3; // issue request for an order to print
const int processPrintData = 4; // process the return data (and display and print it)

int processStep = requestOrders;

void setup() {
// --------------------set up LCD--------------------

 lcd.begin(20,4);

// --------------------set up printer --------------------

// printer.sleep(); // epson does not seem to use this

// --------------------Start serial, might need to disable when adding LCD--------------------

 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

// --------------------set up ethernet--------------------

 // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET /arduino1.php?sc=1234 HTTP/1.1");
    client.println("Host: 87.51.52.114");
    client.println("Connection: close");
    client.println();
  }
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

void checkOrders() {

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

int toPrint[10]; // assume no more than 10 pending orders in one print update
int numToPrint = 0;

// add an order to the print queue

void addToPrint(int order) {
  toPrint[numToPrint]=order;
  ++numToPrint;
  }

// return an order to print, or -1 if there are no more orders to print

int getOrderToPrint() {
  if (numToPrint <1) return -1;
  return toPrint[--numToPrint];
  }

void loop()
{

  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);

      if (state == 0) { // skipping header
        if (c == matchString[matched]) {
          ++matched;
//          Serial.print("Matched ");
//          Serial.print(matched);
//          Serial.println(" char. ");
          }
        else {
//          if (matched>0) Serial.println("No match."); // end matching
          matched = 0;
          }
        if (matched>=matchLen) state=inArgs;
      }

      if (state == 1) {
        if (c == '<') { // end of line (order number and status)
          Serial.println();
          Serial.print(F("Processing order "));
          Serial.print(a[0]);
          Serial.print(" status ");
          Serial.println(a[1]);
          Serial.println();
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
        else {
          if (inNum) {
            inNum = 0;
//            Serial.print(F("arg "));
//            Serial.print(numArgs);
//            Serial.print(F(" is "));
//            Serial.print(a[numArgs]);
            }
          }
        }

      if (state == 2) {
      }

  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    while(true);
  }
}

// for each pending order
//
// retrieve data (e.g. http://87.51.52.114/arduino3.php?sc=1234&o=69)
// send the result to the printer/display
// and call  http://87.51.52.114/arduino4.php?sc=1234&o=69&s=processing

void PrintOrder(int order) {
    
    tone(buzzer, 2093, 1000);
    noTone(buzzer);

    
  
  TM88.start();
  //printer.wake(); // adafruit printer version of the above
  
  
  
  TM88.println("Hello World start");  
  //printer.println("hello");// adafruit printer version of the above

  TM88.doubleHeightOn();
  //printer.doubleHeightOn(); // adafruit printer version of the above
  
  TM88.boldOn();
  //printer.boldOn(); // adafruit printer version of the above
  
  TM88.println("PAYMENT METHOD");  
  
  TM88.doubleHeightOff();
  //printer.doubleHeightOFF(); // adafruit printer version of the above

  TM88.boldOff(); 
  //printer.boldOff(); // adafruit printer version of the above

  TM88.cut(); // adafruit printer does not have a knife.... hmm... might want to stick to the epson?
  
  //printer.sleep(); // only for adafruit printer
  
  }
