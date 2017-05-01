#!/usr/bin/python

import time
import logging
import IPModems

class Autoconnected(IPModems.IPModems):

    def __init__(self, USB):
        IPModems.IPModems.__init__(self, USB)
        self.modem_str = 'autoconnected'

        # If no IP set in 120s, we'll teardown and re-setup the interface
        self.timer = 10
        self.ip_check_errors_threshold = 12
        self.ip_check_errors = 0


    def get_static_values(self):
        # Reset to defaults
        self.linkid = 'unknown'
        self.modem_name = 'unknown'
        self.modem_version = 'unknown'
        self.isp_name = 'unknown'
        self.activation_status = 'activated'

        logging.debug("[dev=%s]: setting up interface %s on start...", self.USB, self.ifname)
        self.teardown_network_interface()
        self.setup_network_interface()


    def get_dynamic_values(self):
        # Statistics
        self.signal_strength = -120
        self.signal_percentage = 0
        self.rx_session_bytes = self.runcmd("ifconfig " + self.ifname + " | awk  '/RX bytes:/ { split($2, a, /:/); print a[2]}'")
        self.tx_session_bytes = self.runcmd("ifconfig " + self.ifname + " | awk  '/TX bytes:/ { split($6, a, /:/); print a[2]}'")

        # Try to check whether an IPv4 is assigned to the network interface
        #   ip -f inet -o addr show dev eth4
        #   37: eth4    inet 192.168.8.100/24 brd 192.168.8.255 scope global eth4\       valid_lft forever preferred_lft forever
        #
        logging.debug("[dev=%s]: checking IP settings in autoconnected device...", self.USB)
        cmd = "/usr/sbin/ip -f inet -o addr show dev " + self.ifname + " | /usr/bin/awk '{ print $4 }'"
        ipaddress = self.runcmd(cmd)
        if ipaddress != '':
            self.ip_check_errors = 0
            self.set_modem_status_connected()
            logging.debug("[dev=%s]: IPv4 %s found in interface %s...", self.USB, ipaddress, self.ifname)
        else:
            self.ip_check_errors += 1
            self.set_modem_status("No IPv4 address set")
            logging.debug("[dev=%s]: No IPv4 found in interface %s... (%u/%u errors)", self.USB, self.ifname, self.ip_check_errors, self.ip_check_errors_threshold)

        # If too many errors, teardown interface and set it up again
        if self.ip_check_errors >= self.ip_check_errors_threshold:
            logging.debug("[dev=%s]: reloading network interface %s...", self.USB, self.ifname);
            self.teardown_network_interface()
            time.sleep(5)
            self.setup_network_interface()
            # Reset to retry loop from scratch next time
            self.ip_check_errors = 0
