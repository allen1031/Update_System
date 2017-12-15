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
TIME_COMMAND = "time"

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


"""
 *	lwm2m objects : /1025 container app
 *	
 *	resources : 
 *				
 *             
"""
instanceNo = '0'
url_device_role          = '/1025'+instanceNo+'/0'
url_device_name          = '/1025'+instanceNo+'/1'
url_container_supported  = '/1025'+instanceNo+'/2'
url_image_name           = '/1025'+instanceNo+'/3'
url_image_status         = '/1025'+instanceNo+'/4'
url_exec_cmd             = '/1025'+instanceNo+'/5'
url_trigger              = '/1025'+instanceNo+'/6'
url_container_status     = '/1025'+instanceNo+'/7'
url_container_id         = '/1025'+instanceNo+'/8'
url_kill                 = '/1025'+instanceNo+'/9'
url_container_url        = '/1025'+instanceNo+'/10'
url_update               = '/1025'+instanceNo+'/11'
url_update_state         = '/1025'+instanceNo+'/12'
url_update_result        = '/1025'+instanceNo+'/13'
url_duration             = '/1025'+instanceNo+'/14'



class SP():
	def __init__(self):
		
		#main processes intialization 
		self.__InitSuccess = False
		self.__InitSuccess_client = False
		self.__InitSuccess_mongodb = False
		self.__InitSuccess_server = False
		
		#client variables 
		self.__clients = {}
		self.__clients_names = {}
		self.__num_clients = 0
		self.__client_no = 0
		self.__objects = {"init":"empty"}
		self.__client_duration = {}
		self.__client_result = {}
		
		#other variables
		self.__line = 0
		self.__input = ""
		self.__obj = ""
		self.__resource_id = 0
		self.__mongodb = None
		url_value = [];
	
	def __del__(self):

		self.Kill_Server_Process()

	"""
		initiate main programs in Broker
		+ programs
		|	+ __Start_LWM2M        : lwm2m server
		|	+ __Start_LWM2M_Client : lwm2m client 
		|   + __Start_httpserver   : fIle server (http server)
		|   + __Start_mongodb      : db
	"""

	# Subprocess: LWM2M Server
	def __Start_LWM2M(self,db_addr,db_name,collection_name):
		try:
			print "lwm2m is starting"
			self.__lwm2m =subprocess.Popen(['./lwm2mserver', '-4', '-a',db_addr,'-d', db_name, '-b', collection_name], stdout=subprocess.PIPE, stdin=subprocess.PIPE)

		except:

			return False
		self.__nbsr = NBSR(self.__lwm2m.stdout)

		print Error.ServerInitSuccess
		#Time for the server to stabilize
		time.sleep(2)

		return True
		
	#Subprocess : API to Cloud
	def __Start_LWM2M_Client(self,endpoint,host_ip,db_ip,db_name,collection_name):
		try:
			#self.__lwm2m_client =subprocess.Popen(['java','-jar','client-0.0.jar','-n','zheng','-u','131.155.241.109'], stdout=subprocess.PIPE, stdin=subprocess.PIPE)
			self.__lwm2m_client =subprocess.Popen(['./lwm2mclient', '-4', '-h', host_ip, '-d', db_name, '-n', endpoint, '-m', collection_name], stdout=subprocess.PIPE, stdin=subprocess.PIPE)
		except:
			return False
		self.__nbsr_client = NBSR(self.__lwm2m_client.stdout)
		print self.__nbsr_client
		print Error.ClientInitSuccess
		#Time for the server to stabilize
		time.sleep(2)

		return True
	
	# Subprocess : File server start from 
	def Start_private_registry(self):
		try:
			self.__httpserver = subprocess.Popen(['python','-m','SimpleHTTPServer','8080'],stdout=subprocess.PIPE, stdin=subprocess.PIPE)
		except :
			return False
		self.__nbsr_file_transfer = NBSR(self.__httpserver.stdout)
		print "http servers begin successfully"
		return True
	
	# Subprocess : File server start from 
	def __Start_httpserver(self):
		try:
			self.__httpserver = subprocess.Popen(['python','-m','SimpleHTTPServer','8080'],stdout=subprocess.PIPE, stdin=subprocess.PIPE)
		except :
			return False
		self.__nbsr_file_transfer = NBSR(self.__httpserver.stdout)
		print "http servers begin successfully"
		return True
		
	#SUbprocess : MongoDB database
	def __Start_mongodb(self):
		try:
			self.__mongodb = subprocess.Popen(['mongod'],stdout=subprocess.PIPE, stdin=subprocess.PIPE)
		except :
			return False
		self.__nbsr_db = NBSR(self.__mongodb.stdout)
		print self.__nbsr_db
		print Error.DatabaseInitSuccess
		return True
		
	#def __Start_docker_repo(self):
	
		
	"""
	assitant function
	|   + pipe_flush          : flush the pipe stored info
	|   + clientOutputDisplay : lwm2m client debug info 
	|   + serverOutputDisplay : lwm2m server debug info
	|   + urlParse            : parse the lwm2m resource url
	|   + requestValidation   : validate the resource url
	|   + HTTP_URL_PARSE      : parse http urls
	|   + SWDownload          : download package from http url
	|   + Download_and_Forward: forward the downloaded package to local file server

	"""
	# Flush the stdout buffer from LWM2M Server process
	def pipe_flush(self):
		message_queue = " "
		while message_queue:
			message_queue = self.__nbsr.readline(0.01)

	
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
		
		return OBJ_SUCCESS
		
		
	def HTTP_URL_PARSE(self, url):
		
		url_comp = url.split("/")
		
		print url_comp
		
		if len(url_comp) <= 1 or url.find("http") < 0 or url.find("https") < 0:
			
			url_comp = None
			
		return url_comp
		
		
	def SWDownload(self,package_url):	
	
		try:
			url_comp = self.HTTP_URL_PARSE(package_url)
			
			if url_comp == None:
				print INVALID_URL
				return False
			
			content = url_comp[len(url_comp)-1]
			print content

			UpdateProcess = subprocess.Popen(["sudo","wget",package_url],stdout=subprocess.PIPE, stdin=subprocess.PIPE)
		except:
			print "some thing bad happens"
			return False 

	
		exit_code = UpdateProcess.wait()
		
		print exit_code
		
		if exit_code == 0:
			print "package is received in local"
			return True
			
		return False
		
	def Download_and_Forward(self, url,localhost):
		
		download_res = self.SWDownload(url)
		
		value = None
		
		port = 8080
		
		if download_res == True:
		
			url_comp = self.HTTP_URL_PARSE(url)
			
			content = url_comp[len(url_comp)-1]
			
			value = "http://" + localhost +":"+ str(port) +"/" + content
			
			print value
			
			
		return value
	
	

		
	"""
		JSON Files
		+ Lwm2m client communication 
		|
		|	+ clientObserv     : Read command from lwm2m client through json file
		|	+ Lwm2mClientWrite : Write feedback to lwm2m client through json file
		
		
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
		|  + exec  : execue resource value
	
	"""
	
	
	
		
		
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
		
		
	def wakaamaObserve(self, clientNo, url):
		
		OBSERVE_REQUEST = OBSERVE_COMMAND + ' ' + str(clientNo) + ' ' + url
		
		self.pipe_flush()
		
		self.__lwm2m.stdin.write(OBSERVE_REQUEST)
		
		print "\n OBSERVE COMMAND \n"
		
		print OBSERVE_REQUEST
		
		#time.sleep(2)
		
		tmp_output = " "
		
	
	def WakaamaRead(self, clientNo , url):
		
		READ_REQUEST = READ_COMMAND + ' ' + str(clientNo) + ' ' + url
		
		self.pipe_flush()
		
		self.__lwm2m.stdin.write(READ_REQUEST)
		
		print "\n READ COMMAND \n"
		
		print READ_REQUEST
		#time.sleep(1)
		
		'''
		primary_search = "Client #" + str(clientNo) + " " + url + " : " + "2.05"
		time.sleep(2)
		tmp_output = " "

		line = 0
		read_mark = 0
		while tmp_output:
			print "The {} line is : {}".format(line, tmp_output)
			line = line + 1
			
			if tmp_output.find(primary_search)>=0:
				line1 = tmp_output
				print line1
				line2 = self.__nbsr.readline(0.1)
				print line2
				line3 = self.__nbsr.readline(0.1)
				print line3
				read_mark = 1
			
			tmp_output = self.__nbsr.readline(1)
			
		
		if (read_mark == 0):
			return READ_FAILURE
		else:
			value = self.__Parser(line1,line2,line3)
			return value
		'''
	
	def WakaamaWrite(self, clientNo , url, write_value):
	
		'''
			start the write request 
		'''
		WRITE_REQUEST = WRITE_COMMAND + " " + str(clientNo) + " " + url + " " + str(write_value)	
		
		self.pipe_flush()
		self.__lwm2m.stdin.write(WRITE_REQUEST)
		
		print "\n WRITE COMMAND \n"
		
		print "WRITE_REQUEST"
		
		'''
		tmp_output = " "
		#tmp_output = self.__nbsr.readline(1)
		primary_search = "Client #" + str(clientNo) + " " + url + " : " + "2.04"
		line = 0
		while tmp_output:
			print "The {} line is : {}".format(line, tmp_output)
			
			if tmp_output.find(primary_search)>=0:
				return WRITE_SUCCESS
			line = line + 1
			
			tmp_output = self.__nbsr.readline(5)
			
		return WRITE_FAILURE
		'''
	
	#def WakaamaExecute(self,db,collection,key, clientNo , url):
	def WakaamaExecute(self,clientNo , url):
		'''
		array = db.getObject(collection,key)
		client_list = array[Data.CLIENT_LIST]
		client = str(client_list[str(clientNo)])
		brokerID = str(array[Data.ID])
		client_key = {Data.ROLE:"Client",Data.ID:client,Data.broker_id:brokerID}
		print client_key
		
		post = {Data.USER_INPUT:"None",Data.UPDATE_RESULT:"None",Data.UPDATE_STATE:"idle"}
		db.updateDB(collection,client_key,post)		
		
		line = 0
		'''
		EXECUTE_REQUEST = EXECUTE_COMMAND + " " + str(clientNo) + " " + url 
			
		print EXECUTE_REQUEST
		
		
		self.pipe_flush()
		self.__lwm2m.stdin.write(EXECUTE_REQUEST)
		
		print "\n EXECUTE COMMAND \n"
		
		'''
		tmp_output = " "
		primary_search = "Client #" + str(clientNo) + " " + url + " : " + "2.04"
		
		while tmp_output:
			print "The {} line is : {}".format(line, tmp_output)
			
			if tmp_output.find(primary_search)>=0:
				
				return WRITE_SUCCESS
			
			line = line + 1
			
			tmp_output = self.__nbsr.readline(1)
			
		return EXECUTE_FAILURE
		'''
	# local read
	def UpdateWait(self,clientNo,distribution_time):
		
		update_state_url = '/1025/0/12'
		update_result_url = '/1025/0/13'
		update_duration_url = '/1025/0/14'
		
		state = 0
		result = 0
		duration = 0
		
		time.sleep(1)
		
		
		
		
		while (1):
			
			if str(result) != str(0):
				break;
				
			array = db.getObject(collection,client_key)
			
			result = array[Data.update_result]
		
		if str(result) == "1":
			return True
		else:
			return False
		
		'''
		while str(result) == "0":
			time.sleep(2)
			
			#print "state at present : {}".format(state) 
			print "result at present : {}".format(result)
		
			#state = self.WakaamaRead(clientNo, update_state_url)
			result = self.WakaamaRead(clientNo, update_result_url)	
		
		duration = self.WakaamaRead(clientNo, update_duration_url)
		if duration != READ_FAILURE:
			duration = float(duration) #+ distribution_time
		
		print "time in device costs :{}".format(duration)
		self.__client_result.update({str(clientNo) : result})
		self.__client_duration.update({str(clientNo) : duration})
		
		if str(result) == "1":
			return True
		else:
			return False	
		'''
	
	def ClientDBInit(self,db_ip,db,collection,key,group_size):
	
		mongoclient = db.connectDB(db_ip,27017)
		i = 0
		while True:
			if i >= int(group_size):
				break
			
			array = db.getObject(collection,key)
			client_list = array[Data.CLIENT_LIST]
			client = str(client_list[str(i)])
			#brokerID = str(array[Data.ID])
			#client_key = {Data.ROLE:"Client",Data.ID:client,Data.broker_id:brokerID}
			client_key = {Data.ROLE:"Client",Data.device_no:i}
			print client_key
			db.initClientDB(mongoclient,client_key);
			
			i = i + 1
			
	def ClientDBCheck(self,db_ip,db,collection,key,group_size):
	
		mongoclient = db.connectDB(db_ip,27017)
		i = 0
		while True:
			if i >= int(group_size):
				break
			
			array = db.getObject(collection,key)
			client_list = array[Data.CLIENT_LIST]
			client = str(client_list[str(i)])
			#brokerID = str(array[Data.ID])
			#client_key = {Data.ROLE:"Client",Data.ID:client,Data.broker_id:brokerID}
			client_key = {Data.ROLE:"Client",Data.device_no:i}
			print client_key
			db.initClientDB(mongoclient,client_key);
			
			i = i + 1
			
	
	def ClientUpdate(self,db,collection,broker_key,client_key,group_size,package_url):
		
		print "start update 0"
		i = 0
		url = '/1025/0/10' #/10400/0/0
		exe = '/1025/0/11'
		
		value = package_url
		
		processed_node = 0
		successful_node = 0
		failed_node = 0
		process = {}

		start_time = time.time()
		
		while True:
			if i >= int(group_size):
				break
			
			write_result = self.WakaamaWrite(i , url, value)
			
			client_key = {Data.device_role:'End Device', Data.device_no:i}
			
			while 1:
				print '\n write to client \n'
				
				array = db.getObject(collection,client_key)
				
				print "writing value is {} and writed value is {}".format(value, array[Data.container_url])
				
				if int(array[Data.update_state]) == 2 and int(array[Data.update_result]) == 0:
					print '\n write sccussess \n'
					
					successful_node = successful_node + 1	
					break;
					
				if int(array[Data.update_result]) != 0:
					print '\n write failure \n'
					failed_node = failed_node + 1
					break;
				
			i = i + 1
			
			processed_node = processed_node + 1
			print "processed node:{}, success node:{}, failed node:{}".format(processed_node,successful_node,failed_node)
			post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node, Data.TIME_CONSUMPTION:self.__client_duration, Data.CLIENT_UPDATE_RESULT:self.__client_result}
			db.updateDB(collection, broker_key, post)
		
		distribution_time = round((time.time() - start_time),2)
		
		#post = {Data.NUM_PROCESSED_NODE: processed_node,Data.NUM_SUCCESSFUL_NODE:successful_node, Data.NUM_FAILED_NODE:failed_node}
		#db.updateDB(collection, key, post)
		
		'''
		i = 0
		while True:
			
			if i >= int(group_size):
				break
			
			client_key = {Data.device_role:'End Device', Data.device_no:i}
			
			while 1:
				print '\n update to client \n'
				
				array = db.getObject(collection,client_key)
				
				result = int(array[Data.update_result])
				
				if result != 0 :
					print '\n execute finished \n'
					
						
					break
			i = i + 1
		'''	
			
		
	def JSON_READ(self,key,value):
		FILE = "UpdateInfo"
		if (os.path.isfile(FILE)):
			with open(FILE,"r") as f:
				data = json.load(f)
				
			value = data[key]
				
		else:
			value = ""
		
		return value
		
		
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
		
		self.pipe_flush()
		self.__lwm2m.stdin.write(LIST_COMMAND)
		
		print "\n LIST COMMAND \n"
		time.sleep(0.01)
		'''
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
			
				#post = {Data.NUM_CLIENTS : num_client}
				#db.updateDB(collection, key, post)
				#self.Lwm2mClientWrite(Data.OBJECT,self.__objects)
				
				#self.Lwm2mClientWrite(Data.SEND_FLAG,"done")
			
			#timesleep(0.01)
			tmp_output = self.__nbsr.readline(0.01)
		
		print self.__clients_names
		i = 0
		client_list = {}
		
		while i<(num_client):
			self.wakaamaObserve(i,'/1025/0')
			
		
		
		
		#self.Lwm2mClientWrite(Data.NUM_CLIENTS,num_client)
		print self.__clients_names
		i = 0
		client_list = {}
		while i<(num_client):
			
			client_list = self.merge_two_dicts(client_list , {str(i) : self.__clients_names[i]})
			#client_list.append(self.__clients[i])
			i = i + 1
			
			client_key = {Data.device_role:"End Device",Data.device_name:self.__clients_names[i]}
			client_post = {Data.device_no:i}
			db.update(collection,client_key,post)
		
		post = {Data.NUM_CLIENTS : num_client, Data.CLIENT_LIST : client_list}
		db.updateDB(collection, key, post)
		'''
		
	def Main_Server_Process(self,db_name,endpoint,host_ip,db_ip):

		
		collection_name = endpoint
		print "database: {},{},{}".format(db_ip,db_name,endpoint)
		
		self.__InitSuccess = self.__Start_LWM2M(db_ip,db_name,collection_name)
		self.__InitSuccess_client = self.__Start_LWM2M_Client(endpoint,host_ip,db_ip,db_name,collection_name) 
		
		db = database.mongodb()  
		db_check = db.mongodb_connection(db_ip)
		
		#self.__InitSuccess_httpserver = self.__Start_httpserver()
		
		if db_check==False:
			sys.exit(Error.DatabaseInitError)
		if not self.__InitSuccess:
			sys.exit(Error.ServerInitError)
		if not self.__InitSuccess_client :
			sys.exit(Error.ClientInitError)	
		#if not self.__InitSuccess_httpserver:
		#	sys.exit("Error starting http server")

	def Kill_Server_Process(self):

		output = subprocess.check_output(["sudo","pkill","lwm2mserver1"])

		print output + Error.ServerStop
		
		output_client = subprocess.check_output(["sudo","pkill","java"])
		
		print output_client + Error.ClientStop 
		
		output_mongodb = subprocess.check_output(["sudo","pkill","mongod"])
		
		
		output_mongodb = subprocess.check_output(["sudo","pkill","python"])
		
		print output_mongodb + Error.DatabaseStop 
		

