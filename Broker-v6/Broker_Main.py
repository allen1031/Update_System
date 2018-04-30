import sys, os, time
import subprocess
import serv
from Data import *
import ServiceAdvertise
import database

broker_key = {Data.ROLE:'broker'}
client_key = {Data.device_role:'End Device'}
 
def Broker():

	'''
	Read Command Input(ARG): file_name db_addr lwm2m_server_addr endpoint_name(name registered on server) 
	'''
	print len(sys.argv)
	if len(sys.argv) == 1:
		db_ip = "localhost"
		lwm2m_server_ip = "localhost"
		endpoint = "pi32"
	elif len(sys.argv) == 5:
		db_ip = str(sys.argv[1]) 
		lwm2m_server_ip = str(sys.argv[2]) 
		endpoint = str(sys.argv[3])
		localhost = str(sys.argv[4])
	else:
		print "The input argument is invalid, please check it"
		return 1	
	print str(sys.argv)
	
	'''
	Publish the Broker Service
	'''
	
	arg = ["avahi-publish-service",endpoint,
		"_coap._udp","5683","/Broker",
		"--sub","_"+endpoint+"._sub._coap._udp"]
		
	serviceObj = ServiceAdvertise.SA()
	serviceObj.Advertise_Services(arg)
	print "the service has been published\n"
	db_name = 'software_update'
	
	'''
	Triggering Main Processes
	'''
	serverObj = serv.SP()	
	serverObj.Main_Server_Process(db_name,endpoint,lwm2m_server_ip,db_ip)
	
	'''
	MongoDB Initialization
	'''
	
	mycollection = endpoint
	db = database.mongodb()
	id = endpoint
	mycoll = db.initDB(db_ip,db_name,endpoint,broker_key,id) 
	
	'''
	Broker Routine Check 
	'''
	non_localtest = 1
	flag = "old"
	time_count = 0
	old_num = 0
	
	if (non_localtest):
	
		while True:
			
			time.sleep(0.5)
			
			array = db.getObject(mycoll,broker_key)
			
			if (array == None):
				print "cannot get items arrary from the database"
				return 1
			
			flag = array[Data.REQUEST_FLAG]
			flag = str(flag)
			
			# observe the new client list if the client registration is updated
			
			if (array[Data.NUM_CLIENTS] > 0):
				
				if (array[Data.NUM_CLIENTS] != old_num):
					
					old_num = array[Data.NUM_CLIENTS]
					tmp_num = 0
					
					while True:
					
						if (tmp_num >= old_num):
							break;
							
						print old_num
						
						url = '/1025/0'
						
						serverObj.wakaamaObserve(tmp_num, url)
						
						tmp_num = tmp_num + 1
			
			# Empty request
			
			if (flag == "Unset"):
				
				post = {Data.REQUEST_FLAG:'old'}
				mycoll = db.updateDB(mycoll,broker_key,post)
				array = db.getObject(mycoll,broker_key)
				print array[Data.REQUEST_FLAG]
				
				
			# New request 
			
			if (flag == "new"):
				
				usr_input = array[Data.USER_INPUT]
				post = {Data.REQUEST_FLAG:'old'}
				mycoll = db.updateDB(mycoll,broker_key,post)
				
				print "user input now is : {}".format(usr_input);
				
				if(usr_input == "Unset"):
				
					print "no request from server"
					
		
				if(usr_input == "exit"):
					
					break
					
				if(usr_input == "list"):
					print "list start"
					serverObj.ClientFinder(db, mycoll, broker_key)
				
				if(usr_input == "read"):
					
					print "read start"
					clientNo = array[Data.CLIENT_NUM]
					print "client number : {}".format(clientNo)
					
					url = array[Data.URL]
					print "resource url : {}".format(url)
					
					url = array[Data.URL]
					url_array = url.split("/")
					objectid = int(url_array[1]) - 1
					instanceid = int(url_array[2])
					resourceid = int(url_array[3]) - 1
					
					device_url = '/'+str(objectid)+'/'+str(instanceid)+'/'+str(resourceid) 
					
					print url 
					serverObj.WakaamaRead(clientNo , device_url)
					
				if(usr_input == "write"):
				
					clientNo = array[Data.CLIENT_NUM]
					print "client number : {}".format(clientNo)
					
					url = array[Data.URL]
					print "resource url : {}".format(url)
					
					url_array = url.split("/")
					objectid = int(url_array[1]) - 1
					instanceid = int(url_array[2])
					resourceid = int(url_array[3]) - 1
					
					device_url = '/'+str(objectid)+'/'+str(instanceid)+'/'+str(resourceid) 
					
					value = array[Data.VALUE] 
					
					serverObj.WakaamaWrite(clientNo , device_url, value)
					
				
				if(usr_input == "exec"):
				
					clientNo = array[Data.CLIENT_NUM]
					print "client number : {}".format(clientNo)
					
					url = array[Data.URL]
					print "resource url : {}".format(url)
					
					url = array[Data.URL]
					url_array = url.split("/")
					objectid = int(url_array[1]) - 1
					instanceid = int(url_array[2])
					resourceid = int(url_array[3]) - 1
					
					device_url = '/'+str(objectid)+'/'+str(instanceid)+'/'+str(resourceid) 
					
					serverObj.WakaamaExecute(clientNo , device_url)
					
					
				if(usr_input == "group update"):
					start_time = time.time()
					num_nodes2update = array[Data.NUM_CLIENTS]
					print "total number to be updated : {}".format(num_nodes2update)
					
					if num_nodes2update <= 0:
						print "no client can be operated"
						return 1
						
					update_url = array[Data.UPDATE_URL]
					
					forward_url = update_url
					
					if forward_url == None:
						print "INVALID URL"
						return 1
					
					serverObj.ClientUpdate(db,mycoll,broker_key,client_key,num_nodes2update,forward_url)
					duration_broker = round((time.time() - start_time),2)
					post = {Data.update_time_in_broker:duration_broker}
					db.updateDB(mycoll,broker_key,post)
					
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
				
				#url = raw_input()
				
				url = "/1025/0/1"
				
				i = 0
				while i<10:
			
					value = serverObj.WakaamaRead(i,url)
				
					print "read value is {}".format(value)
					
					i = i + 1
			'''
			if (usr_input == "rtest"):
				
				client = '0'
				url = "/1025/0/4"
				i = 0
				while i < 10:
					time.sleep(1)
					value = serverObj.WakaamaRead(client,url)
					print "read value is {}".format(value)
					i = i + 1
			'''		
				
			if (usr_input == "write"):
				
				print "enter the client number"
				
				client = '0'
				
				print "enter the resource url"
				
				#url = "/10400/0/1"
				url = "/1025/0/1"
				
				print "enter the input value"
				
				value = 'http://192.168.2.15:8080/windows-client5.jar'
				
				#write = serverObj.WriteToLwm2m(client, url,value)
				i = 0
				while i<10:
				
					time.sleep(1)
					
					write = serverObj.WakaamaWrite(i, url,value)
				
					#print "write result is {}\n".format(write)
				
					i = i + 1
					
					
			if (usr_input == "exec"):
			
				print "enter the client number"
				
				client = "0"
				
				print "enter the resource url"
				
				url = "/1025/0/2"
				
				execute = serverObj.WakaamaExecute(client, url)
				
				#print "execute result is {}".format(execute)
				
			if (usr_input == "test"):
				
				mode = local
				
				serverObj.sw_distribution(mode)
				
			
	serverObj.Kill_Server_Process()
	

if __name__=='__main__':
    Broker()