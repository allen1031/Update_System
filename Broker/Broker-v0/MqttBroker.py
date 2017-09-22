#-------------------------------------------------------------------------------
# File Name : ServiceAdvertise.py
# Purpose   : Service advertisement for Server
# Author    : Sri Muthu Narayanan Balasubramanian
# Created   : 8 Jan 2016
# Copyright :
#-------------------------------------------------------------------------------
import sys, os, time
import subprocess
import re
from Error import *

arg = ["mosquitto",
		"-d"]

class MQ():

	def __init__(self):
		self.__InitSuccess = False

	def Mqtt_Services(self):

		try:
			mq = subprocess.Popen([arg[0],arg[1]],shell=True)
		except:
			sys.exit(Error.MqttInitError)

