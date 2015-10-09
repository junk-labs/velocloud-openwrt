#!/usr/bin/python

import commands
import qmi

class Sierra(qmi.Qmi):

	def __init__(self, USB):
		qmi.Qmi.__init__(self, USB)
		self.modem_str = 'sierra'

	def get_static_values(self):
		qmi.Qmi.get_static_values(self)
		self.isp_name = 'AT&T'
