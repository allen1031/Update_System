import sys, os, time
import subprocess
import ast
import datetime
from Error import *
from Data import *
from subprocess import Popen, PIPE
from nbstreamreader import NonBlockingStreamReader as NBSR
import re
import json
import os.path
import database

"""
lwm2m server 
"""
serverPath = './lwm2mserver.exe'
serverArg = '-4'

LIST_COMMAND = "list"
NEW_CLIENT = 'New'
NO_CLIENT = 'No client'
NO_BROKER = 'No broker registered'
CLIENT = 'Client'
OBJECT = 'objects'
NAME = 'name: '
READ_COMMAND = 'read'
WRITE_COMMAND = "write"
OBSERVE_COMMAND = "observe"
EXECUTE_COMMAND = "exec"

#CLIENT_NUMBER
#RESOURCE_URL
FILE = "s148340.json"

#####################
## RETURN ERROR
#####################
CLIENT_SUCCESS = 'valid client'
INVALID_CLIENT = 'invalid client'
URL_SUCCESS = 'valid url'
INVALID_URL = 'invalide url'
OBJ_SUCCESS = 'valid object'
INVALID_OBJ = 'invalid object'

EMPTY_REQUEST = "an empty request, please request again"

READ_BYTE = "bytes received of type"
C1 = 'C1'


TIME_COMMAND ="time"
#CLIENT_NUMBER
#RESOURCE_URL
PMIN = "1"
PMAX = "5"


READ_SUCCESS = 'read request succeeds'
READ_FAILURE = 'read request fails'

WRITE_SUCCESS = 'write request succeeds'
WRITE_FAILURE = 'write request fails'

EXECUTE_SUCCESS = 'execute request succeeds'
EXECUTE_FAILURE = 'execute request fails'

"""
 *	resources : /10500/0/0 number of registrating end nodes (RW)  default : 0
 *				/10500/0/1 end node to be updated           (RW)  default : Unset
 *              /10500/0/2 end node name to be updated      (RW)  default : Unset
 *              /10500/0/3 resource url                     (RW)  default : Unset
 *              /10500/0/4 resource value                   (RW)  default : Unset
 *              /10500/0/5 update registration              (E)  
 *              /10500/0/6 update resource                  (E)
 *              /10500/0/7 execute resource                 (E)
"""
BROKER_NUM = '/10500/0/0'
BROKER_CLIENT = '/10500/0/1'
BROKER_NAME = '/10500/0/2'
BROKER_RESOURCE = '/10500/0/3'
BROKER_VALUE = '/10500/0/4'
BROKER_UPDATE_REG = '/10500/0/5'
BROKER_UPDATE_RES = '/10500/0/6'
BROKER_EXEC_RES = '/10500/0/7'

"""
 *	lwm2m objects : /10400/0 container app
 *	
 *	resources : /10400/0/0 container name (RW)  default : hello-world
 *				/10400/0/1 updating state (RW)   default : 0
 *              /10400/0/2 update command (RW)  default : docker pull
 *              /10400/0/3 update         (E)   default :
 *              /10400/0/4 update result  (RW)  default : 0
 *              /10400/0/5 update argument(RW)  default : 
"""

CONTAINER_NAME = '/10400/0/0'
CONTAINER_STATE = '/10400/0/1'
CONTAINER_COMMAND = '/10400/0/2'
CONTAINER_UPDATE = '/10400/0/3'
CONTAINER_RESULT = '/10400/0/4'
CONTAINER_ARGU = '/10400/0/5'
CONTAINER_WRITE_TIME = '10400/0/6'


