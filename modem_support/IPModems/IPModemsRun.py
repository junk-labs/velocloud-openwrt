#!/usr/bin/python

import sys
import IPModems
import logging
import time

def supported_modems():
    logging.debug("================");
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
    logging.debug("Supported generic:")
    logging.debug("================");
    for typestr, plugin in supported_generic_type_list.iteritems():
        logging.debug("  " + typestr + ": " + plugin + " plugin")
    logging.debug("================");


#
# Main program starts here!
#
USB = sys.argv[1]

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

# This is a list of generic implementations that may be used for some
# modem types
supported_generic_type_list = {
    'huawei-ncm' : 'huawei',
    'qmihybrid'  : 'qmihybrid'
}

supported_modems() #print the supported modems
modemobj = IPModems.IPModems(USB)
modemobj.set_link_id()

while True:
    try:
        found_plugin         = ''
        found_plugin_id      = ''
        device_vidpid        = modemobj.product
        device_vendor_string = modemobj.manufacturer
        device_type_string   = modemobj.modemtype

        # Look for plugin to use based on VID/PID
        for vidpid, plugin in supported_vidpid_list.iteritems():
            if device_vidpid.find(vidpid) >= 0:
                found_plugin = plugin
                found_plugin_id = vidpid
                break

        # Look for plugin to use based on vendor string
        if not found_plugin:
            for vendor_string, plugin in supported_vendor_string_list.iteritems():
                if device_vendor_string.find(vendor_string) >= 0:
                    found_plugin = plugin
                    found_plugin_id = vendor_string
                    break

        # Some default plugins we can fallback to
        if not found_plugin:
            for typestr, plugin in supported_generic_type_list.iteritems():
                if device_type_string.find(typestr) >= 0:
                    found_plugin = plugin
                    found_plugin_id = device_type_string
                    break;

        # Error out if no plugin really found
        if not found_plugin:
            logging.warning("No plugin found for device (manufacturer %s, vidpid %s, type %s)", device_vendor_string, device_vidpid, device_type_string)
            sys.exit(0)

        obj_name = found_plugin.capitalize()
        exec "from %s import %s" %(found_plugin, obj_name)
        exec "modemobj = %s('%s')" %(obj_name, USB)
        try:
            logging.debug("[%s], setting up device", found_plugin_id);
            modemobj.setup()
            logging.warning("[%s], starting collecting info", found_plugin_id);
            modemobj.monitor()
            logging.warning("[%s], info collection completed", found_plugin_id);
            modemobj.teardown()
        except NameError:
            logging.error("NameError..");

    except (SystemExit, AttributeError):
        # Explicitly run teardown before exiting...
        modemobj.teardown()
        logging.warning("Exiting Program due to interrupt")
        sys.exit(1)
    except Exception as e:
        logging.exception("Sleeping due to exception ")
        time.sleep(1)
