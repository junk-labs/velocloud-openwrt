#!/usr/bin/python

import time
import logging
import json
import os
import fcntl
import math
import subprocess
import socket
import struct
import datetime

# This signal strength table is obtained from rssi/cinr mapping
# Source:
# https://chromium.googlesource.com/chromiumos/platform/wimax_manager/+/7a6a547768c14afd73bde5d18ca7a3eb11ec3a85%5E!/
SignalStrengthTbl = [
    [0,  0,  0,  0,  0,  0],
    [0,  0,  0, 20, 20,  40],
    [0,  0, 20, 20, 40,  60],
    [0, 20, 20, 40, 60,  80],
    [0, 20, 40, 60, 80, 100],
]


def LTESignalStrengthPercentage(rsrp):
    rsrp = int(rsrp)
    val = (rsrp + 70) * 2
    percentage = 100 + (int(math.floor(val / 20.0) * 20))
    if(percentage < 0):
        percentage = 0
    elif(percentage > 100):
        percentage = 100
    return percentage

def SignalStrengthPercentage(rssi, cinr):
    row = 4
    column = 5
    rssi = int(rssi)
    cinr = int(cinr)

    if (rssi <= -80):
       row = 0
    elif(rssi <= -75):
       row = 1
    elif(rssi <= -65):
       row = 2
    elif(rssi <= -55):
       row = 3

    if (cinr <= -3):
       column = 0
    elif(cinr <= 0):
       column = 1
    elif(cinr <= 3):
       column = 2
    elif(cinr <= 10):
       column = 3
    elif(cinr <= 15):
       column = 4

    return str(SignalStrengthTbl[row][column])


