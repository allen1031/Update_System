import sys, os, time
import subprocess
import serv
from Data import *
#import ServiceAdvertise
#import MqttBroker
#import HttpServer
import database

key = {Data.TITLE:'UPDATE'}
db_name = 'test_database'
mycollection = 'update'

def Broker():

	'''
	python Broker_File db_addr lwm2m_server_addr endpoint(name registered on cloud lwm2m server) 
	'''
	print len(sys.argv)
	if len(sys.argv) == 1:
		db_ip = "localhost"
		lwm2m_server_ip = "localhost"
		endpoint = "pi"
	elif len(sys.argv) == 4:
		db_ip = str(sys.argv[1]) 
		lwm2m_server_ip = str(sys.argv[2]) 
		endpoint = str(sys.argv[3])
	else:
		print "The input argument is invalid, please check it"
		return 1	
	print str(sys.argv)
	
	
	serverObj = serv.SP()	
	serverObj.Main_Server_Process(endpoint,lwm2m_server_ip,db_ip)
	#MongoDB Initialization
	db = database.mongodb()  

	mongoclient = db.connectDB(db_ip,27017)
	
	id = endpoint
	key = {Data.ROLE:"Broker",Data.ID:id}
	
	mycoll = db.initDB(mongoclient,key) # initialize my collection
	 
	
	print "lwm2m server is started"
	
	non_localtest = 0
	flag = "old"
	"""
	local test : localtest = 1
	"""
	if (non_localtest):
	
		while True:
			
			time.sleep(0.5)
			array = db.getObject(mycoll,key)
			if (array == None):
				print "bad request"
				return 1
			
			flag = array[Data.REQUEST_FLAG]
			flag = str(flag)
			#print flag
			
			if (flag == "Unset"):
				
				post = {Data.REQUEST_FLAG:'old'}
				mycoll = db.updateDB(mycoll,key,post)
				array = db.getObject(mycoll,key)
				print array[Data.REQUEST_FLAG]
				
				#return 0
			
			if (flag == "new"):
			
				#usr_input = serverObj.clientObserv(Data.USER_INPUT)
				#serverObj.Lwm2mClientWrite(Data.REQUEST_FLAG,"old")
				usr_input = array[Data.USER_INPUT]
				post = {Data.REQUEST_FLAG:'old'}
				mycoll = db.updateDB(mycoll,key,post)
				
				
				#print "user input now is {}".format(usr_input)
				if(usr_input == "Unset"):
				
					print "no request from lwm2m client (from cloud)"
					break
		
				if(usr_input == "exit"):
				
					break
					
				if(usr_input == "list"):
				
					print "list start"
					serverObj.ClientFinder(db, mycoll, key)
					print "user input now is {}".format(usr_input);
				
				
				if(usr_input == "read"):
					
					print "enter the client number"
				
					#client = str(serverObj.clientObserv(Data.CLIENT_NUM))
					client = array[Data.CLIENT_NUM]
					
					print client
					
					print "enter the resource url"
					
					#url = str(serverObj.clientObserv(Data.URL))
					url = array[Data.URL]
					
					print url 
			
					
					serverObj.ClientReader(db, mycoll, key, client, url)
				
				
				if(usr_input == "write"):
				
					print "enter the client number"
				
					#client = str(serverObj.clientObserv(Data.CLIENT_NUM))
					client = array[Data.CLIENT_NUM]
					print "enter the resource url"
					
					#url = serverObj.clientObserv(Data.URL)
					url = array[Data.URL]
					
					#value = str(serverObj.clientObserv(Data.VALUE))
					value = array[Data.VALUE] 
					
					serverObj.ClientWriter(db, mycoll, key, client, url, value)
					
					
					
					
				if(usr_input == "Execute"):
				
					print "enter the client number"
				
					#client = str(serverObj.clientObserv(Data.CLIENT_NUM))
					client = array[Data.CLIENT_NUM]
					
					print "enter the resource url"
					
					#url = serverObj.clientObserv(Data.URL)
					url = array[Data.URL]
					
					serverObj.ClientExecuter(db, mycoll, key, client, url)
					
				if(usr_input == "Update"):
					start_time = time.time()
					num_nodes2update = array[Data.NUM_NODE2UPDATE]
					print "total number to be updated : {}".format(num_nodes2update)
					
					if num_nodes2update <= 0:
						print "no client can be operated"
						return 1
						
					update_url = array[Data.UPDATE_URL]
					
					if update_url == None:
						print "INVALID URL"
						return 1
				 
					#serverObj.GroupUpdate(db,mycoll,key,num_nodes2update, update_url)
					serverObj.ClientUpdate(db_ip,db,mycoll,key,num_nodes2update, update_url)
					duration_broker = round((time.time() - start_time),2)
					post = {Data.update_time_in_broker:duration_broker}
					db.updateDB(mycoll,key,post)
					#client = str(serverObj.clientObserv(Data.NUM_NODES2UPDATE))
					
					print "The number of nodes to be updated is {}"
				
	
	else:
		"""
		interface to lwm2m client : localtest = 0
		"""
		while (1):
			
			usr_input = raw_input()
						
			if(usr_input == "exit"):
			
				break
			
			if(usr_input == "list"):
				
				flag = serverObj.clientObserv(Data.REQUEST_FLAG)
				
				
				#updateInfo = db.find_one(key)
				
				#print updateInfo[Data.USER_INPUT]
				
				
				serverObj.ClientFinder(db,mycoll,key)
		
			
			if(usr_input == "check"):
			
				print "enter the client number"
				
				client = raw_input()
				
				print "enter the resource url"
				
				url = raw_input()
				
				result = serverObj.requestValidation(client,url)
				
				print result 
			
			
			if (usr_input == "read"):
			
				print "enter the client number"
				
				client = '0'
				
				print "enter the resource url"
				
				url = raw_input()
				
				#url = raw_input()
				
				#value = serverObj.ReadNumFromLwm2m(client, url)
				
				value = serverObj.WakaamaRead(client,url)
				
				print "read value is {}".format(value)
				
				
			if (usr_input == "write"):
				
				print "enter the client number"
				
				client = '0'
				
				print "enter the resource url"
				
				#url = "/10400/0/1"
				url = "/1025/0/1"
				
				print "enter the input value"
				
				value = 'http://192.168.2.15:8080/windows-client5.jar'
				
				#write = serverObj.WriteToLwm2m(client, url,value)
				serverObj.WakaamaWrite(client, url,value)
				
				#print "write result is {}".format(write)
				
			if (usr_input == "exec"):
			
				print "enter the client number"
				
				client = "0"
				
				print "enter the resource url"
				
				url = "/1025/0/2"
				
				execute = serverObj.WakaamaExecute(client, url)
				
				#print "execute result is {}".format(execute)
				
			if (usr_input == "test"):
				
				client = "0"
				
				url = "/1025/0/1"
				
				serverObj.pipe_communication(client,url);
				
			
	serverObj.Kill_Server_Process()
	


if __name__=='__main__':
    Broker()