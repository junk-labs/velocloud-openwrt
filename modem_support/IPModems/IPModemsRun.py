#!/usr/bin/python

import sys
import signal
import IPModems
import logging
import time

def signal_handler(signal, frame):
    logging.warning(USB + ": Received Signal " + str(signal))
    sys.exit(0)

def supported_modems():
    logging.debug("Supported Modems")
    logging.debug("================");
    for modem in supported_modem_list:
        logging.debug(modem)
    logging.debug("================");

#
# Main program starts here!
#
USB = sys.argv[1]
signal.signal(signal.SIGTERM, signal_handler)
signal.signal(signal.SIGINT, signal_handler)

# Debugging: DEBUG, INFO, WARNING, ERROR, CRITICAL
debugflag = logging.WARNING #logging.DEBUG
debugfile = '/tmp/USB/'+USB+'_ipmodem_info_collector.log'
logging.basicConfig(filename=debugfile, level=debugflag)

supported_modem_list = {
        'Beceem Communications' : 'ubee',
        'Pantech' : 'pantech',
        'ZTE,Incorporated' : 'zte',
        'Sierra Wireless, Incorporated' : 'qmi'
        }

supported_modems() #print the supported modems
modemobj = IPModems.IPModems(USB)
modemobj.set_link_id()

found = 0
while True:
    try:
        manufacturer = modemobj.manufacturer
        for modem in supported_modem_list:
            if manufacturer.find(modem) >= 0:
                found = 1
                modem_str = supported_modem_list[modem]
                modem_ustr = modem_str.capitalize()
                exec "from %s import %s" %(modem_str, modem_ustr)
                exec "modemobj = %s('%s')" %(modem_ustr, USB)
                try:
                    logging.warning("[manufacturer=%s], starting collecting info", modem);
                    modemobj.get_modem_info()
                    logging.warning("[manufacturer=%s], info collection completed", modem);
                except NameError:
                    logging.error("NameError..");
        if not found:
            logging.warning("manufacturer %s not in supported modem list", manufacturer);
            modemobj.set_link_id()
            '''All mandatory values can be set here and implemented in the base
            class'''
            sys.exit(0)
    except (SystemExit, AttributeError):
        logging.warning("Exiting Program due to interrupt")
        sys.exit(1)
    except Exception as e:
        logging.exception("Sleeping due to exception ")
        time.sleep(1)
