#!/usr/bin/python

import logging
import commands
import IPModems
import os
import time

class Qmihybrid(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'qmihybrid'
		self.timer = 3
		self.connection_errors = 0
		self.soft_reset_threshold = 20
		self.hard_reset_threshold = 40


	def get_static_values(self):
		logging.debug("[dev=%s]: static values...", self.USB)

		cmd = "gcom -d " + self.device + " -s /etc/gcom/getcardinfo.gcom"
		self.modem_name = self.runcmd(cmd + " | awk '/Model:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		self.modem_version = self.runcmd(cmd + " | awk '/Revision:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		self.linkid	= self.runcmd(cmd + " | awk '/IMEI:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		if not self.linkid:
			self.linkid = self.runcmd(cmd + " | awk '/ESN:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		self.isp_name = ''
		self.activation_status = 'activated'

		# Autoconnected
		self.setup_network_interface()


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
				self.signal_strength = (-1) * (113 - value - value)
		except:
			self.signal_strength = -120

		# Signal percentage
		#	Marginal - Levels of -95dBm or lower
		#	Workable under most conditions - Levels of -85dBm to -95dBm
		#	Good - Levels between -75dBm and -85dBm
		#	Excellent - levels above -75dBm.
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

		# Check IP address
		logging.debug("[dev=%s]: checking IP settings in device...", self.USB)
		f = os.popen('ifconfig %s | grep "inet\ addr" | cut -d: -f2 | cut -d" " -f1' % self.ifname)
		ipaddress = f.read().rstrip()

		# If modem has had no ip address for a while, reset it
		if (ipaddress == "") or (ipaddress == "192.168.1.2"):
			self.connection_errors += 1
			self.set_modem_status("Disconnected")

			# Only relaunch network interface settings on soft reset
			if self.connection_errors == self.soft_reset_threshold:
				self.teardown_network_interface()
				time.sleep(5)
				self.setup_network_interface()

			# Fully reboot device in hard reset
			if self.connection_errors == self.hard_reset_threshold:
				self.teardown_network_interface()
				time.sleep(5)
				logging.warning("[dev=%s]: too many connection errors (%d), ipaddr (%s): requesting hard reset", self.USB, self.connection_errors, ipaddress)
				cmd = "gcom -d " + self.device + " -s /etc/gcom/atreset.gcom"
				self.runcmd(cmd)
				self.connection_errors = 0
		else:
			self.set_modem_status_connected()
			self.connection_errors = 0