class SP():
	def __init__(self):
		
		self.__InitSuccess = False
		self.__InitSuccess_client = False
		self.__InitSuccess_mongodb = False
		self.__clients = {}
		self.__clients_names = {}
		self.__objects = {"init":"empty"}
		#self.__objectsjson = {}
		
		self.__line = 0
		self.__input = ""
		self.__num_clients = 0
		self.__client_no = 0
		self.__num_clients = 0
		
		self.__obj = ""
		self.__resource_id = 0
		
		self.__mongodb = None

		self.__client_duration = {}

	
	def __del__(self):

		self.Kill_Server_Process()


	"""
		initiate different modules in Broker
		
		+ Broker: 
		|
		|	+ lwm2m server
		|	+ lwm2m client 
	"""


	def __Start_LWM2M(self):



		try:
			print "lwm2m is starting"
			self.__lwm2m =subprocess.Popen(['./lwm2mserver1', '-4'], stdout=subprocess.PIPE, stdin=subprocess.PIPE)

		except:

			return False

		

		self.__nbsr = NBSR(self.__lwm2m.stdout)

		print Error.ServerInitSuccess
		
		#Time for the server to stabilize

		time.sleep(2)

		return True
		
	
	def __Start_mongodb(self):
		
		try:
			self.__mongodb = subprocess.Popen(['mongod'],stdout=subprocess.PIPE, stdin=subprocess.PIPE)
		except :
			return False
		
		self.__nbsr_db = NBSR(self.__mongodb.stdout)
		
		print self.__nbsr_db
		
		print Error.DatabaseInitSuccess
		
		return True
		
	def __Start_LWM2M_Client(self,endpoint,host_ip,db_ip):
	
		try:

			#self.__lwm2m_client =subprocess.Popen(['java','-jar','client-0.0.jar','-n','zheng','-u','131.155.241.109'], stdout=subprocess.PIPE, stdin=subprocess.PIPE)
			self.__lwm2m_client =subprocess.Popen(['java','-jar','client-0.0.jar','-n',endpoint,'-u',host_ip,'-m',db_ip,'-r',"broker"], stdout=subprocess.PIPE, stdin=subprocess.PIPE)

		except:

			return False

		self.__nbsr_client = NBSR(self.__lwm2m_client.stdout)
		
		
		print self.__nbsr_client

		print Error.ClientInitSuccess
		
		#Time for the server to stabilize

		time.sleep(2)

		return True
		
		
	def pipe_flush(self):
		message_queue = " "
		while message_queue:
			message_queue = self.__nbsr.readline(0.01)
		
			
			
	def pipe_communication(self,clientNo,url):
	
		self.pipe_flush()
		
		READ_REQUEST = READ_COMMAND + ' ' + str(clientNo) + ' ' + url
		
		self.__lwm2m.stdin.write(READ_REQUEST)
		
		print READ_REQUEST
		
		tmp_output = " "
		
		line = 0
		
		primary_search = "Client #" + str(clientNo) + " " + url + " : " + "2.05"
		
		while tmp_output:
		
			print "The {} line is : {}".format(line, tmp_output)
			
			if tmp_output.find(primary_search)>=0:
				print "read success"
				return 0
			
			line = line + 1
		
			tmp_output = self.__nbsr.readline(0.01)
		
		print "read failed"
		return 1
	"""
		output debug
		
		|   + client debug
		|   + server debug
		
	
	"""
	
	def clientOutputDisplay(self):
		
		tmp_line = 0
		
		temp = self.__nbsr_client.readline(0.1)
			
		while temp:
		
			tmp_line = tmp_line + 1
			
			print "the {} line from client is {}".format(tmp_line, temp)
			
			temp = self.__nbsr_client.readline(0.1)
				
				
	def serverOutputDisplay(self):
	
		tmp_line = 0
	
		temp = self.__nbsr.readline(0.1)
			
		while temp:
		
			tmp_line = tmp_line + 1
			
			print "the {} line from server is {}".format(tmp_line, temp)
			
			temp = self.__nbsr.readline(0.1)


		
	"""
		communication between Broker and lwm2m client 
		
		+ Lwm2m client communication 
		|
		|	+ Read command from lwm2m client through json file
		|	+ Write feedback to lwm2m client through json file
		
		
	"""
	
	def clientObserv(self,key):
		
		if (os.path.isfile(FILE)):
		
			with open(FILE,"r") as data_file:
				
				data = json.load(data_file)
				
			self.__input = data[key]
			
			#print self.__input
		else :
			self.__input = 'null'
			print "target json file is not existed"
			
		return self.__input
	
	def Lwm2mClientWrite(self, key, value):
		
		with open(FILE,"r") as f:
			data = json.load(f)
			
		if key not in data:
			obj = {key: value}
			data.update(obj)
		else: 
			data [key] = value
	
		with open(FILE,"w") as f:
			f.write(json.dumps(data))
		
	"""
		+ operations of lwm2m server in Broker
		
		|  + list  : list out registered client 
		|  + read  : read resources value  
		|  + write : write resources value 
	
	"""
	
	
	'''
		parse the resource id from url 
	'''
	
	def urlParse(self, url):
		
		url_list = url.split("/")
		
		#print url_list
		
		if len(url_list)<3:
		
			print INVALID_URL
			
			return INVALID_URL
		
		self.__obj = "/" + url_list[1] + "/" + url_list[2]
		
		
		if len(url_list) == 4:
			self.__resource_id = url_list[3]	
			
		return URL_SUCCESS
	
	
	'''
		VALIDATE IF THE REQUEST HAS REGULATED INFO
	'''
		
	def requestValidation(self, db, mycoll, key, clientNo, url):
		
		'''
		READ FROM DATABASE
		'''
		array = db.getObject(mycoll,key)
		
		self.__num_clients = array[Data.NUM_CLIENTS]
		
		#self.__num_clients = self.clientObserv(Data.NUM_CLIENTS)
		
		#self.__objects = self.clientObserv(Data.OBJECT)
		
		#print "input client number is {} and number of registered client is {}".format(clientNo, self.__num_clients)
		
		if int(clientNo) >= int(self.__num_clients) and not isinstance(clientNo,int):
		
			'''
			WRITE TO DATABASE
			'''
			post = {Data.VALUE:INVALID_CLIENT}
			mycoll = db.updateDB(mycoll,key,post)
			#self.Lwm2mClientWrite(Data.VALUE,INVALID_CLIENT)
			
			#self.Lwm2mClientWrite(Data.SEND_FLAG,"done")
			
			return INVALID_CLIENT
		
		#print url
		
		#print self.__objects
		
		#print self.__objects[clientNo]
		'''
		for object in self.__objects[clientNo]:
			
			#print object
			
			if url == object:
				
				return OBJ_SUCCESS
		
		return INVALID_OBJ
		'''
		
		return OBJ_SUCCESS
		
		
	'''
		READ REQUEST 
	'''
	######################################################################################################
	############################################Wakaama Client API########################################
	######################################################################################################
	
	
	def __Parser(self,line1,line2,line3):

		start = line1.find("#")+1

		end = start+1

		client_id = line1[start:end]

		client_id = int(client_id)

		start = line1.find("/")

		end = line1.find(" ",start)

		resource_id = line1[start:end]

		if line3.find("sv") >= 0:
			start = line3.find("v") + 4
			end = line3.find("}",start)-1
		else:
			start = line3.find("v") + 3
			end = line3.find("}",start)
		data = line3[start:end]
		
		return data
		
	
	def WakaamaRead(self, clientNo , url):
		
		'''
			start the read request
		'''
		READ_REQUEST = READ_COMMAND + ' ' + str(clientNo) + ' ' + url
		
		self.__lwm2m.stdin.write(READ_REQUEST)
		
		primary_search = "Client #" + str(clientNo) + " " + url + " : " + "2.05"
		#time.sleep(0.05)
		 
		tmp_output = " "

		line = 0
		read_mark = 0
		while tmp_output:
		
			print "The {} line is : {}".format(line, tmp_output)
			
			line = line + 1

			if tmp_output.find(primary_search)>=0:
			
				line1 = tmp_output
				print line1
				line2 = self.__nbsr.readline(0.01)
				print line2
				line3 = self.__nbsr.readline(0.01)
				print line3
				
				read_mark = 1

			tmp_output = self.__nbsr.readline(0.01)
		
		if (read_mark == 0):
			return READ_FAILURE
		else:
			value = self.__Parser(line1,line2,line3)
			return value
		
	
	def WakaamaWrite(self, clientNo , url, write_value):
	
		'''
			start the write request 
		'''
		WRITE_REQUEST = WRITE_COMMAND + " " + str(clientNo) + " " + url + " " + str(write_value)
			
		print WRITE_REQUEST
		
		self.__lwm2m.stdin.write(WRITE_REQUEST)
		
		
		tmp_output = " "
		
		primary_search = "Client #" + str(clientNo) + " " + url + " : " + "2.04"
		line = 0
		while tmp_output:
		
			print "The {} line is : {}".format(line, tmp_output)
			
			if tmp_output.find(primary_search)>=0:
				
				return WRITE_SUCCESS
			
			line = line + 1
			
			tmp_output = self.__nbsr.readline(0.01)
		
		return WRITE_FAILURE
	
	def WakaamaExecute(self,db,collection,key, clientNo , url):
	
		array = db.getObject(collection,key)
		client_list = array[Data.CLIENT_LIST]
		client = str(client_list[str(clientNo)])
		brokerID = str(array[Data.ID])
		client_key = {Data.ROLE:"Client",Data.ID:client,Data.broker_id:brokerID}
		print client_key
		
		post = {Data.USER_INPUT:"None",Data.UPDATE_RESULT:"None",Data.UPDATE_STATE:"idle"}
		db.updateDB(collection,client_key,post)		
		
		line = 0
		
		EXECUTE_REQUEST = EXECUTE_COMMAND + " " + str(clientNo) + " " + url 
			
		print EXECUTE_REQUEST
		
		self.__lwm2m.stdin.write(EXECUTE_REQUEST)
		
		primary_search = "Client #" + str(clientNo) + " " + url + " : " + "2.04"
		
		tmp_output = " "
		
		while tmp_output:
		
			print "The {} line is : {}".format(line, tmp_output)
			
			if tmp_output.find(primary_search)>=0:
				
				return WRITE_SUCCESS
			
			line = line + 1
			
			tmp_output = self.__nbsr.readline(0.01)
			
		return EXECUTE_FAILURE
		
	'''	
	def UpdateWait(self,clientNo):
		
		update_state_url = '/1025/0/3'
		update_result_url = '/1025/0/5'
		update_duration_url = '/1025/0/8'
		
		state = 0
		result = 0
		
		while str(state) != "1" or str(result) == "0":
			
			time.sleep(1)
			
			print "state at present : {}".format(state) 
			print "result at present : {}".format(result)
		
			state = self.WakaamaRead(clientNo, update_state_url)
		
			result = self.WakaamaRead(clientNo, update_result_url)	
		
		duration = self.WakaamaRead(clientNo, update_duration_url)
			
		print "time in device costs :{}".format(duration)
		
		self.__client_duration.update({str(clientNo) : duration})
		if str(result) == "1":
			return True
		else:
			return False	
	'''
	def UpdateWait(self,db,collection,key,clientNo):
		
		array = db.getObject(collection,key)
		client_list = array[Data.CLIENT_LIST]
		client = str(client_list[str(clientNo)])
		brokerID = str(array[Data.ID])
		client_key = {Data.ROLE:"Client",Data.ID:client,Data.broker_id:brokerID}
		print client_key
		array = db.getObject(collection,client_key)
		update_result = str(array[Data.UPDATE_RESULT])
		
		while update_result == "None":	
			time.sleep(0.05)
			#print update_result
			array = db.getObject(collection,client_key)
			update_state = str(array[Data.UPDATE_STATE])
			update_result = str(array[Data.UPDATE_RESULT])
		
		if update_result == "1":
			print "client {} update success".format(clientNo)
			return True
		else:
			print "client {} update failure".format(clientNo)
			return False
		
	def ClientDBInit(self,db_ip,db,collection,key,group_size):
	
		mongoclient = db.connectDB(db_ip,27017)
		i = 0
		while True:
			if i >= int(group_size):
				break
			
			array = db.getObject(collection,key)
			client_list = array[Data.CLIENT_LIST]
			client = str(client_list[str(i)])
			brokerID = str(array[Data.ID])
			client_key = {Data.ROLE:"Client",Data.ID:client,Data.broker_id:brokerID}
			print client_key
			db.initClientDB(mongoclient,client_key);
			
			i = i + 1
			
	
	def ClientUpdate(self,db_ip,db,collection,key,group_size,package_url):
		
		print "start update 0"
		i = 0
		url = '/1025/0/1' #/10400/0/0
		exe = '/1025/0/2'
		value = package_url
		processed_node = 0
		successful_node = 0
		failed_node = 0
		process = {}
		
		self.ClientDBInit(db_ip,db,collection,key,group_size) 
		
		while True:
			if i >= int(group_size):
				break
			
			write_result = self.WakaamaWrite(i , url, value)
			
			if write_result != WRITE_SUCCESS:
				print "client {} write failure".format(i)
				process[i] = 0
				 
				#processed_node = processed_node + 1
			else:
				print "client {} write success".format(i)
				process[i] = 1
				exec_result = self.WakaamaExecute(db,collection,key,i, exe)
			i = i + 1
		
		#post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node}
		#db.updateDB(collection, key, post)
		
		i = 0
		while True:
			
			if i >= int(group_size):
				break
			exec_result = self.WakaamaExecute(db,collection,key,i, exe)
			if process[i] == 1:
				result = self.UpdateWait(db,collection,key,i)
				if result == True:
					successful_node = successful_node + 1
				else:
					failed_node = failed_node + 1
			else:
				failed_node = failed_node + 1
			i = i + 1
			
			processed_node = processed_node + 1
			print "processed node:{}, success node:{}, failed node:{}".format(processed_node,successful_node,failed_node)
			post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node, Data.TIME_CONSUMPTION:self.__client_duration}
			db.updateDB(collection, key, post)
		
		
	######################################################################################################
	############################################Leshan Client API#########################################
	######################################################################################################
		
	'''
		READ REQUEST 
	'''
	
	def ReadNumFromLwm2m(self, clientNo , url):
		
		line = 0;
		
		if not self.urlParse(url) == URL_SUCCESS:
			return INVALID_URL
		#print "url is valid"
		
		'''
			start the read request
		'''
		READ_REQUEST = READ_COMMAND + ' ' + str(clientNo) + ' ' + url
		print READ_REQUEST
		
		self.__lwm2m.stdin.write(READ_REQUEST)
		
		tmp_output = " "
		
		value = 'None'
		
		search_key = "Client #" + str(clientNo) + " " + url + " : " + "2.05"
		
		while tmp_output:	
			print "The {} line is : {}".format(line, tmp_output)
			if tmp_output.find(search_key) >= 0:	
				while True:
					if tmp_output.find("data as Float") >= 0:	
						print "The {} line is : {}".format(line, tmp_output)
						start = tmp_output.find(":") + 2
						end = tmp_output.find("\r")
						
						value = tmp_output[start:end]
						break
						
					tmp_output = self.__nbsr.readline(0.01)
			
			tmp_output = self.__nbsr.readline(0.01)
		
		return value
			
		
	def WriteToLwm2m(self, db, collection, key, clientNo , url, write_value):
		
		line = 0
		self.urlParse(url)
		
	
		'''
			start the write request 
		'''
		WRITE_REQUEST = WRITE_COMMAND + " " + str(clientNo) + " " + url + " " + str(write_value)
		self.__lwm2m.stdin.write(WRITE_REQUEST)	

		
		primary_search = "Client #" + str(clientNo) + " " + url + " : "
		search_key = "Client #" + str(clientNo) + " " + url + " : " + "2.04"
		
		print WRITE_REQUEST
		
		write_mark = 0
		tmp_output = " "
		while tmp_output:	
		
			print "The {} line is : {}".format(line, tmp_output)
			line = line + 1
			
			if tmp_output.find(primary_search) >= 0:
				write_mark = 1
				
			if tmp_output.find(search_key) >= 0:
				#write_mark = 1
				return WRITE_SUCCESS
			
			tmp_output = self.__nbsr.readline(0.01) 
		
		if write_mark == 0:
			return 'None'
		else:
			return WRITE_FAILURE
		
	def ExecToLwm2m(self, db, collection, key, clientNo , url):	
		
		line = 0
		
		EXECUTE_REQUEST = EXECUTE_COMMAND + " " + str(clientNo) + " " + url 
			
		print EXECUTE_REQUEST
		
		self.__lwm2m.stdin.write(EXECUTE_REQUEST)
		
		time.sleep(0.1)
		
		primary_search = "Client #" + str(clientNo) + " " + url + " : "
		search_key = "Client #" + str(clientNo) + " " + url + " : " + "2.04"
		
		exec_mark = 0
		tmp_output = " "
		while tmp_output:	
		
			#print "The {} line is : {}".format(line, tmp_output)
			line = line + 1
			
			if tmp_output.find(primary_search) >= 0:
				exec_mark = 1
				
			if tmp_output.find(search_key) >= 0:
				return EXECUTE_SUCCESS
			
			tmp_output = self.__nbsr.readline(0.01) 
		
		if exec_mark == 0:
			return 'None'
		else:
			return EXECUTE_FAILURE
		
	def UpdateResult(self,clientNo):
		
		update_state_url = '/10400/0/1'
		update_result_url = '/10400/0/4'
		update_duration_url = '/10400/0/6'
		
		
		
		state = self.ReadNumFromLwm2m(clientNo, update_state_url)
		while state == 'None':
			state = self.ReadNumFromLwm2m(clientNo, update_state_url)
		
		result = self.ReadNumFromLwm2m(clientNo, update_result_url)
		while result == 'None':
			result = self.ReadNumFromLwm2m(clientNo, update_result_url)
		
		print "state at present : {}".format(state) 
		print "result at present : {}".format(result)
		
		while str(state) != "1" or str(result) == "0":
			time.sleep(0.5)
			state = self.ReadNumFromLwm2m(clientNo, update_state_url)
			while state == 'None':
				state = self.ReadNumFromLwm2m(clientNo, update_state_url)
		
			result = self.ReadNumFromLwm2m(clientNo, update_result_url)
			while result == 'None':
				result = self.ReadNumFromLwm2m(clientNo, update_result_url)
		
		duration = self.ReadNumFromLwm2m(clientNo, update_duration_url)
		while duration == 'None':
			duration = self.ReadNumFromLwm2m(clientNo, update_duration_url)
			
		print "time in device costs :{}".format(duration)
		
		self.__client_duration.update({str(clientNo) : duration})
		if str(result) == "1":
			return True
		else:
			return False
		
		
	
	def GroupUpdate(self,db,collection,key,group_size,target_url):
		
		print "start update 0"
		i = 0
		url = CONTAINER_NAME #/10400/0/0
		exe = CONTAINER_UPDATE
		value = target_url
		processed_node = 0
		successful_node = 0
		failed_node = 0
		post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node}
		db.updateDB(collection, key, post)
		
		print "start update 1"
		
		while True:
			if i >= int(group_size):
				break
				
			write_result = self.WriteToLwm2m(db, collection, key, i , url, value)
			while write_result == 'None':
				write_result = self.WriteToLwm2m(db, collection, key, i , url, value)
			
			if write_result != WRITE_SUCCESS:
				print "client {} write failure".format(i)
				failed_node = failed_node + 1
				#processed_node = processed_node + 1
			else:
				print "client {} write success".format(i)
				
				exec_result = self.ExecToLwm2m(db, collection, key, i, exe)
				if exec_result != EXECUTE_SUCCESS:
					time.sleep(0.1)
			'''	
			################################################
				result = self.UpdateResult(i)
				if result == True:
					successful_node = successful_node + 1
				else:
					failed_node = failed_node + 1
			
			processed_node = processed_node + 1
			print "processed node:{}, success node:{}, failed node:{}".format(processed_node,successful_node,failed_node)
			post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node, Data.TIME_CONSUMPTION:self.__client_duration}
			db.updateDB(collection, key, post)
			#####################################################
			'''
			i = i + 1
		
		
		i = 0
		while True:
			if i >= int(group_size):
				break
				
			result = self.UpdateResult(i)
			if result == True:
				successful_node = successful_node + 1
			else:
				failed_node = failed_node + 1
			i = i + 1
			
			processed_node = processed_node + 1
			print "processed node:{}, success node:{}, failed node:{}".format(processed_node,successful_node,failed_node)
			post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node, Data.TIME_CONSUMPTION:self.__client_duration}
			db.updateDB(collection, key, post)
		
		
			
			
	#######################################################################################################################
	#######################################################################################################################
		
		
	
	def ClientReader(self, db, collection, key, clientNo , url):
	
		line = 0	

		if not self.urlParse(url) == URL_SUCCESS:
			
			
			'''
				WRITE TO DATABASE
			'''
			#self.Lwm2mClientWrite(Data.VALUE,INVALID_URL)
			post = {Data.VALUE:INVALID_URL}
			db.updateDB(mycoll,key,post)
			
			
			
			self.clientOutputDisplay();
			
			return INVALID_URL
		
		print "url is valid"
		
		valid_result = self.requestValidation(db, collection, key, clientNo,self.__obj)
		
		print "object is success"
		
		if str(valid_result) != OBJ_SUCCESS:
		
			print valid_result
		
			return valid_result
			
		print "Read requeset is valid"
		
		'''
			start the read request
		'''
		
		READ_REQUEST = READ_COMMAND + ' ' + clientNo + ' ' + url
		
		print READ_REQUEST
		
		self.__lwm2m.stdin.write(READ_REQUEST)
		
		#print request
		
		tmp_output = self.__nbsr.readline(0.01) 
		
		read_mark = 0
		
		while tmp_output:	
		
			print "The {} line is : {}".format(line, tmp_output)
			
			if tmp_output.find(READ_BYTE) >= 0:
			
				read_mark = 1
				
				end = tmp_output.find(READ_BYTE)
			
				start0 = end - 2
				
				start1 = end - 1
				
				if start0 == " ":
					start = start1
				else:
					start = start0
					
				start = 4
				
				packet_size = int(tmp_output[start:end])
				
				print "packet size is {}".format(packet_size)
						
			
			if tmp_output.find(C1) >= 0:	

				print tmp_output.find(C1)
				
				start = tmp_output.find(C1) + (packet_size-1) * 3 
				
				print start 
				
				end = tmp_output.find("...")
				
				print end
				
				int_value = int(tmp_output[start:end])
				
				print "integer value is {}".format(int_value)
				
				'''
				WRITE TO DATABASE
				'''
				post = {Data.VALUE:int_value}
				db.updateDB(mycoll,key,post)
				#self.Lwm2mClientWrite(Data.VALUE,int_value)
				
				self.clientOutputDisplay()
				
				return READ_SUCCESS
				
				
			if tmp_output.find(".") >= 0 and tmp_output.count("Client") <=0 and tmp_output.find(C1) < 0:
			
				count = tmp_output.count(".")
				
				start = tmp_output.find(".") + count
				
				end = tmp_output.find("\r")
				
				str_value = str(tmp_output[start:end])
				
				if str_value != " ":
				
					print "string value is {}".format(str_value)
					
					#self.Lwm2mClientWrite(Data.VALUE,str_value)
				
					'''
					WRITE TO DATABASE
					'''
					post = {Data.VALUE:str_value, Data.READ_VALUE:str_value}
					db.updateDB(mycoll,key,post)
					
					
					self.clientOutputDisplay();				
					
				return READ_SUCCESS
				
			
			line = line + 1
			
			tmp_output = self.__nbsr.readline(0.01) 
			
			#tmp_output = self.stdout.readline(0.01)
		
		if not read_mark:
		
			str_value = EMPTY_REQUEST
			
			'''
			WRITE TO DATABASE
			'''
			post = {Data.VALUE:str_value}
			db.updateDB(mycoll,key,post)			
		
			#self.Lwm2mClientWrite(Data.VALUE,str_value)
			
			return READ_FAILURE
		###############################
		#retrieve the info from request
		###############################
		
	'''
		WRITE REQUEST
	'''
	
	def ClientWriter(self, db, collection, key, clientNo , url, write_value):
	
		self.urlParse(url)
		
		valid_result = self.requestValidation(db, collection, key, clientNo,self.__obj)
		
		if str(valid_result) != OBJ_SUCCESS:
		
			print valid_result
		
			return valid_result
			
		print "Write requeset is valid"
	
		'''
			start the write request 
		'''
		WRITE_REQUEST = WRITE_COMMAND + " " + str(clientNo) + " " + url + " " + str(write_value)
			
		print WRITE_REQUEST
		
		self.__lwm2m.stdin.write(WRITE_REQUEST)
		
		time.sleep(0.5)
	
		print "start to check write"

		write_result = self.__write_check(db, collection, key, clientNo, url, write_value)
		
		return write_result
		
	def __write_check(self,db, collection, key, clientNo, url, write_value):
	
		
		#read_result = ClientReader(db, collection, key, clientNo , url)
		
		'''
		READ FROM BROKER DATABASE, determine if there is any client
		'''
		array = db.getObject(collection,key)
		num_clients = array[Data.NUM_CLIENTS]
		if num_clients == "0":
			
			print NO_CLIENT
			return WRITE_FAILURE
		
		'''
		READ FROM CLIENT DATABASE
		'''
		print clientNo
		client_list = array[Data.CLIENT_LIST]
		print client_list
		client = str(client_list[str(clientNo)])
		
		
		client_key = {Data.ROLE:"Client",Data.ID:client}
		print client_key 
		
		
		post = {Data.UPDATE_URL : write_value}
		db.updateDB(collection, client_key,post)
		
		
		WRITE_REQUEST = WRITE_COMMAND + " " + str(clientNo) + " " + url + " " + str(write_value)
			
		print WRITE_REQUEST
		 
		self.__lwm2m.stdin.write(WRITE_REQUEST)
		
		#Stime.sleep(0.5)
			
	
		read_value = "Unset"

		while read_value == "Unset":
			time.sleep(0.05)
			array = db.getObject(collection,client_key)
			read_value = str(array[Data.UPDATE_URL])
			print "during writing, the update url is {}".format(read_value)
		
		print "the final update url is {}".format(read_value)
		if (str(read_value) == str(write_value)):
			
			return WRITE_SUCCESS
			
		else:
			print "the written value is not passed"
			return WRITE_FAILURE
		
	'''
		EXECUTE REQUEST
	
	'''
	
	def ClientExecuter(self, db, collection, key, clientNo , url):
	
		self.urlParse(url)
		
		valid_result = self.requestValidation(db, collection, key, clientNo,self.__obj)
		
		if str(valid_result) != OBJ_SUCCESS:
		
			print valid_result
		
			return valid_result
			
		print "Execute requeset is valid"
	
		'''
			start the write request 
		'''
		#EXECUTE_REQUEST = EXECUTE_COMMAND + " " + str(clientNo) + " " + url
		
		#print EXECUTE_REQUEST
		 
		#self.__lwm2m.stdin.write(EXECUTE_REQUEST)
		
		#time.sleep(0.5)
		
		execute_result = self.__execute_check(db, collection, key, clientNo, url)
		
		return execute_result

		#self.Lwm2mClientWrite(Data.SEND_FLAG,"done")
		
	def __execute_check(self,db, collection, key, clientNo, url):
		
		'''
		READ FROM CLIENT DATABASE
		'''
		array = db.getObject(collection,key)
		client_list = array[Data.CLIENT_LIST]
		client = str(client_list[str(clientNo)])
		client_key = {Data.ROLE:"Client",Data.ID:client}
		print client_key
		
		EXECUTE_REQUEST = EXECUTE_COMMAND + " " + str(clientNo) + " " + url
		  
		print EXECUTE_REQUEST
		
		
		post = {Data.USER_INPUT:"None",Data.UPDATE_RESULT:"None",Data.UPDATE_STATE:"idle"}
		db.updateDB(collection,client_key,post)		
		update_result = "None"
		 
		self.__lwm2m.stdin.write(EXECUTE_REQUEST)
		'''
		while True:
			time.sleep(0.5)
			array = db.getObject(collection,client_key)
			usr_input = str(array[Data.USER_INPUT])
			print "ready to be updated"
			if usr_input == "update":
				break 
		'''
		print "the initial update result is set to None"
		while update_result == "None":	
			time.sleep(0.05)
			array = db.getObject(collection,client_key)
			update_state = str(array[Data.UPDATE_STATE])
			update_result = str(array[Data.UPDATE_RESULT])
			#print "During updating, the result of update is: {}".format(update_result)
				
		
			
		'''
		WRITE TO DATABASE
		'''
		print "the final result of update is: {}".format(update_result)
		if int(update_result) == int(1):
			return EXECUTE_SUCCESS
		else:
			return EXECUTE_FAILURE
		
	'''
		LIST REGISTED CLIENTS
	'''
	
	def merge_two_dicts(self,x, y):
		"""Given two dicts, merge them into a new dict as a shallow copy."""
		z = x.copy()
		z.update(y)
		return z
	
	def ClientFinder(self,db,collection,key):
		"""
		clear the clients
		"""
		self.__clients.clear()
		
		self.__clients_names.clear()
		

		self.__lwm2m.stdin.write(LIST_COMMAND)
		
		time.sleep(0.01)

		tmp_output = self.__nbsr.readline(0.01) 
		
		line = 0
		
		next = 0
		
		num_client = 0
		
		print "clients registration is going to be retrieved..."

		while tmp_output:
       
			print "The {} line : {}".format(line,tmp_output)
			
			line = line + 1
			
			
			if tmp_output.find(NO_CLIENT) >= 0:
				
				self.__clients.clear()
				
				num_client = 0
				
				'''
				WRITE TO DATABASE
				'''
				post = {Data.NUM_CLIENTS : num_client}
				db.updateDB(collection, key, post)
				
				print Error.NoRegisteredClients
				
				return Error.Success
				
			if tmp_output.find(CLIENT) >= 0:
				
				start = tmp_output.find(CLIENT) + 8
				
				end = tmp_output.find(":",start)
				
				tmp_id = str(tmp_output[start:end])
				
				if len(tmp_id) > 4:
					if tmp_id.find("/") >= 0:
						if tmp_id.find("text"):
							tmp_output = self.__nbsr.readline(0.01)
							continue
						end = tmp_output.find("/",start)-1	
						
					#if tmp_output.find("unregistered") >= 0
					else:
						tmp_output = self.__nbsr.readline(0.01)
						continue
				
				tmp_id = int(tmp_output[start:end])
				
				self.__client_no = tmp_id
				
				if tmp_id + 1 > num_client :
					
					num_client = num_client + 1
				
				#if num_client >= tmp_id + 1:
				
				self.__num_clients = num_client
				
				print num_client
				#self.Lwm2mClientWrite(Data.SEND_FLAG,"done")
				
				#num_client = num_client + 1
			
				
				if self.__clients.has_key(tmp_id):
				
					tmp_output = self.__nbsr.readline(0.01)
					
					continue
					
				tmp_output = self.__nbsr.readline(0.01)
				
				tmp_list=[]

				start = tmp_output.find(NAME) + 7

				end = tmp_output.find('\"',start)

				tmp_name = tmp_output[start:end]
				
				tmp_list.append(tmp_name)
				
				self.__clients_names[tmp_id] = tmp_name

				#print tmp_list
				self.__clients[tmp_id] = tmp_list
				
				#print self.__clients
			
			
			if tmp_output.find(OBJECT) >= 0:
			
				tmp_obj_id = 0
				
				tmp_list=[]
				
				start = tmp_output.find(OBJECT) + 9
				
				if self.__objects.has_key(self.__client_no):
				
					tmp_output = self.__nbsr.readline(0.01)
					
					continue
				
				while start != 1:
				
					end = tmp_output.find(',',start)
					
					tmp_object = tmp_output[start:end]
					
					if tmp_object == "\r":
						
						break
					
					#print "The {} start is : {}".format(tmp_obj_id,tmp_object)
					
					tmp_list.append(tmp_object)
					
					start = end + 2
					
					tmp_obj_id = tmp_obj_id + 1
					
				self.__objects[self.__client_no] = tmp_list
				
				print self.__objects[self.__client_no]					
			
				'''
				WRITE TO DATABASE
				'''
				#post = {Data.NUM_CLIENTS : num_client}
				#db.updateDB(collection, key, post)
				#self.Lwm2mClientWrite(Data.OBJECT,self.__objects)
				
				#self.Lwm2mClientWrite(Data.SEND_FLAG,"done")
			
			#timesleep(0.01)
			tmp_output = self.__nbsr.readline(0.01)
				
		'''
		WRITE TO DATABASE
		'''
		#self.Lwm2mClientWrite(Data.NUM_CLIENTS,num_client)
		print self.__clients_names
		i = 0
		client_list = {}
		while i<(num_client):
			client_list = self.merge_two_dicts(client_list , {str(i) : self.__clients_names[i]})
			#client_list.append(self.__clients[i])
			i = i + 1
		
		post = {Data.NUM_CLIENTS : num_client, Data.CLIENT_LIST : client_list}
		db.updateDB(collection, key, post)
	
	"""
	distributed software update
	"""	

	
	
	def groupUpdate(self,db,collection,key,group_size,target_url):
		
		i = 0
		url = CONTAINER_NAME #/10400/0/0
		value = target_url
		processed_node = 0
		successful_node = 0
		failed_node = 0
		post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node}
		db.updateDB(collection, key, post)
		
		while True:

			if i >= int(group_size):
				break
			
			write_result = self.ClientWriter(db, collection, key, i , url, value)
			
			if write_result != WRITE_SUCCESS:
				print "client {} write failure".format(i)
				failed_node = failed_node + 1
			else:
				print "client {} write success".format(i)
				url = CONTAINER_UPDATE
				execute_result = self.ClientExecuter(db, collection, key, i, url)
				if execute_result != EXECUTE_SUCCESS:
					print "client {} execute failure".format(i)
					failed_node = failed_node + 1
				else:
					print "client {} execute success".format(i)
					successful_node = successful_node + 1
			
			i = i + 1
			processed_node = processed_node + 1
			print "processed node:{}, success node:{}, failed node:{}".format(processed_node,successful_node,failed_node)
			post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node}
			db.updateDB(collection, key, post)
			
		
	def Main_Server_Process(self,endpoint,host_ip,db_ip):

		self.__InitSuccess = self.__Start_LWM2M()
		self.__InitSuccess_client = self.__Start_LWM2M_Client(endpoint,host_ip,db_ip) 
		
		db = database.mongodb()  
		db_check = db.mongodb_connection(db_ip)
		
		if db_check==False:
			sys.exit(Error.DatabaseInitError)
			#self.__InitSuccess_mongodb = self.__Start_mongodb()


		if not self.__InitSuccess:
			sys.exit(Error.ServerInitError)
			
		if not self.__InitSuccess_client :
			sys.exit(Error.ClientInitError)	


	def Kill_Server_Process(self):

		output = subprocess.check_output(["pkill","lwm2mserver1"])

		print output + Error.ServerStop
		
		output_client = subprocess.check_output(["pkill","java"])
		
		print output_client + Error.ClientStop 
		
		output_mongodb = subprocess.check_output(["pkill","mongod"])
		
		
		

