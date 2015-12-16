#!/usr/bin/python

import logging
import commands
import IPModems

class Huawei(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'huawei'
		self.timer = 3
                self.connection_status_check_errors = 0
                self.connection_status = 'disconnected'

                # If we have 10 consecutive checks (~30s) without a proper connection, we request assume disconnected
                self.connection_status_check_threshold = 10

	def runcmd(self, cmd):
		return commands.getstatusoutput(cmd)[1].strip()

	def reload_connection_status(self):
		try:
			cmd = "gcom -d " + self.device + " -s /etc/gcom/huaweistatus.gcom"
			new_connection_status = self.runcmd(cmd)
                        if new_connection_status == 'connected':
                                self.connection_status_check_errors = 0
                                self.connection_status = new_connection_status
                                return
                        # If we were already disconnected, nothing to do
                        if self.connection_status == 'disconnected':
                                return
                        # If we're reported disconnected N consecutive times times, flag disconnected
                        self.connection_status_check_errors += 1
                        if self.connection_status_check_errors == self.connection_status_check_threshold:
                                logging.debug("[dev=%s]: too many checks reported disconnection...", self.USB)
                                self.connection_status_check_errors = 0
                                self.connection_status = 'disconnected'
                                return
                        # Otherwise, do nothing else, we don't consider the new state change to
                        # disconnected until the 10th try
                        logging.debug("[dev=%s]: disconnection reported (%d)...", self.USB, self.connection_status_check_errors)

		except RuntimeError:
			logging.warning("[dev=%s]: couldn't load connection status", self.USB)

	def reconnected(self):
		# Make sure WWAN is always up when just connected
		logging.debug("[dev=%s]: setting %s device up...", self.USB, self.ifname)
		self.runcmd("/usr/sbin/ip link set dev " + self.ifname + " up")
		logging.debug("[dev=%s]: reloading network...", self.USB)
		self.runcmd("ubus call network reload")

	def get_static_values(self):
		logging.debug("[dev=%s]: static values...", self.USB)

		cmd = "gcom -d " + self.device + " -s /etc/gcom/getcardinfo.gcom"
		self.modem_name	   = self.runcmd(cmd + " | awk '/Model:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		self.modem_version = self.runcmd(cmd + " | awk '/Revision:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		self.linkid	   = self.runcmd(cmd + " | awk '/IMEI:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		self.isp_name	   = ''
		self.activation_status = 'activated'

                # Disable unsolicited messages that flood the ports
		cmd = "MODE=\"AT^CURC=0\" gcom -d " + self.device + " -s /etc/gcom/setmode.gcom"
		self.runcmd(cmd)

		# Make sure we start disconnected
		self.reload_connection_status()
		logging.debug("[dev=%s]: connection status (initial): %s", self.USB, self.connection_status);
		if self.connection_status == 'connected':
			cmd = "MODE=\"AT^NDISDUP=1,0\" gcom -d " + self.device + " -s /etc/gcom/setmode.gcom"
			self.runcmd(cmd)

	def get_dynamic_values(self):
		logging.debug("[dev=%s]: dynamic values...", self.USB)

		# Signal strength
		cmd = "gcom -d " + self.device + " -s /etc/gcom/getstrength.gcom"
		try:
			value = int(self.runcmd(cmd + " grep '+CSQ:' | awk -F\":|,\" '{print $2}'").strip())
			# 99 means unknown, otherwise value betwen 0 and 31
			if value == 99:
				self.signal_strength = -120
			else:
				self.signal_strength = (-1) * (113 - value - value);
		except:
			self.signal_strength = -120

		# Signal percentage
		#   Marginal - Levels of -95dBm or lower
		#   Workable under most conditions - Levels of -85dBm to -95dBm
		#   Good - Levels between -75dBm and -85dBm
		#   Excellent - levels above -75dBm.
		self.signal_percentage = 0
		if self.signal_strength < -95:
			self.signal_percentage = 25
		elif self.signal_strength < -85:
			self.signal_percentage = 50
		elif self.signal_strength < -75:
			self.signal_percentage = 75
		else:
			self.signal_percentage = 100

		# Statistics
		self.rx_session_bytes = self.runcmd("ifconfig " + self.ifname + " | awk	 '/RX bytes:/ { split($2, a, /:/); print a[2]}'")
		self.tx_session_bytes = self.runcmd("ifconfig " + self.ifname + " | awk	 '/TX bytes:/ { split($6, a, /:/); print a[2]}'")

		# Flag to decide whether we need to bring iface down and perform reconnection
		wwan_down = False

		# Connection status
		self.reload_connection_status()
		if self.connection_status != 'connected':
			logging.warning("[dev=%s]: not connected in network", self.USB)
			wwan_down = True

		# Do we need to force a full explicit disconnection of all our state info?
		if wwan_down:
			# If we have WWAN net info, bring it down
			logging.debug("[dev=%s]: setting %s device down...", self.USB, self.ifname)
			self.runcmd("/usr/sbin/ip link set dev " + self.ifname + " down")

			# Get APN from profile
			myvars = {}
			with open("/tmp/USB/" + self.USB + ".profile") as myfile:
				for line in myfile:
					name, var = line.partition("=")[::2]
					myvars[name.strip()] = var.strip()

			# Launch connection
			logging.debug("[dev=%s]: restarting connection...", self.USB)
			cmd = "MODE=\"AT^NDISDUP=1,1,\\\"" + myvars['APN'] + "\\\"\" gcom -d " + self.device + " -s /etc/gcom/setmode.gcom"
			self.runcmd(cmd)

			self.reload_connection_status()
			if self.connection_status == 'connected':
				self.reconnected()
				logging.warning("[dev=%s]: reconnected", self.USB)
			else:
				logging.warning("[dev=%s]: couldn't reconnect", self.USB)
