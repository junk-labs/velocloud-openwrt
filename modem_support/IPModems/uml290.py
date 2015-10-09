#!/usr/bin/python

import commands
import qmi

class Uml290(qmi.Qmi):

	def __init__(self, USB):
		qmi.Qmi.__init__(self, USB)
		self.modem_str = 'uml290'

	def get_static_values(self):
		qmi.Qmi.get_static_values(self)
		self.isp_name = 'Verizon'
