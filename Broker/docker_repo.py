#-------------------------------------------------------------------------------
# File Name : docker_repo.py
# Purpose   : create private docker registry
# Author    : Sri Muthu Narayanan Balasubramanian
# Created   : 8 Jan 2016
# Copyright :
#-------------------------------------------------------------------------------
import sys, os, time
import subprocess
import re
from Error import *

arg = ["avahi-publish-service","broker",
		"_coap._udp","5683","/Broker",
		"--sub","_broker._sub._coap._udp"]

class SA():

	def __init__(self):
		self.__InitSuccess = False

	def Advertise_Services(self):

		try:
			ad = subprocess.Popen([arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]])
		except:
			sys.exit(Error.AvahiInitError)

	def Terminate_Advertisement(self):
		output = subprocess.check_output(["pkill","avahi-publish-s"])
		print output + Error.AvahiStop
