#!/usr/bin/python

import os
import urllib2
import json
import re
import IPModems
import os

class Zte(IPModems.IPModems):

    def __init__(self, USB):
        IPModems.IPModems.__init__(self, USB)
        self.modem_str = 'zte'
        self.timer = 3
        self.connected = 0

    def connect(self):
        gwfname = "/tmp/%s_gip" % self.ifname
        if not os.path.isfile(gwfname):
            return
        gwfile = open(gwfname)
        self.IP = gwfile.readline()
        # This is how ZTE modem connects to the net
        try:
            response = urllib2.urlopen('http://' + self.IP + '/goform/goform_set_cmd_process?goformId=CONNECT_NETWORK', timeout=5)
            self.log('Modem %s connect status %s\n' % (self.USB, response))
            self.connected = 1
        except:
            self.log('Unable to connect modem %s\n' % self.USB)
            pass

    def get_static_values(self):
        # Set the values
        self.linkid = self.mac
        self.supported_technologies = '4G'
        self.connection_status = ""
        self.activation_status = ""
        self.isp_name = ""
        self.modem_name = 'ZTE'
        self.modem_version = ""

    def get_dynamic_values(self):
        # Try connecting if last attempt failed
        if not self.connected:
            self.connect()
        # Now go fetch signal strenght etc..
        rssi = sspercentage = ""
        rxvalue = txvalue = ""
        try:
            response = urllib2.urlopen('http://' + self.IP + '/goform/goform_get_cmd_process?cmd=signalbar', timeout=5)
            self.log('Modem %s signal status %s\n' % (self.USB, response))
            values = response.read()
            jsonparams = json.loads(values)
            sspercentage = int(jsonparams['signalbar'])*25
            if sspercentage > 100:
                sspercentage = 100
            self.log('Modem %s signal status json %s\n' % (self.USB, sspercentage))
        except:
            sspercentage = 0
            self.log('modem %s unable to retrieve sspercent' % self.USB)
            pass
        try:
            response = urllib2.urlopen('http://' + self.IP + '/goform/goform_get_cmd_process?cmd=lte_rssi', timeout=5)
            self.log('Modem %s lte_rssi status %s\n' % (self.USB, response))
            values = response.read()
            jsonparams = json.loads(values)
            rssi = jsonparams['lte_rssi']
            self.log('Modem %s lte_rssi status json %s\n' % (self.USB, rssi))
        except:
            rssi = 0
            self.log('modem %s unable to retrieve rssi' % self.USB)
            pass

        rxvalue = int(os.popen('cat /sys/class/net/%s/statistics/rx_bytes' % self.ifname).read())
        txvalue = int(os.popen('cat /sys/class/net/%s/statistics/tx_bytes' % self.ifname).read())

        # Set the values
        self.signal_strength = rssi
        self.signal_percentage = sspercentage
        self.rx_session_bytes = rxvalue
        self.tx_session_bytes = txvalue

    def getRHSvalue(self, line):
        line = line.split('=')[1].replace('\"', '')
        line = line.replace(';', '')
        return line
