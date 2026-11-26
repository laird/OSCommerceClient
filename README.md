# OSCommerceClient

A 2014 Raspberry Pi project (with Bo) that polls an [osCommerce](https://www.oscommerce.com/) shop and prints new orders on a thermal printer.

Every 30 seconds the Python client calls a small set of PHP endpoints. If there are pending orders, it prints them and sets the order status to processing. You can choose whether the customer gets an email.

This stack is historical. The install steps below assume Raspberry Pi OS Wheezy, NOOBS, `inittab`, Python 2, and downloads from Google Code and old SourceForge URLs — several of those links are dead. Treat the repo as a reference, not something to install on a current Pi without a rewrite.

## How it works

PHP files in the shop (see `php-files/`):

| Script | Purpose |
|---|---|
| `arduino1.php` | List pending orders. Call with `?sc=YOUR_CODE` |
| `arduino2.php` | List orders in processing. Also requires `sc` |
| `arduino3.php` | Order details: `?sc=YOUR_CODE&o=ORDER_NUMBER` |
| `arduino4.php` | Update status (notifies the customer): `?sc=YOUR_CODE&o=ORDER_NUMBER&s=processing` or `s=done` |
| `arduino5.php` | Same update, but does not email the customer |
| `define_sc.php` | Prints the MD5 hash of a security code so you can paste it into the scripts above |

The client scripts are `OSCommerceClient.py` (print pending orders) and `OSCommerceClient_1.py` (a later variant used to mark old orders done without reprinting).

## Original Pi install (2014, outdated)

The original README walked through: flash Raspbian Wheezy, `apt-get` packages including `python-serial`, `python-imaging`, and `python-rpi.gpio`, then install PyUSB, `python-qrcode`, and `python-escpos` from upstream tarballs, and finally fetch:

```text
https://raw.githubusercontent.com/laird/OSCommerceClient/master/OSCommerceClient.py
https://raw.githubusercontent.com/laird/OSCommerceClient/master/OSCommerceClient_1.py
```

It also disabled the serial getty so the thermal printer could use the UART, started the script from `rc.local`, and used cron to kill and restart the client every 15 minutes because the script sometimes hung. After about six months, SD cards wore out; the notes recommend a read-only root filesystem.

Do not copy the Dynamic DNS crontab examples verbatim — they were written with credential placeholders, and dns4e may no longer be the service you want.

## License

No license file is present. Add one if you intend anyone else to reuse the code.
