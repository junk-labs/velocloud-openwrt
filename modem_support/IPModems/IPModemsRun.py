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
    logging.debug("Supported vid/pids:")
    logging.debug("================");
    for vidpid, plugin in supported_vidpid_list.iteritems():
        logging.debug("  " + vidpid + ": " + plugin + " plugin")
    logging.debug("================");
    logging.debug("Supported vendors:")
    logging.debug("================");
    for vendor_string, plugin in supported_vendor_string_list.iteritems():
        logging.debug("  " + vendor_string + ": " + plugin + " plugin")
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

# This is a VID/PID based list
supported_vidpid_list = {
    '106c:3718' : 'uml290'
}

# This is a Vendor string based list
supported_vendor_string_list = {
    'Beceem Communications'         : 'ubee',
    'Pantech'                       : 'pantech',
    'ZTE,Incorporated'              : 'zte',
    'Sierra Wireless, Incorporated' : 'sierra'
}

supported_modems() #print the supported modems
modemobj = IPModems.IPModems(USB)
modemobj.set_link_id()

while True:
    try:
        found_plugin = ''
        found_plugin_id = ''

        # Look for plugin to use based on VID/PID
        device_vidpid = modemobj.product
        for vidpid, plugin in supported_vidpid_list.iteritems():
            if device_vidpid.find(vidpid) >= 0:
                found_plugin = plugin
                found_plugin_id = vidpid
                break

        # Look for plugin to use based on vendor string
        if not found_plugin:
            device_vendor_string = modemobj.manufacturer
            for vendor_string, plugin in supported_vendor_string_list.iteritems():
                if device_vendor_string.find(vendor_string) >= 0:
                    found_plugin = plugin
                    found_plugin_id = vendor_string
                    break

        # Error out if no plugin really found
        if not found_plugin:
            logging.warning("manufacturer %s not in supported modem list", device_vendor_string);
            modemobj.set_link_id()
            '''All mandatory values can be set here and implemented in the base
            class'''
            sys.exit(0)

        obj_name = found_plugin.capitalize()
        exec "from %s import %s" %(found_plugin, obj_name)
        exec "modemobj = %s('%s')" %(obj_name, USB)
        try:
            logging.warning("[%s], starting collecting info", found_plugin_id);
            modemobj.get_modem_info()
            logging.warning("[%s], info collection completed", found_plugin_id);
        except NameError:
            logging.error("NameError..");

    except (SystemExit, AttributeError):
        logging.warning("Exiting Program due to interrupt")
        sys.exit(1)
    except Exception as e:
        logging.exception("Sleeping due to exception ")
        time.sleep(1)
