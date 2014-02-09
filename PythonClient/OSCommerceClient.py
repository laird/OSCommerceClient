#!/usr/bin/python

__author__ = 'laird'
#
# OSCommerceClient.py
#
# Polls an OSCommerce site and pulls down a list of pending orders, prints them, and sets their status to 'processing'
#
# uses requests from http://docs.python-requests.org/en/latest/user/install/#distribute-pip

import logging
import requests
import Queue
from threading import Thread
import time
import argparse

# configuration

server = "87.51.52.114"
securityCode = "1234"
pollPage = "/arduino1.php" # poll for orders
detailPage = "/arduino3.php" # get text of receipt to print
setPage = "/arduino4.php" # set status of an order

# Standard

waitPollForOrders = 30 # wait 30 seconds between polls
maxNumToPrint = 5

# parse command line arguments

parser = argparse.ArgumentParser(description='This is the OSCommerce client by laird.', epilog="set either poll or reset, not both.")
group = parser.add_mutually_exclusive_group()
group.add_argument('-p','--poll', metavar='seconds', type=int, default=30, help='poll for new orders')
group.add_argument('-r','--reset', action='store_true', help='reset orders to pending')
args = parser.parse_args()

if args.poll: waitPollForOrders=args.poll

# setup

#order = ""
#orders = []

# queue of orders received in polling that need to be printed
ordersToPrint = Queue.Queue()
# queue of orders that were printed that need to be set to status 'processing'
ordersToConfirm = Queue.Queue();

# poll for orders, add any pending to queue for processing

def pollForUpdates(ordersToPrint, ordersToConfirm):
    "Poll for pending orders, add them to queue to print"
    print "starting poll worker"
    while True:
        print "poll for orders"

        url = "http://"+server+pollPage;
        payload = {'sc': securityCode};
        pollResult = requests.get(url, params=payload);

        orders=pollResult.text.split(',');

        for order in orders:
            # strip out formatting
            order = order.replace("[",""); # ignore starting [
            order = order.replace("<br />",""); # ignore break between lines
            #order = order.replace("OK]",""); # ignore end OK]

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
                else:
                    logging.warn("Found bad order "+order);

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

        print printResult.text

        ordersToConfirm.put(order) # if successful. if failed, add back to queue to print
        ordersToPrint.task_done()

def confirmOrders(ordersToPrint, ordersToConfirm):
    "worker thread to send confirmation message"
    print "starting confirm worker"
    while True:
        order = ordersToConfirm.get()
        print "confirming order "+order+"."

        url = "http://"+server+setPage;
        payload = {'sc': securityCode, "o": order, "s":"processing"};
        printResult = requests.get(url, params=payload);
        ordersToConfirm.task_done()

if args.reset:
    print "Sorry, I don't know how to reset orders yet."

else:
    if args.poll:
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
            print "bored"

# sit back and relax while the workers work

# do everything here
# while True:
#     # top priority confirm an order
#     while not ordersToConfirm.empty():
#         confirmOrder(ordersToConfirm, ordersToPrint)
#         # second priority print an order
#         while not ordersToPrint.empty():
#             printOrder(ordersToPrint)
#             # lowest priority poll for orders
#     pollForUpdates(ordersToPrint)

