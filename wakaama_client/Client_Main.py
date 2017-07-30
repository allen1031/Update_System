import sys, os, time
import subprocess
from subprocess import Popen, PIPE
import json
import datetime
import database
from Data import *
FILE = "UpdateInfo"

ERROR_INIT = 0
SUCCESS_DOWNLOAD = 1
ERROR_INPUT_RARA = 2
ERROR_UNFOUND_NAME = 3
ERROR_SERVER_NETWORK = 4
ERROR_OUT_OF_MEM = 5
ERROR_UNKNOWN = 6
FAILED_DOWNLOAD = 7

error_init = "update init failed"
success_download = "download success"
error_input_para = "input error"
error_unfound_name = "unfound image"
error_server_network = "network error"
error_out_of_mem = "memory error"
error_unknown = "unknown error"

'''
Update Result Update
'''
DB = "UpdateInfo"
db_name = 'test_database'
mycollection = 'update'
	


def UpdateResult(key,value):
	
	if (os.path.isfile(FILE)):
		with open(FILE,"r") as f:
			data = json.load(f)
			
		if key not in data:
			obj = {key: value}
			data.update(obj)
		else: 
			data [key] = value
			
		with open(FILE,"w") as f:
			f.write(json.dumps(data))
			
	else:
		with open(FILE,"w") as f:
			obj = {key: value}
			json.dump(obj,f)
	
	
		
'''
Main Update Process
'''	

def urlParse(url):
		
	url_comp = url.split("/")
	
	print url_comp
	
	if len(url_comp) <= 1 or url.find("http") < 0:
	
		url_comp = None

		return url_comp
	
	return url_comp	
		
def SWUpdate(exec_file,package_url):	
	
	try:
		url_comp = urlParse(package_url)
		
		if url_comp == None:
			print INVALID_URL
			return ERROR_INPUT_RARA
		content = url_comp[len(url_comp)-1]
		print content

		UpdateProcess = subprocess.Popen(["sudo",exec_file,package_url,"50",content],stdout=subprocess.PIPE, stdin=subprocess.PIPE)
	except:
		print "some thing bad happens"
		return False 
	'''	
	while True:
	  line = UpdateProcess.stdout.readline()
	  if line != '':
		#the real code does filtering here
		print "test:", line.rstrip()
	  else:
		break
	'''	
	
	exit_code = UpdateProcess.wait()
	
	print exit_code
	return exit_code
	
	
'''
Update Management
'''


def UpdateManagement():
	
	#flushing log file
	update_result = 0
	update_state = 2
	duration = 0
	UpdateResult("update_result",update_result)
	UpdateResult("update_state",update_state)
	UpdateResult("update_duration",duration)
	
	
	print sys.argv
	'''
	if len(sys.argv) <= 1 or len(sys.argv) > 2:
		print "No package url, or more than one url\n"
		return 1
	'''	
	
	
	
	
	
	start_time = time.time()
	
	package_url = sys.argv[1]
	db_ip = sys.argv[2]
	brokerNo = sys.argv[3]
	endpoint = sys.argv[4]
	
	##################################################
	###########db connection & initialization#########
	'''
	db_ip = sys.argv[2]
	brokerNo = sys.argv[3]
	endpoint = sys.argv[4]
	
	id = endpoint
	key = {Data.ROLE:"Client",Data.ID:id,Data.BROKER:brokerNo}
	
	db = database.mongodb()  
	mongoclient = db.connectDB(db_ip,27017)
	mycoll = db.initDB(mongoclient,key) # initialize my collection
	
	post = {Data.BROKER:brokerNo,Data.update_url:package_url}
	db.updateDB(mycoll,key,post)
	'''
	################################################
	
	print "downloading is starting"

	exit_code = SWUpdate("./download.sh",package_url)
	
	print "downloading is finished"
	update_state = 3
	UpdateResult("update_state",update_state)
	#exit = 0
	if exit_code != None:
			#exit = 1
		if exit_code == ERROR_INIT:
			print error_init
			update_result = ERROR_INIT	
		if exit_code == SUCCESS_DOWNLOAD:
			print success_download
			update_result = SUCCESS_DOWNLOAD
			#exit = 1
		if exit_code == ERROR_INPUT_RARA:
			print error_input_para
			update_result = ERROR_INPUT_RARA
			#exit = 1
		if exit_code == ERROR_UNFOUND_NAME:
			print error_unfound_name
			update_result = ERROR_UNFOUND_NAME
			#exit = 1
		if exit_code == ERROR_SERVER_NETWORK:
			print error_server_network
			update_result = ERROR_SERVER_NETWORK
			#exit = 1
		if exit_code == ERROR_OUT_OF_MEM:
			print error_out_of_mem
			update_result = ERROR_OUT_OF_MEM
			#exit = 1
		if exit_code == ERROR_UNKNOWN:
			print error_unknown
			update_result = ERROR_UNKNOWN
			#exit = 1
		if exit_code == FAILED_DOWNLOAD:
			update_result = FAILED_DOWNLOAD
			#exit = 1
	else: 
		update_result = ERROR_UNKNOWN
	
	update_state = 1
	update_duration = round((time.time() - start_time),2)
	
	
	UpdateResult("update_state",update_state)
	UpdateResult("update_result",update_result)
	UpdateResult("update_duration",update_duration)	
	
	'''
	post = {Data.update_result:update_result, Data.update_state: update_state, Data.update_time_in_device:update_duration}
	db.updateDB(mycoll,key,post)
	
	mycoll = db.initDB(mongoclient,key) # initialize my collection
	'''
		
if __name__=='__main__':
    UpdateManagement()
