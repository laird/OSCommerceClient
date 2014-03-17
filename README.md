OSCommerceClient
================

Project with Bo


This project creates an printer that can print out orders from oscommerce (oscommerce.com)


It will pool a php file for a list of pending orders every 30 sec, if any it will print it on a thermal printer and set the status to processing, every 12 hours or so it will set them all to done, there is a free choice to let the customer know or not.

Short Explanation of the PHP files:

arduino1.php will list all pending orders in the shop, you must add the security code to the url like this: ?sc=1234
arduino2.php will list all orders under processing, security code must also be added
arduino3.php will list the order data of a given order. add secuity code and order number like this ?sc=1234&o=xxxx
arduino4.pgp will update the order. add security code, order number and status you want to update to like this: ?sc=1234&o=xxxx&s=processing or done
arduino5.php does the same as above BUT IT DOES NOT notify the customer.
define_sc.php calculates the md5 hash of the security code you enter, so you enter the code you want (ex. 1234) and it tells you what md5 checksum to put inside all the files above.

Quick install guide for the Raspberry Pi:
-----------------------------------------

format the sd card and noobs to it, boot and install wheezy and after boot do these commands:
------------------
sudo apt-get update
sudo apt-get upgrade
sudo rpi-update
sudo reboot
------------------
sudo apt-get install python-imaging python-serial python-setuptools python python-dev libfreetype6-dev python-pip python-requests python-rpi.gpio
------------------
sudo easy_install -U distribute
------------------
wget http://downloads.sourceforge.net/project/pyusb/PyUSB%201.0/1.0.0-beta-1/pyusb-1.0.0b1.zip
unzip pyusb-1.0.0b1.zip
cd pyusb-1.0.0b1/
python setup.py build
sudo python setup.py install
------------------
git clone https://github.com/lincolnloop/python-qrcode
cd python-qrcode/
python setup.py build
sudo python setup.py install
------------------
wget http://python-escpos.googlecode.com/files/python-escpos-1.0.tgz
tar zxvf python-escpos-1.0.tgz
cd python-escpos-1.0/
python setup.py build
sudo python setup.py install
------------------
wget https://raw.github.com/laird/OSCommerceClient/8d304fdeaf8d44af90978ae7850c6005fa0f56fd/PythonClient/OSCommerceClient.py
------------------
if you want Dynamic dns (good for remote access etc) i use the dns4e.com service

crontab -e
add: */10 * * * *   curl 'https://api.dns4e.com/v7/*******************.dns4e.net/a' --user '******apikey1*****************:*****************apikey2*****************' --data ''
------------------
touch start.sh
sudo chmod +x start.sh
add: sudo python /home/pi/OSCommerceClient.py
------------------
sudo nano /etc/rc.local
add before exit 0: # ./home/pi/start.sh
------------------
sudo nano /etc/inittab

T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100

Change to

#T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100

the above lines disables login, kind of autologin for windows

------------------
sudo nano /boot/cmdline.txt

find the line : dwc_otg.lpm_enable=0 console=ttyAMA0,115200 kgdboc=ttyAMA0,115200 console=tty1 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline rootwait

remove the block of console parameters in the middle: dwc_otg.lpm_enable=0 console=tty1 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline rootwait

The above disables boot messages on the serial line, thermal paper is not free :-D


when you know its working then you can remove the # in the rc.local line

a work in progress still, will update as we get along


/bo
