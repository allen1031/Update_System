import sys, os, time
import subprocess
from subprocess import Popen, PIPE
import json
import datetime
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
def DataInit():
	with open(FILE, "w") as f:
		obj = {"update_state":1,"update_result":0,"update_duration":0.00,"package":""}
		json.dump(obj,f)


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
	
	'''
	make the file to be executable first
	'''
	
	try:
		url_comp = urlParse(package_url)
		
		if url_comp == None:
			print INVALID_URL
			return ERROR_INPUT_RARA
		content = url_comp[len(url_comp)-1]
		print content
		UpdateResult("package",content)	

		UpdateProcess = subprocess.Popen(['sudo',exec_file,package_url,"50",content],stdout=subprocess.PIPE, stdin=subprocess.PIPE)
	except:
		print "Subprocess created failed on the device"
		return False 
		
	server_wait = 0;
	while True:
	  line = UpdateProcess.stdout.readline()
	  print "exec file is processing"
	  if line != '':
		#the real code does filtering here
		print "test:", line.rstrip()
	  else:
		break
		
	
	exit_code = UpdateProcess.wait()
	
	print "the exit_code is {}".format(exit_code) 
	
	if (not os.path.isfile(content)):
		exit_code = FAILED_DOWNLOAD;
		
		
	
	print exit_code
	return exit_code
	
	
'''
Update Management
'''


def UpdateManagement():
	
	#print sys.argv
	#print len(sys.argc)
	
	
	if len(sys.argv) <= 1 or len(sys.argv) > 2:
		print "No package url, or more than one url\n"
		return 7
	
	DataInit();
	
	start_time = time.time()
	
	package_url = sys.argv[1]
	
	
	
	print "downloading is starting"
	update_state = 2
	UpdateResult("update_state",update_state)
	
	exit_code = SWUpdate("./update-git.sh",package_url)
	
	print "downloading is finished"
	update_state = 3
	UpdateResult("update_state",update_state)
	#exit = 0
	if exit_code != None:
			#exit = 1
		if exit_code == ERROR_INIT:
			print error_init
			update_result = 8
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
	

		
if __name__=='__main__':
    UpdateManagement()