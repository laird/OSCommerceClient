#!/usr/bin/python
# coding: utf8

__author__ = 'laird'
#
# OSCommerceClient.py
#
# Polls an OSCommerce site and pulls down a list of pending orders, prints them, and sets their status to 'processing'
#
# uses requests from http://docs.python-requests.org/en/latest/user/install/#distribute-pip
#
# Todo:
# - remove extended character substitution starting at line 203
# - make client exit and restart on lost connection, maybe after 3 tries 5 mins apart write a error file of some kind? or send an mail?
# - so far i manage by rebooting the pi every 24 hours just before shop opens
# - make a parameter that forces all orders to a set status and then exit. Usefull to set orders as done each midnight
# - as an alternative, make line 174 an parameter so it can be chosen what status to look for and just change what php file to use for lookup. Arduino2.php will list all orders processing and status in that one is 2 and not 1??
# - of course the force option is most ideal i think, but then it would have to look in another file and not the default one...

import logging
import requests
import Queue
from threading import Thread
import time
import argparse
from escpos import *

try:
    import RPi.GPIO as GPIO
except ImportError:
    print "error importing GPIO. No access to printer for you!"
    havePrinter = False;

# configuration (command line args are preferable, see argparse below)

server = "87.51.52.114" # Old ip of test server, replace with your own
securityCode = "1234" # Example security code
pollPage = "/arduino1.php" # poll for orders
detailPage = "/arduino3.php" # get text of receipt to print
setPage = "/arduino4.php" # set status of an order. arduino4 notify the custome, arduino5 does not
printCopies = 2 # number of copies, usefull to check if driver returns correct amount of cash
testMode = 0 # set to 1 to suppress setting order status (so can retest the same order)

# Set to the serial port for your printer

havePrinter = True
serialPort = "/dev/ttyAMA0" # for GPIO
#serialPort = "/dev/ttys001" # for USB port on Mac (may vary)

# Buzzer on GPIO 22 = pin 15
try:
    buzzer = 15
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(buzzer, GPIO.OUT, initial=GPIO.HIGH) # Initial high stat works with a PNP transistor driving a buzzer
except NameError:
    print "No GPIO means no buzzer"

# Standard

waitPollForOrders = 30 # wait 30 seconds between polls
printCopies = 1 # number of copies of receipt to print, uefull to check if driver returns the right amount of cash.
maxNumToPrint = 5
buzzerTime = 3 # buzz for three second

# parse command line arguments

parser = argparse.ArgumentParser(description='This is the OSCommerce client by laird.', epilog="set either poll or reset, not both.")
# group = parser.add_mutually_exclusive_group()
parser.add_argument('-p','--poll', metavar='seconds', type=int, default=30, help='poll for new orders')
parser.add_argument('-f','--force', type=int, default=0, help='force orders to a given status code')
parser.add_argument('-c','--printCopies', type=int, default=1, help='print this many copies of each receipt')
parser.add_argument('-t','--testMode', type=int, default=0, help='set test mode to 1 to leave orders in unchanged state')
parser.add_argument('-i','--havePrinter', type=int, default=1, help='set to 1 if you have a printer, 0 if not')
parser.add_argument('-s','--server', default="87.51.52.114", help='address of server')
parser.add_argument('-e','--securityCode', default="1234", help='security code for server')
parser.add_argument('-q','--pollPage', default="/arduino1.php", help='page to poll for orders')
parser.add_argument('-d','--detailPage', default="/arduino3.php", help='page to retrieve order details')
parser.add_argument('-a','--setPage', default="/arduino4.php", help='page to set status')
parser.add_argument('-n','--serialPort', default="/dev/ttyAMA0", help='serial port for printer')


# note: Running with -h or --help prints the above info

args = parser.parse_args()

# special params

if args.force:
    print "I don't know how to force status yet."
else:

    # set variables for the params
    if args.poll:
        waitPollForOrders=args.poll
        print "waitPollForOrders "+str(waitPollForOrders)
    if args.printCopies:
        printCopies = args.printCopies
        print "printCopies "+str(printCopies)
    if args.testMode:
        testMode = args.testMode
        print "testMode "+str(testMode)
    if args.havePrinter:
        havePrinter = args.havePrinter
        print "havePrinter "+str(havePrinter)
    if args.server:
        server = args.server
        print "server "+server
    if args.securityCode:
        securityCode = args.securityCode
        print "securityCode "+securityCode
    if args.pollPage:
        pollPage = args.pollPage
        print "pollPage "+pollPage
    if args.detailPage:
        detailPage = args.detailPage
        print "detailPage "+detailPage
    if args.setPage:
        setPage = args.setPage
        print "setPage "+setPage
    if args.serialPort:
        serialPort = args.serialPort
        print "serialPort "+serialPort

    # setup

    if havePrinter:
        try:
            Epson = printer.Serial(serialPort)
            Epson._raw('\x1b\x52\x04') # Set to Danish 1 character set, comment out for english.
        except:
            havePrinter = False
            print "Failed to open serial port "+serialPort
    else:
        print "No printer"


    #order = ""
    #orders = []

    # queue of orders received in polling that need to be printed
    ordersToPrint = Queue.Queue()
    # queue of orders that were printed that need to be set to status 'processing'
    ordersToConfirm = Queue.Queue();

# modules that do the work:

# poll for orders, add any pending to queue for processing

