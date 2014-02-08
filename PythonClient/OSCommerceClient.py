__author__ = 'laird'

# uses requests from http://docs.python-requests.org/en/latest/user/install/#distribute-pip

import logging

import requests

# configuration

server = "87.51.52.114"
securityCode = "1234"
pollPage = "/arduino1.php"
detailPage = "" # set to retrieve detail to print
setPage = "" # set to set status of an order

# Standard

waitPollForOrders = 10000
maxNumToPrint = 5

# setup

logger = logging.getLogger()

# poll for update

url = "http://"+server+pollPage
payload = {'sc': securityCode}
pollResult = requests.get(url, params=payload);
logger.info("poll for orders")
logger.debug("received "+pollResult.text)