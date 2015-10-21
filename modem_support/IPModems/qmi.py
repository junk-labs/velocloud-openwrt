#!/usr/bin/python

import logging
import commands
import IPModems

class Qmi(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'qmi'
		self.timer = 3
		self.clicmd = '/usr/bin/qmicli -d ' + self.device + ' --device-open-proxy '
		self.dms_cid = 0
		self.nas_cid = 0
		self.wds_cid = 0
		self.wwan_iface = ''
		self.registration_status = ''
		self.connection_errors = 0
		self.ip_check_errors = 0
		self.expected_ip = ''

		# If we have 100 consecutive IP check errors (~300s), we request a hard reset
		self.ip_check_errors_threshold = 100
		# If we have 20 consecutive checks (~60s) without a proper connection, we request a soft reset
		self.soft_reset_threshold = 20
		# If we have 100 consecutive checks (~300s) without a proper connection, we request a hard reset
		self.hard_reset_threshold = 100

	def runcmd(self, cmd):
		return commands.getstatusoutput(cmd)[1].strip()

	def qmicli(self, cmd):
		return self.runcmd(self.clicmd + cmd)

	def qmicli_dms(self, cmd):
		if self.dms_cid > 0:
			return self.runcmd(self.clicmd + "--client-cid=" + str(self.dms_cid) + " --client-no-release-cid " + cmd)
		else:
			raise RuntimeError('No DMS client allocated')

	def qmicli_nas(self, cmd):
		if self.nas_cid > 0:
			return self.runcmd(self.clicmd + "--client-cid=" + str(self.nas_cid) + " --client-no-release-cid " + cmd)
		else:
			raise RuntimeError('No NAS client allocated')

	def qmicli_wds(self, cmd):
		if self.wds_cid > 0:
			return self.runcmd(self.clicmd + "--client-cid=" + str(self.wds_cid) + " --client-no-release-cid " + cmd)
		else:
			raise RuntimeError('No WDS client allocated')

	def setup(self):
		IPModems.IPModems.setup(self)
		try:
			logging.debug("[dev=%s]: allocating DMS QMI client...", self.USB);
			self.dms_cid = int(self.qmicli("--dms-noop --client-no-release-cid | sed  -n \"s/.*CID.*'\(.*\)'.*/\\1/p\""))
		except:
			logging.warning("[dev=%s]: couldn't allocate DMS QMI client", self.USB);
		try:
			logging.debug("[dev=%s]: allocating NAS QMI client...", self.USB);
			self.nas_cid = int(self.qmicli("--nas-noop --client-no-release-cid | sed  -n \"s/.*CID.*'\(.*\)'.*/\\1/p\""))
		except:
			logging.warning("[dev=%s]: couldn't allocate NAS QMI client", self.USB);
		try:
			logging.debug("[dev=%s]: allocating WDS QMI client...", self.USB);
			self.wds_cid = int(self.qmicli("--wds-noop --client-no-release-cid | sed  -n \"s/.*CID.*'\(.*\)'.*/\\1/p\""))
		except:
			logging.warning("[dev=%s]: couldn't allocate WDS QMI client", self.USB);
		try:
			logging.debug("[dev=%s]: checking WWAN iface", self.USB);
			self.wwan_iface = self.qmicli("--get-wwan-iface")
			logging.debug("[dev=%s]: WWAN iface loaded: %s", self.USB, self.wwan_iface);
		except:
			logging.warning("[dev=%s]: couldn't get WWAN iface name", self.USB);

	def teardown(self):
		IPModems.IPModems.teardown(self)
		if (self.dms_cid > 0):
			logging.debug("[dev=%s]: releasing DMS QMI client %d", self.USB, self.dms_cid);
			self.qmicli("--dms-noop --client-cid=" + str(self.dms_cid))
		if (self.nas_cid > 0):
			logging.debug("[dev=%s]: releasing NAS QMI client %d", self.USB, self.nas_cid);
			self.qmicli("--nas-noop --client-cid=" + str(self.nas_cid))
		if (self.wds_cid > 0):
			logging.debug("[dev=%s]: releasing WDS QMI client %d", self.USB, self.wds_cid);
			self.qmicli("--wds-noop --client-cid=" + str(self.wds_cid))
		if self.wwan_iface:
			logging.debug("[dev=%s]: setting %s device down...", self.USB, self.wwan_iface)
			self.runcmd("/usr/sbin/ip link set dev " + self.wwan_iface + " down")

	def reload_registration_status(self):
		try:
			self.registration_status = self.qmicli_nas("--nas-get-serving-system | sed	-n \"s/.*Registration state.*'\(.*\)'.*/\\1/p\"")
		except RuntimeError:
			logging.warning("[dev=%s]: couldn't load registration status", self.USB)


	def reload_connection_status(self):
		try:
			self.connection_status = self.qmicli_wds("--wds-get-packet-service-status | awk -F\"'\" '/Connection status/ { print $2}'")
		except RuntimeError:
			logging.warning("[dev=%s]: couldn't load connection status", self.USB)

	def reconnected(self):
		# Clear connection errors once connected
		self.connection_errors = 0

		# Make sure WWAN is always up when just connected
		if self.wwan_iface:
			logging.debug("[dev=%s]: setting %s device up...", self.USB, self.wwan_iface)
			self.runcmd("/usr/sbin/ip link set dev " + self.wwan_iface + " up")

		logging.debug("[dev=%s]: reloading network...", self.USB)
		self.runcmd("ubus call network reload")

		# We keep track of the IP address we expect to see in the network interface
		self.expected_ip = self.qmicli_wds("--wds-get-current-settings | grep 'IPv4 address' | awk '{ print $3}'")
		self.ip_check_errors = 0

	def get_static_values(self):
		# Reset to defaults before reloading
		self.linkid = 'unknown'
		self.modem_name = 'unknown'
		self.modem_version = 'unknown'

		try:
			self.linkid = self.qmicli_dms("--dms-uim-get-imsi | awk -F\"'\" '{for(i=1;i<=NF;i++){ if(match($i, /[0-9]{14,15}/)){printf $i} } }'")
			self.modem_name = self.qmicli_dms("--dms-get-manufacturer | awk '/Manufacturer:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
			self.modem_version = self.qmicli_dms("--dms-get-model | awk '/Model:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
		except RuntimeError:
			logging.warning("[dev=%s]: couldn't load static values", self.USB)

		logging.debug("[dev=%s]: manufacturer: %s", self.USB, self.modem_name);
		logging.debug("[dev=%s]: model: %s", self.USB, self.modem_version);
		logging.debug("[dev=%s]: linkid: %s", self.USB, self.linkid);

		# Set the static values
		self.activation_status = 'activated'
		self.isp_name = ''
		self.supported_technologies = 'LTE'

		# If we're connected when loading static values we may be in auto-connect, so just
		# setup the interface. This will avoid 'PolicyMismatch' errors at this stage.
		self.reload_connection_status()
		logging.debug("[dev=%s]: connection status (initial): %s", self.USB, self.connection_status);
		if self.connection_status == 'connected':
			self.reconnected()

		#self.ip_value = wanip
		#self.gateway_value = wangw
		#self.dns_value = wandns

	def get_dynamic_values(self):
		try:
			rsrp = int(self.qmicli_nas("--nas-get-signal-info | awk '/LTE:/ { for(i=1; i<5; i++) { getline; gsub(/\\x27/, \"\", $0); if(match($0, /RSRP:/)) {printf $2 } }}'"))
		except:
			rsrp = -120

		rxvalue = self.runcmd("ifconfig " + self.ifname + " | awk  '/RX bytes:/ { split($2, a, /:/); print a[2]}'")
		txvalue = self.runcmd("ifconfig " + self.ifname + " | awk  '/TX bytes:/ { split($6, a, /:/); print a[2]}'")
		#TODO: Make changes related to qmi modems
		sspercentage = IPModems.LTESignalStrengthPercentage(str(rsrp))

		# Set the values
		self.signal_strength = rsrp
		self.signal_percentage = sspercentage
		self.rx_session_bytes = rxvalue
		self.tx_session_bytes = txvalue

		# Flag to decide whether we need to bring iface down and perform reconnection
		wwan_down = False

		# Check if we're registered
		self.reload_registration_status()
		if self.registration_status != 'registered':
			logging.warning("[dev=%s]: not registered in network: %s", self.USB, self.registration_status)
			wwan_down = True

		# Check if we're connected
		self.reload_connection_status()
		if self.connection_status == 'disconnected':
			logging.warning("[dev=%s]: not connected in network", self.USB)
			wwan_down = True

		# If we're connected, we'll do an IP check
		if self.connection_status == 'connected' and self.expected_ip != '':
			ip_valid = self.runcmd("/usr/sbin/ip addr ls dev " + self.wwan_iface + " | grep " + self.expected_ip + " | wc -l")
			if ip_valid == '0':
				logging.warning("[dev=%s]: IP check (%s) failed", self.USB, self.expected_ip)
				self.ip_check_errors += 1
			else:
				logging.debug("[dev=%s]: IP check (%s) succeeded", self.USB, self.expected_ip)


		# Do we need to force a full explicit disconnection of all our state info?
		if wwan_down:
			# Flag a new connection error detected
			self.connection_errors += 1

			# Cleanup expected IP address
			self.expected_ip = ''
			self.ip_check_errors = 0

			# If we have WWAN net info, bring it down
			if self.wwan_iface:
				logging.debug("[dev=%s]: setting %s device down...", self.USB, self.wwan_iface)
				self.runcmd("/usr/sbin/ip link set dev " + self.wwan_iface + " down")

			logging.debug("[dev=%s]: explicitly stopping connection...", self.USB)
			self.runcmd("/usr/bin/qmi-network --profile=/tmp/USB/" + self.USB + "_qminetwork.profile " + self.device + " stop")

			# If we're registered, we relaunch reconnection
			if self.registration_status == 'registered':
				# Launch qmi-network start
				logging.debug("[dev=%s]: restarting connection...", self.USB)
				self.runcmd("/usr/bin/qmi-network --profile=/tmp/USB/" + self.USB + "_qminetwork.profile " + self.device + " start")

				self.reload_connection_status()
				if self.connection_status == 'connected':
					self.reconnected()
					logging.warning("[dev=%s]: reconnected", self.USB)
				else:
					logging.warning("[dev=%s]: couldn't reconnect", self.USB)

		# If we reach too many IP check errors, we trigger a full device reboot
		if self.ip_check_errors == self.ip_check_errors_threshold:
			logging.warning("[dev=%s]: too many IP check errors (%d): requesting hard reset", self.USB, self.ip_check_errors)
			# Device should reboot itself after this
			self.linkid = self.qmicli_dms("--dms-set-operating-mode=offline")
			self.linkid = self.qmicli_dms("--dms-set-operating-mode=reset")
			self.ip_check_errors = 0
		# If we reach too many connection errors, we trigger a full device reboot
		elif self.connection_errors == self.hard_reset_threshold:
			logging.warning("[dev=%s]: too many connection errors (%d): requesting hard reset", self.USB, self.connection_errors)
			# Device should reboot itself after this
			self.linkid = self.qmicli_dms("--dms-set-operating-mode=offline")
			self.linkid = self.qmicli_dms("--dms-set-operating-mode=reset")
			self.connection_errors = 0
		# If we reach the soft reset threshold, reset the RF subsystem
		elif self.connection_errors > 0 and (self.connection_errors % self.soft_reset_threshold) == 0:
			logging.warning("[dev=%s]: too many connection errors (%d): requesting soft reset", self.USB, self.connection_errors)
			# Device will explicitly re-register after this
			self.linkid = self.qmicli_dms("--dms-set-operating-mode=low-power")
			self.linkid = self.qmicli_dms("--dms-set-operating-mode=online")


		# Ubee does not provide the following stuff
		#self.rx_session_packets = ''
		#self.tx_session_packets = ''

		#self.rx_cumulative_bytes = ''
		#self.tx_cumulative_bytes = ''

		#self.rx_cumulative_packets = ''
		#self.tx_cumulative_packets = ''
