#!/usr/bin/python

import logging
import commands
import IPModems

class Qmihybrid(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'qmihybrid'
		self.timer = 3
		self.connection_status = 'disconnected'
		self.connection_errors = 0

		# If we have 10 consecutive checks (~30s) without a proper connection, we request a hard reset
		self.hard_reset_threshold = 10

	def runcmd(self, cmd):
		return commands.getstatusoutput(cmd)[1].strip()

	def reload_connection_status(self):
		try:
			# Support auto-connected devices
			if self.product == '1410:9022':
				logging.debug("[dev=%s]: QMI hybrid modem is set to autoconnect...", self.USB)
				self.connection_status = 'connected'
				return

			# Query the modem
			self.connection_status = 'disconnected'
			cmd = "gcom -d " + self.device + " -s /etc/gcom/qmihybridstatus.gcom | grep ' CONNECTED'"
			cmdout = self.runcmd(cmd).strip()
			if cmdout:
				self.connection_status = 'connected'
			else:
				cmd = "gcom -d " + self.device + " -s /etc/gcom/qmihybridstatus.gcom | grep 'QMI_WDS_PKT_DATA_CONNECTED'"
				cmdout = self.runcmd(cmd).strip()
				if cmdout:
					self.connection_status = 'connected'
		except RuntimeError:
			logging.warning("[dev=%s]: couldn't load connection status", self.USB)

	def reconnected(self):
		# Clear connection errors once connected
		self.connection_errors = 0

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
		if not self.linkid:
			self.linkid = self.runcmd(cmd + " | awk '/ESN:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		self.isp_name	   = ''
		self.activation_status = 'activated'

		# Support autoconnected devices (i.e. already connected when started)
		self.reload_connection_status()
		logging.debug("[dev=%s]: connection status (initial): %s", self.USB, self.connection_status);
		if self.connection_status == 'connected':
			self.reconnected()

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
			# Flag a new connection error detected
			self.connection_errors += 1

			# If we have WWAN net info, bring it down
			logging.debug("[dev=%s]: setting %s device down...", self.USB, self.ifname)
			self.runcmd("/usr/sbin/ip link set dev " + self.ifname + " down")

			# Launch connection
			logging.debug("[dev=%s]: restarting connection...", self.USB)
			cmd = "MODE=\"AT$NWQMICONNECT=,,\" gcom -d " + self.device + " -s /etc/gcom/setmode.gcom"
			self.runcmd(cmd)

			self.reload_connection_status()
			if self.connection_status == 'connected':
				self.reconnected()
				logging.warning("[dev=%s]: reconnected", self.USB)
			else:
				logging.warning("[dev=%s]: couldn't reconnect", self.USB)

		if self.connection_errors == self.hard_reset_threshold:
			logging.warning("[dev=%s]: too many connection errors (%d): requesting hard reset", self.USB, self.connection_errors)

			# Modem was working, now it seems to be disconnected, I have seen
			# at times that when modem goes from connected to disconnected state
			# after being connected for long, it doesnt help trying an NWQMICONNECT
			# again, it really needs a knock on the head and then a NWQMICONNECT
			cmd = "gcom -d " + self.device + " -s /etc/gcom/atreset.gcom"
			self.runcmd(cmd)
			# After a reset we get a hotplug remove event, so there is no question
			# of "going back" or anything here, but just code assume we will go back
			# to the beginning of the while loop
			self.connection_errors = 0
