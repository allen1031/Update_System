int container_trigger(lwm2m_object_t * objectP,uint16_t instanceId) {
	
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	if (strlen(data->image_name)==0){
		return -1;
	}
	
	if (strcmp(data->container_status,OFF)==0){
		char check_cmd[100] = "docker run -it -d ";
		strcat(check_cmd,data->image_name);
	}else if(strcmp(data->container_status,ON)==0){
		if(data->pid != NULL){
			char check_cmd[100] = "docker stop ";
			strcat(check_cmd,data->pid);
		}else{
			return -1;
		}
	}
	
	return thread_creation(data,check_cmd,trigger_thread);
}


int trigger_thread(void *arg){
	
	app_control_data_t *obj = (app_control_data_t*) arg;
	char *proc_pid = (char *)obj->appObject->pid;
	char *image_name = (char *)obj->appObject->pid;
	
	if (strcmp(obj->appObject->image_status,OFF)==0){
		
		char check_pid[128]={};
		char short_pid[128]={};
		char tmp_pid[128]={};
		
		FILE *fp;
		/* Open the check command for reading. */
		printf("Check command : %s\n", obj->cmd);
		
		fp = popen(obj->cmd, "r");
		if (fp == NULL) {
			printf("Failed to run command\n" );
			strcpy(obj->appObject->pid,NONE);
			fireResourceChanged(common_context, pidURI, NONE);
			return 1;
		}
		
		while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
			
			strcat(check_pid, tmp_pid);
			printf("%s",tmp_pid);
		}
		
		fclose(fp);
		
		printf("the check pid is %s\n",check_pid);
		
		if(strlen(check_pid)==0){
			
			strcpy(obj->appObject->container_status,OFF);
			fireResourceChanged(common_context, statusURI, OFF);
			strcpy(obj->appObject->pid,NONE);
			fireResourceChanged(common_context, pidURI, NONE);
			
			return 1;
		}
		else{
			strncpy(short_pid,check_pid,12);
			*(short_pid + 12) = 0;
			printf("the check process id is %s\n",short_pid);
			
			return trigger_check(short_pid,proc_pid);

		}
	}else if(strcmp(obj->appObject->image_status,ON)==0){
		
		int ret = system(obj->cmd);
		if(ret==0){
			strcpy(obj->appObject->container_status,OFF);
			fireResourceChanged(common_context, statusURI, OFF);
			strcpy(obj->appObject->pid,NONE);
			fireResourceChanged(common_context, pidURI, NONE);
			return 0
		}else{
			return ret;
		}
	}
}



int trigger_check(char* short_pid,char* proc_pid){
	
	char check_pid[128]={};
	char final_pid[128]={};
	char tmp_pid[128]={};
	char check_cmd[] = "docker ps --filter id=";
	strcat(check_cmd,short_pid);
	
	int i;
	int a = 0;
	FILE *fp;
	//memset(check_pid,0,strlen(check_pid));
	/* Open the check command for reading. */
	printf("Check command : %s\n", check_cmd);
	
	fp = popen(check_cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		strcpy(proc_pid , "none"); 
		return 0;
	}
	
	while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
		strcat(check_pid, tmp_pid);
		printf("%s",tmp_pid);
	}
	
	if(strlen(check_pid)==0){
		
		strcpy(obj->appObject->container_status,OFF);
		fireResourceChanged(common_context, statusURI, OFF);
		strcpy(obj->appObject->pid,NONE);
		fireResourceChanged(common_context, pidURI, NONE);
		
		return 1;
	}
	else{
		strcpy(obj->appObject->container_status,ON);
		fireResourceChanged(common_context, statusURI, ON);
		strcpy(obj->appObject->pid,proc_pid);
		fireResourceChanged(common_context, pidURI, proc_pid);
		return 0;
	}
	
	printf("the pid of proceeding container is %s\n",proc_pid);
	 /* close */
	pclose(fp);
	return 1;
}