class IPModems():

    def __init__(self, USB):
        self.USB = USB
        self.dontping = 0
        # Traces disabled by default
        self.notraces = 1
        # self.linkid
        # self.timer, self.modem_str
        # self.modem_name, self.modem_version
        # self.isp_name, self.server_name,
        # self.supported_technologies
        # self.connection_status, self.activation_status,
        # self.signal_strength, self.signal_percentage
        # self.ip_value, self.gateway_value, self.dns_value,
        # self.rx_session_bytes, self.tx_session_bytes,
        # self.rx_cumulative_bytes, self.tx_cumulative_bytes,

        # Enable the things if necessary, and the functions
        # self.rx_session_packets, self.tx_session_packets,
        # self.rx_cumulative_packets, self.tx_cumulative_packets
        logging.debug("[dev=%s]: Getting Info", self.USB);
        self.modem_path = {}
        self.print_pid()
        self.get_modem_variables()
        self.get_modem_chip_manufacturer()
        self.get_modem_product()
        self.get_modem_type()
        self.get_modem_device()
        self.get_modem_ifname()
        self.get_modem_ifmac()

        self.json_file_open()

    def setup(self):
        logging.debug("[dev=%s]: setup", self.USB)

    def teardown(self):
        logging.debug("[dev=%s]: teardown", self.USB)

    def log(self, contents):
        logging.warning('%s: %s' % (self.USB, contents))

    def runcmd(self, cmd):
	return commands.getstatusoutput(cmd)[1].strip()

    def get_uci_value(self, name):
        path = self.modem_path['modem_path']
        cmdget = "uci -c " + path + " get modems." + self.USB
        pipe = os.popen(cmdget + "." + name)
        value = ""
        try:
            value = pipe.readline().strip()
            logging.debug("[dev=%s]: Got the value for %s = %s", self.USB, name, value)
        except:
            logging.debug("[dev=%s]: Unable to get the value for %s", self.USB, name)
        return value

    def json_file_open(self):
        filename = "/tmp/USB/" + self.USB + '_periodic.txt'
        try:
            logging.debug("[dev=%s]: open json modem info file %s", self.USB, filename)
            self.jsonfile = open(filename, "w")
            self.jsonfile.close()
            self.jsonfile = open(filename, "r+")
        except:
            logging.warning("[dev=%s]: Unable to open json modem info file %s", self.USB,filename)

    def ping_test(self, max_tries, address):
        with open(os.devnull, "wb") as limbo:
            tries = 1
            while tries <= max_tries:
	        try:
                    result=subprocess.Popen(["ping", "-I", self.ifname, "-c", "1", "-W", "2", address],
                               stdout=limbo, stderr=limbo).wait()
                    if not result:
		        time.sleep(1)
                        return 0, tries
                except:
                    pass
                tries = tries + 1
                time.sleep(1)
        return 1, tries

    def print_pid(self):
        filew = open("/tmp/USB/" + self.USB + ".pid", "w")
        filew.write(str(os.getpid()))
        filew.close()

    def get_modem_variables(self):
        filer = open('/etc/modems/modem.path')
        for line in filer.readlines():
            self.modem_path[line.split('=')[0]] = line.split('=')[1].strip()
        filer.close()

    def get_modem_chip_manufacturer(self):
        self.manufacturer = self.get_uci_value("manufacturer")

    def get_modem_product(self):
        self.product = self.get_uci_value("product")

    def get_modem_type(self):
        self.modemtype = self.get_uci_value("type")

    def get_modem_device(self):
        self.device = self.get_uci_value("device")

    def get_modem_ifname(self):
        self.ifname = self.get_uci_value("ifname")

    def get_modem_ifmac(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        info = fcntl.ioctl(s.fileno(), 0x8927,  struct.pack('256s', self.ifname[:15]))
        self.mac = ''.join(['%04d' % ord(char) for char in info[18:24]])

    def get_attrib_xmlpath(self, root, path, index=0):
        return root.findall(path)[index]

    def get_static_values(self):
        pass # It will be inherited by derived class

    def get_dynamic_values(self):
        pass # It will be inherited by derived class

    def set_modem_status(self):
        path = self.modem_path['modem_path']
        cmd = "uci -c " + path + " set modems." + self.USB
        os.system(cmd + ".status='CONNECTED SUCCESSFULLY'")
        os.system("uci -c " + path + " commit modems")

    def set_static_values_uci(self):
        # This has to be in uci network file as its one time config
        path = self.modem_path['modem_path']
        cmd = "uci -c " + path + " set modems." + self.USB

        modem_manufacturer = self.modem_name + " " + self.modem_version
        if self.modem_name:
            os.system(cmd + ".modelnumber='" + modem_manufacturer + "'")
        if self.linkid:
            os.system(cmd + ".linkid='" + self.linkid.lower() + "'")
        if self.isp_name:
            os.system(cmd + ".isp='" + self.isp_name + "'")
        if self.activation_status:
            os.system(cmd + ".activated='" + self.activation_status + "'")
        os.system("uci -c " + path + " commit modems")

    def set_dynamic_values_uci(self):
        # This is tempory solution to update to uci network file
        # Shared memory or the json print text ????
        path = self.modem_path['modem_path']
        cmd = "uci -c " + path + " set modems." + self.USB

        if self.signal_strength:
            os.system(cmd + ".sigstrength='" + self.signal_strength + "'")
        if self.signal_percentage:
            os.system(cmd + ".sigpercentage='" + self.signal_percentage + "'")
        if self.rx_session_bytes:
            os.system(cmd + ".rxbytes='" + self.rx_session_bytes + "'")
        if self.tx_session_bytes:
            os.system(cmd + ".txbytes='" + self.tx_session_bytes + "'")
        os.system("uci -c " + path + " commit modems")

    def set_dynamic_values_json(self):
        jsonObj = {}
        jsonObj['SigStrength'] = self.signal_strength
        jsonObj['SigPercentage'] = self.signal_percentage
        jsonObj['Rxbytes'] = self.rx_session_bytes
        jsonObj['Txbytes'] = self.tx_session_bytes
        self.jsonfile.seek(0)
        fcntl.flock(self.jsonfile.fileno(), fcntl.LOCK_EX)
        self.jsonfile.truncate()
        self.jsonfile.write(json.dumps(jsonObj))
        self.jsonfile.flush()
        os.fsync(self.jsonfile.fileno())
        fcntl.flock(self.jsonfile.fileno(), fcntl.LOCK_UN)
        # Have to take care on signal handling to proper close
        # self.jsonfile.close() # Have to take care on signal handling

    def write_traces(self):
        # Do nothing if disabled
        if self.notraces:
            return

        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # Iface address test
        cmd = "ip -f inet -o addr show " + self.ifname + " 2>/dev/null | cut -d\\  -f 7 | cut -d/ -f 1"
        ipaddr = self.runcmd(cmd)
        if not ipaddr:
            ipaddr = "no IP"
            logging.debug("[dev=%s,traces]: no IP address set", self.USB)
        else:
            logging.debug("[dev=%s,traces]: IP address set: %s", self.USB, ipaddr)

        # Ping test
        ret, tries = self.ping_test(1, '8.8.8.8')
        if ret != 0:
            pingtest = "ping failed"
            logging.debug("[dev=%s,traces]: ping test failed", self.USB)
        else:
            pingtest = "ping success"
            logging.debug("[dev=%s,traces]: ping test succeeded", self.USB)

        # Signal quality
        if self.signal_strength:
            signalstrength = self.signal_strength
        else:
            signalstrength = "??"
        if self.signal_percentage:
            signalpercentage = self.signal_percentage
        else:
            signalpercentage = "??"

        # Write to file
        with open('/tmp/USB/' + self.USB + '_ipmodem_traces.log', "a") as myfile:
            trace = "[%s] %s, %s, %s, %s dBm, %s%%\n" % (timestamp, self.ifname, ipaddr, pingtest, signalstrength, signalpercentage)
            myfile.write(trace)

    def monitor(self):
        logging.warning("[dev=%s]: Started monitoring device", self.USB)
        if self.modemtype == "cdcether" and not self.dontping:
            time.sleep(3) # Always takes 3 sec to get IP
            ret, tries = self.ping_test(10, self.IP)
            if ret != 0:
                logging.warning("[dev=%s]: Ping Failed to %s", self.USB, self.IP)
            else:
                logging.warning("[dev=%s]: Ping test succeeded : tries %s", self.USB, str(tries))
                self.set_modem_status()

        self.get_static_values()
        self.set_static_values_uci()
        while True:
            self.get_dynamic_values()
            self.set_dynamic_values_json()
            self.write_traces()
            logging.debug("[dev=%s]: Waiting for %d secs to collect next info", self.USB, self.timer)
            time.sleep(self.timer)

    def set_link_id(self):
        path = self.modem_path['modem_path']
        cmd = "uci -c " + path + " set modems." + self.USB
        os.system(cmd + ".linkid='" + self.product.lower() + self.USB.lower() + "'")
        os.system("uci -c " + path + " commit modems")


'''
    def get_supported_technologies(self):
        return self.supported_technologies

    def get_connection_status(self):
        return self.connection_status

    def get_activation_status(self):
        return self.activation_status

    def get_signal_strength(self):
        return self.signal_strength

    def get_isp_name(self):
        return self.isp_name

    def get_server_name(self):
        return self.server_name

    def get_ip_value(self):
        return self.ip_value

    def get_gateway_value(self):
        return self.gateway_value

    def get_dns_value(self):
        return self.dns_value

    def get_rx_session_bytes(self):
        return self.rx_session_bytes

    def get_tx_session_bytes(self):
        return self.tx_session_bytes

    def get_rx_session_packets(self):
        return self.rx_session_packets

    def get_tx_session_packets(self):
        return self.tx_session_packets

    def get_rx_cumulative_bytes(self):
        return self.rx_cumulative_bytes

    def get_tx_cumulative_bytes(self):
        return self.tx_cumulative_bytes

    def get_rx_cumulative_packets(self):
        return self.rx_cumulative_packets

    def get_tx_cumulative_packets(self):
        return self.tx_cumulative_packets
'''