def pollForUpdates(ordersToPrint, ordersToConfirm):
    "Poll for pending orders, add them to queue to print"
    print "starting poll worker"
    while True:
        print "poll for orders"

        url = "http://"+server+pollPage;
        payload = {'sc': securityCode};
        pollResult = requests.get(url, params=payload);

        textResult = pollResult.text;

        if (len(textResult) > 0):

            orders=textResult.split(',');

            for order in orders:
                # strip out formatting
                order = order.replace("[",""); # ignore starting [
                order = order.replace("<br />",""); # ignore break between lines
                order = order.replace("OK]",""); # ignore end OK]

                logging.debug("processing order "+order);
                vals = order.split();
                if (len(vals) == 2):
                    orderNum = int(vals[0]);
                    status = int(vals[1]);
                    logging.debug("found order "+str(orderNum)+" status "+str(status)+",")
                    if (status == 1):
                        print "queue order %i to print." % orderNum
                        ordersToPrint.put(orderNum)
                    else:
                        logging.debug("skip order "+str(orderNum));
                else:
                    if order=="OK]":
                        logging.debug("OK at end of list");
                    #else:
                        #logging.warn("Found bad order "+order);

        time.sleep(waitPollForOrders)


def printOrders(ordersToPrint, ordersToConfirm):
    "worker thread to print a queued order, then queue it to confirm"
    print "starting print worker"
    while True:
        order = ordersToPrint.get()
        print "printing order %i ." % +order

        url = "http://"+server+detailPage;
        payload = {'sc': securityCode, "o": order};
        printResult = requests.get(url, params=payload);
        printResult.encoding = 'ISO-8859-1'

        for copy in range(printCopies):
            print "Print copy "+copy

            textResult = printResult.text

            # replace non-ASCII characters

            textResult = textResult.replace(u"Å","AA")
            textResult = textResult.replace(u"Æ","AE")
            textResult = textResult.replace(u"Ø","OE")
            textResult = textResult.replace(u"å","aa")
            textResult = textResult.replace(u"æ","ae")
            textResult = textResult.replace(u"ø","oe")

            #print textResult

            if (len(textResult) > 0):
                textBlocks = printResult.text.split("[")
                first=True

                for textBlock in textBlocks:
                    #print "block "+textBlock

                    if first: # don't try to parse control code from first text block because it doesn't have one
                        print textBlock
                        if len(textBlock)>1:
                            if havePrinter: Epson._raw(textBlock.encode('utf-8')) # printer hates null strings
                        first = False
                    else:
                        c = textBlock[0] # first character is formatting command
                        text = textBlock[1:]
                        #print "control "+c+" text "+text
                        if (c == 'B'):
                            if havePrinter: Epson.set(type="B")
                            #print "<b>"
                        elif (c == 'b'):
                            if havePrinter: Epson.set(type="normal")
                            #print "</b>"
                        elif (c == 'U'):
                            if havePrinter: Epson.set(type="U")
                            #print "<u>"
                        elif (c == 'u'):
                            if havePrinter: Epson.set(type="normal")
                            #print "</u>"
                        elif (c == 'D'):
                            if havePrinter: Epson.set(width=2, height=2)
                            #print "<double>"
                        elif (c == 'd'):
                            if havePrinter: Epson.set(width=1, height=1)
                            #print "</double>"
                        elif (c == 'F'):
                            if havePrinter: Epson._raw('\x0a')
                            #print "Feed"
                        elif (c == 'A'):
                            if havePrinter: Epson._raw('\x5B')
                        elif (c == 'a'):
                            if havePrinter: Epson._raw('\x7B')
                        elif (c == 'O'):
                            if havePrinter: Epson._raw('\x5C')
                        elif (c == 'o'):
                            if havePrinter: Epson._raw('\x7C')
                        elif (c == 'E'):
                            if havePrinter: Epson._raw('\x5D')
                        elif (c == 'e'):
                            if havePrinter: Epson._raw('\x7D')
                        else:
                            print "<Bad formatting code ["+c+" >"

                        if len(text)>0:
                            if havePrinter: Epson._raw(text) # print out the text

            if havePrinter: Epson.cut()
            # repeat above for each copy

        # and sound buzzer

        GPIO.output(buzzer, GPIO.LOW)
        time.sleep(buzzerTime)
        GPIO.output(buzzer, GPIO.HIGH)

        ordersToConfirm.put(order) # if successful. if failed, add back to queue to print
        ordersToPrint.task_done()

def confirmOrders(ordersToPrint, ordersToConfirm):
    "worker thread to send confirmation message"
    if (testMode):
        print "starting confirm worker in test mode"
    else:
        print "starting confirm worker"

    while True:
        order = ordersToConfirm.get()

        if (not testMode):
            print "confirming order "+str(order)+"."
            url = "http://"+server+setPage;
            payload = {'sc': securityCode, "o": order, "s":"processing"};
            printResult = requests.get(url, params=payload);

        ordersToConfirm.task_done()

# Now do the work:

print "Starting polling"

pollWorker = Thread(target=pollForUpdates, args=(ordersToPrint, ordersToConfirm))
pollWorker.setDaemon(True)
pollWorker.start()

printWorker = Thread(target=printOrders, args=(ordersToPrint, ordersToConfirm))
printWorker.setDaemon(True)
printWorker.start()

confirmWorker = Thread(target=confirmOrders, args=(ordersToPrint, ordersToConfirm))
confirmWorker.setDaemon(True)
confirmWorker.start();

while True:
    time.sleep(60)
    #print "bored"

# sit back and relax while the workers work

