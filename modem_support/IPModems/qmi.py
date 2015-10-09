#!/usr/bin/python

import commands
import IPModems

class Qmi(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'qmi'
		self.timer = 3
		self.clicmd = '/usr/bin/qmicli -d ' + self.device + ' '

	def runcmd(self, cmd):
		return commands.getstatusoutput(cmd)[1].strip()

	def qmicli(self, cmd):
		return self.runcmd(self.clicmd + cmd)

	def get_static_values(self):
		linkid = self.qmicli("--dms-uim-get-imsi | awk -F\"'\" '{for(i=1;i<=NF;i++){ if(match($i, /[0-9]{14,15}/)){printf $i} } }'")
		conn = self.qmicli("--wds-get-packet-service-status | awk -F\"'\" '/Connection status/ { print $2}'")
		modem_name = self.qmicli("--dms-get-manufacturer | awk '/Manufacturer:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")

		modem_version = self.qmicli("--dms-get-model | awk '/Model:/ { gsub(/\\x27/, \"\", $0); for(i=2;i<=NF;i++) {printf $i\" \"}}'")

		# Set the values
		self.linkid = linkid
		self.connection_status = conn
		self.modem_name = modem_name
		self.modem_version = modem_version
		self.activation_status = 'activated'
		self.isp_name = ''
		self.supported_technologies = 'LTE'

		#self.ip_value = wanip
		#self.gateway_value = wangw
		#self.dns_value = wandns

	def get_dynamic_values(self):
		rsrp = self.qmicli("--nas-get-signal-info | awk '/LTE:/ { for(i=1; i<5; i++) { getline; gsub(/\\x27/, \"\", $0); if(match($0, /RSRP:/)) {printf $2 } }}'")
		try:
			rsrp = int(rsrp)
		except:
			rsrp = "-120"
			pass

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
