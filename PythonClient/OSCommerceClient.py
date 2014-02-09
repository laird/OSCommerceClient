__author__ = 'laird'

# uses requests from http://docs.python-requests.org/en/latest/user/install/#distribute-pip

import logging
import requests
import Queue
from threading import Thread
import time

# configuration

server = "87.51.52.114"
securityCode = "1234"
pollPage = "/arduino1.php" # poll for orders
detailPage = "/arduino3.php" # get text of receipt to print
setPage = "/arduino4.php" # set status of an order

# Standard

waitPollForOrders = 30 # wait 30 seconds between polls
maxNumToPrint = 5

# setup

order = ""
orders = []

# queue of orders received in polling that need to be printed
ordersToPrint = Queue.Queue()
# queue of orders that were printed that need to be set to status 'processing'
ordersToConfirm = Queue.Queue();

# poll for orders, add any pending to queue for processing

def pollForUpdates(ordersToPrint):
    "Poll for pending orders, add them to queue to print"
    logging.info("poll for orders")

    url = "http://"+server+pollPage;
    payload = {'sc': securityCode};
    pollResult = requests.get(url, params=payload);

    orders=pollResult.text.split(',');

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
                logging.info("que order "+str(orderNum));
                ordersToPrint.put(orderNum)
            else:
                logging.debug("skip order "+str(orderNum));
        else:
            if order=="OK]":
                logging.debug("end of list");
            else:
                logging.warn("Found bad order "+order);

    logging.debug("received "+str(len(orders)-1)+" orders.")

def printOrders(ordersToPrint, ordersToConfirm):
    "print a queued order, then queue it to confirm"
    order = ordersToPrint.get()
    logging.info("printing order "+order+".")

    url = "http://"+server+detailPage;
    payload = {'sc': securityCode, "o": order};
    printResult = requests.get(url, params=payload);

    logging.info("Print: "+printResult.text)

    ordersToConfirm.put(order) # if successful. if failed, add back to queue to print

def confirmOrders(ordersToConfirm):
    "send confirmation message"
    order = ordersToConfirm.get()
    logging.info("confirming order "+order+".")

# do everything here
while True:
    # top priority confirm an order
    while not ordersToConfirm.empty():
        confirmOrder(ordersToConfirm, ordersToPrint)
        # second priority print an order
        while not ordersToPrint.empty():
            printOrder(ordersToPrint)
            # lowest priority poll for orders
    time.sleep(waitPollForOrders)
    pollForUpdates(ordersToPrint)

