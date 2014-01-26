OSCommerceClient
================

Project with Bo


This project creates an printer that can print out orders from oscommerce (oscommerce.com)


It will pool a php file for a list of pending orders every 30 sec, if any it will print it on a thermal printer and set the status to processing, every 12 hours or so it will set them all to done, there is a free choice to let the customer know or not.

This is just a quick and dirty STAGE 1. It's made to be fast to use in a pizza delivery.


STAGE 2, will add a LCD display and 3 buttons.

When an order comes in it will start a buzzer, user then press ok and order prints and customer are notified that oder is processing.

LCD will show orders under processing.

When done user selects the order with up and down buttons and then selects it with ok, it will ask if complete and a press with ok will tell the customer the order is done and it should be there shortly

Short Explanation of the PHP files:

arduino1.php will list all pending orders in the shop, you must add the security code to the url like this: ?sc=1234
arduino2.php will list all orders under processing, security code must also be added
arduino3.php will list the order data of a given order. add secuity code and order number like this ?sc=1234&o=xxxx
arduino4.pgp will update the order. add security code, order number and status you want to update to like this: ?sc=1234&o=xxxx&s=processing or done
arduino5.php does the same as above BUT IT DOES NOT notify the customer.
define_sc.php calculates the md5 hash of the security code you enter, so you enter the code you want (ex. 1234) and it tells you what md5 checksum to put inside all the files above.

a work in progress still, will update as we get along


/bo
