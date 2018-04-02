#!/usr/bin/python

import os
import urllib2
import json
import re
import IPModems 

class Ubee(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'ubee'
		self.timer = 3
		self.IP = '192.168.14.1'
		
	def get_static_values(self):
		conn = actsatus = ""
        # In the current firmware versions of Becee modems, all the below glorious
        # http accesses are kaput .. At the minimum we need a linkid for proper stats
        # reporting, so lets set the wanmac as the interfce mac address
		wanmac = self.mac
		wanip = wangw = wandns = ""
		modem_name = modem_version = isp_name = ""
		try:
			response = urllib2.urlopen('http://' + self.IP + '/cgi-bin/webstatus.cgi', timeout=1)
			values = response.read()
			jsonparams = json.loads(values)
			conn = jsonparams['content']['conning']
			actsatus = jsonparams['content']['autoconn']
		except:
			self.log('webstatus.cgi unable to retrieve')
			pass

		try:
			response = urllib2.urlopen('http://' + self.IP + '/cgi-bin/summary.cgi', timeout=1)
			values = response.read()
			jsonparams = json.loads(values)
			machex = jsonparams['content']['wanMAC']
			maclist = machex.split(":")
			wanmac = ''.join(['%04d' % int(i,16) for i in maclist])
			wanip  = jsonparams['content']['wanIP']
			wangw  = jsonparams['content']['wanGateway']
			wandns = jsonparams['content']['wanDns']
		except:
			self.log('summary.cgi unable to retrieve')
			pass
		
		try:
			response = urllib2.urlopen('http://' + self.IP + '/en-us.js', timeout=1)
			contents = response.read()
			for line in contents.split('\n'):
				line = str(line)
				if re.match('.*sta_str60=.*', line): # Ubee
					modem_name = self.getRHSvalue(line)	
				elif re.match('.*sta_str61=.*', line): # PXU1964
					modem_version = self.getRHSvalue(line)
				elif re.match('.*sta_str=.*', line): # Connected to FreedomPoP 4G
					isp_name = self.getRHSvalue(line).replace('Connected to ', '')
		except:
			self.log('en-us.js unable to retrieve')
			pass

		# Set the values
		self.linkid = wanmac
		self.supported_technologies = '4G'
		self.connection_status = conn
		self.activation_status = actsatus
		self.isp_name = isp_name #Freedompop
		self.modem_name = modem_name
		self.modem_version = modem_version

		#self.ip_value = wanip
		#self.gateway_value = wangw
		#self.dns_value = wandns

	def get_dynamic_values(self):
		rssi = cinr = sspercentage = ""
		rxvalue = txvalue = ""
		try:
			response = urllib2.urlopen('http://' + self.IP + '/cgi-bin/webstatus.cgi', timeout=1)
			values = response.read()
			jsonparams = json.loads(values)
			rssi = jsonparams['content']['rssi']
			cinr = jsonparams['content']['cinr']
			sspercentage = IPModems.SignalStrengthPercentage(rssi, cinr)
		except:
			sspercentage = 0
			rssi = 0
			self.log('dynamic webstatus.cgi unable to retrieve')
			pass

		rxvalue = int(os.popen('cat /sys/class/net/%s/statistics/rx_bytes' % self.ifname).read())
		txvalue = int(os.popen('cat /sys/class/net/%s/statistics/tx_bytes' % self.ifname).read())
		
		# Set the values
		self.signal_strength = rssi
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


	def getRHSvalue(self, line):
		line = line.split('=')[1].replace('\"', '')
		line = line.replace(';', '')
		return line

