#-------------------------------------------------------------------------------
# File Name : Error.py
# Purpose   : Contains error messages for Server appliation
# Author    : Sri Muthu Narayanan Balasubramanian
# Created   : 8 Jan 2016
# Copyright :
#-------------------------------------------------------------------------------

class Error():

	Success = ''
	#avahi errors:
	AvahiInitError = "Error running Avahi"
	AvahiSuccess = "Service advertisement Successful"
	AvahiStop = "Advertisement stopped"

	ServerInitError = "Error executing LWM2M server"
	ServerInitSuccess = "LWM2M Server launched"
	ServerStop = "LWM2M Server killed"
	
	ClientInitError = "Error executing LWM2M client"
	ClientInitSuccess = "LWM2M Client launched"
	ClientStop = "LWM2M CLient killed"

	NoRegisteredClients = "No Clients"
	MqttInitError="Mqtt broker error"
	
	UrlError = "Url is invalid"
	
	DatabaseInitError = "Error executing mongo Database"
	DatabaseInitSuccess = "Mongo Database launched"
	DatabaseStop = "MongoDB killed"