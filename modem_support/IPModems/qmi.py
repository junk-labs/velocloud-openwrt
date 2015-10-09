#!/usr/bin/python

import logging
import commands
import IPModems

class Qmi(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'qmi'
		self.timer = 3
		self.clicmd = '/usr/bin/qmicli -d ' + self.device + ' '
		self.dms_cid = 0
		self.nas_cid = 0
		self.wds_cid = 0

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


	def get_static_values(self):
		# Reset to defaults before reloading
		self.linkid = 'unknown'
		self.modem_name = 'unknown'
		self.modem_version = 'unknown'

		try:
			self.linkid = self.qmicli_dms("--dms-uim-get-imsi | awk -F\"'\" '{for(i=1;i<=NF;i++){ if(match($i, /[0-9]{14,15}/)){printf $i} } }'")
			self.modem_name = self.qmicli_dms("--dms-get-manufacturer | awk '/Manufacturer:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
			self.modem_version = self.qmicli_dms("--dms-get-model | awk '/Model:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")
			self.connection_status = self.qmicli_wds("--wds-get-packet-service-status | awk -F\"'\" '/Connection status/ { print $2}'")
		except RuntimeError:
			logging.warning("[dev=%s]: couldn't load static values", self.USB)

		logging.debug("[dev=%s]: manufacturer: %s", self.USB, self.modem_name);
		logging.debug("[dev=%s]: model: %s", self.USB, self.modem_version);
		logging.debug("[dev=%s]: linkid: %s", self.USB, self.linkid);
		logging.debug("[dev=%s]: connection status (initial): %s", self.USB, self.connection_status);

		# Set the static values
		self.activation_status = 'activated'
		self.isp_name = ''
		self.supported_technologies = 'LTE'

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

		# Ubee does not provide the following stuff
		#self.rx_session_packets = ''
		#self.tx_session_packets = ''

		#self.rx_cumulative_bytes = ''
		#self.tx_cumulative_bytes = ''

		#self.rx_cumulative_packets = ''
		#self.tx_cumulative_packets = ''
