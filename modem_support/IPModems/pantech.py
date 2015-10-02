#!/usr/bin/python

import xml.etree.ElementTree as ET
import urllib2
import subprocess

import IPModems 

# Note:
# Assuming all Pantech modems behaves the same way as Pantech UML295
# Otherwise we need to change the things

class Pantech(IPModems.IPModems):

	def __init__(self, USB):
		IPModems.IPModems.__init__(self, USB)
		self.modem_str = 'pantech'
		self.timer = 3
		self.IP = '192.168.32.2'
		self.dontping = 1

		# condata xml
		self.p_id = './/p-answer/id'
		self.p_supported_technologies = './/p-answer/subcomponent'
		self.p_connection_status = './/p-answer/condata/state/value'
		self.p_activation_status = './/p-answer/condata/activationstatus'
		self.p_signal_strength = './/p-answer/condata/signal/parameter[1]/dbm'
		self.p_signal_percentage = './/p-answer/condata/signal/parameter[1]/percent'
		self.p_isp_name = './/p-answer/condata/network/serving/name'
		self.p_server_name = './/p-answer/condata/network/serving/server'
		self.p_ip_value = './/p-answer/condata/connection/address/ipv4/ip'
		self.p_gateway_value = './/p-answer/condata/connection/address/ipv4/gateway'
		self.p_dns_value = './/p-answer/condata/connection/address/ipv4/dns'
		self.p_rx_session_bytes = './/p-answer/condata/rx/session/bytes'
		self.p_tx_session_bytes = './/p-answer/condata/tx/session/bytes'
		self.p_rx_session_packets = './/p-answer/condata/rx/session/packets'
		self.p_tx_session_packets = './/p-answer/condata/tx/session/packets'
		self.p_rx_cumulative_bytes = './/p-answer/condata/rx/cumulative/bytes'
		self.p_tx_cumulative_bytes = './/p-answer/condata/tx/cumulative/bytes'
		self.p_rx_cumulative_packets = './/p-answer/condata/rx/cumulative/packets'
		self.p_tx_cumulative_packets = './/p-answer/condata/tx/cumulative/packets'
		# discovery xml
		self.p_imsi = './/p-answer/discovery/uniqueids/ui[11]'
		self.p_modem_name = './/p-answer/discovery/platform/hw'
		self.p_modem_version = './/p-answer/discovery/platform/model'
		
	def traversexml(self, root, path, index=0):
		return root.findall(path)[index].text

	def get_static_values(self):
		root1 = root2 = ""
		try:
			address = "http://" + self.IP + "/condata"
			output = "/tmp/USB/" + self.USB + "condata.xml"
			p = subprocess.Popen(["curl", "--interface", self.ifname, address], stdout=subprocess.PIPE)
			response, err = p.communicate()
			filew = open(output, 'w')
			filew.write(response)
			filew.close()
			tree = ET.parse(output)
			root1 = tree.getroot()
		except:
			self.log('condata.xml unable to retrieve')
			pass

		try:
			address = "http://" + self.IP + "/discovery"
			output = "/tmp/USB/" + self.USB + "discovery.xml"
			p = subprocess.Popen(["curl", "--interface", self.ifname, address], stdout=subprocess.PIPE)
			response, err = p.communicate()
			filew = open(output, 'w')
			filew.write(response)
			filew.close()
			tree = ET.parse(output)
			root2 = tree.getroot()
		except:
			self.log('discovery.xml unable to retrieve')
			pass
                try:
			self.linkid = self.traversexml(root2, self.p_imsi)
			self.modem_name = self.traversexml(root2, self.p_modem_name)
			self.modem_version = self.traversexml(root2, self.p_modem_version)

			self.connection_status = self.traversexml(root1, self.p_connection_status)
			self.activation_status = self.traversexml(root1, self.p_activation_status)

			self.isp_name = self.traversexml(root1, self.p_isp_name)
		except:
		        pass


	def get_dynamic_values(self):
		root1=""
		try:
			address = "http://" + self.IP + "/condata"
			output = "/tmp/USB/" + self.USB + "condata.xml"
			p = subprocess.Popen(["curl", "--interface", self.ifname, address], stdout=subprocess.PIPE)
			response, err = p.communicate()
			filew = open(output, 'w')
			filew.write(response)
			filew.close()
			tree = ET.parse(output)
			root1 = tree.getroot()
		except:
			self.log('condata.xml unable to retrieve')
			pass

		try:
			self.signal_strength = self.traversexml(root1, self.p_signal_strength)
			self.signal_percentage = self.traversexml(root1, self.p_signal_percentage)
			self.rx_session_bytes = self.traversexml(root1, self.p_rx_session_bytes)
			self.tx_session_bytes = self.traversexml(root1, self.p_tx_session_bytes)
		except:
		        pass

		# The following things are available 
		# Enable it if necessary, and do enable the base class IPModems things
		#
		#self.supported_technologies = self.traversexml(root1, self.p_supported_technologies, 0) + " "
		#self.supported_technologies += self.traversexml(root1, self.p_supported_technologies, 1) + " "
		#self.supported_technologies += self.traversexml(root1, self.p_supported_technologies, 2)

		#self.server_name = self.traversexml(root1, self.p_server_name)

		#self.ip_value = self.traversexml(root1, self.p_ip_value)
		#self.gateway_value = self.traversexml(root1, self.p_gateway_value)
		#self.dns_value = self.traversexml(root1, self.p_rx_session_bytes)

		#self.rx_session_packets = self.traversexml(root1, self.p_rx_session_packets)
		#self.tx_session_packets = self.traversexml(root1, self.p_tx_session_packets)

		#self.rx_cumulative_bytes = self.traversexml(root1, self.p_rx_cumulative_bytes)
		#self.tx_cumulative_bytes = self.traversexml(root1, self.p_tx_cumulative_bytes)

		#self.rx_cumulative_packets = self.traversexml(root1, self.p_rx_cumulative_packets)
		#self.tx_cumulative_packets = self.traversexml(root1, self.p_tx_cumulative_packets)
