/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Julien Vermillard - initial implementation
 *    Fabien Fleutot - Please refer to git log
 *    David Navarro, Intel Corporation - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    
 *******************************************************************************/

/*
 * This object is single instance only, and provide firmware upgrade functionality.
 * Object ID is 5.
 */

/*
 * resources:
 * 0 package                   write
 * 1 package url               write
 * 2 update                    exec
 * 3 state                     read
 * 4 update supported objects  read/write
 * 5 update result             read
 */

#include "liblwm2m.h"
#include "lwm2mclient.h"
#include "object_software.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>


//#include <update-func.h>

// ---- private object "Firmware" specific defines ----
// Resource Id's:
/*
#define RES_M_PACKAGE                   0
#define RES_M_PACKAGE_URI               1
#define RES_M_UPDATE                    2
#define RES_M_STATE                     3
#define RES_M_DOWNLOAD_CMD  			4
#define RES_M_UPDATE_RESULT             5
#define RES_O_PKG_NAME                  6
#define RES_O_PKG_VERSION               7
#define RES_M_DURATION                  8
#define RES_M_PID                       9
#define RES_M_KILL						10
*/
#define RES_O_DEVICE_ROLE               0
#define RES_O_DEVICE_NAME               1  //RW String
#define RES_O_CONTAINER_SUPPORTED       2  //RW String
#define RES_O_PKG_NAME                  3  //RW String
#define RES_O_PKG_STATUS                4  //RW String
#define RES_M_DOWNLOAD_CMD  			5  
#define RES_O_TRIGGER					6  //E  Opaque
#define RES_O_STATUS					7  //R  String
#define RES_M_PID                       8  //R  String
#define RES_M_KILL						9  //E  Opaque  
//RW String
#define RES_M_PACKAGE_URI               10  //RW String
#define RES_M_UPDATE                    11 //E  Opaque
#define RES_M_STATE                     12 //RW Integer
#define RES_M_UPDATE_RESULT             13 //RW Integer
#define RES_M_DURATION                  14 //RW Float

#define NUM_INSTANCE                    1
#define MAX_NUM_INSTANCE                2


char roleURI[9]="/1025/0/0";
char deviceURI[9]="/1025/0/1";
char supportURI[9]="/1025/0/2";
char inameURI[9]="/1025/0/3";
char istatusURI[9]="/1025/0/4";
char cmdURI[9]="/1025/0/5";
char triggerURI[9]="/1025/0/6";
char statusURI[9]="/1025/0/7";
char pidURI[9]="/1025/0/8";
char killURI[9]="/1025/0/9";

char urlURI[10]="/1025/0/10";
char updateURI[10]="/1025/0/11";
char stateURI[10]="/1025/0/12";
char resultURI[10]="/1025/0/13";
char durationURI[10]="/1025/0/14";


char *image_on_board="on board";
char *image_off_board="off board";

int IDLE=0;
int DOWNLOADING=1;
int DOWNLOADED=2;
int UPDATING=3;

int init=INIT;
int update_success=UPDATE_SUCCESS;
int update_failure=UPDATE_FAILURE;
int system_running_error=SYSTEM_RUNNING_ERROR;
char RUNNING[10]="running";
char PAUSED[10]="paused";
char EXITED[10]="exited";
//char OFF[10] = "OFF";
//char ON[10] = "ON";
char NONE[1024]= "none";
time_t start_t, end_t;
double diff_t;
typedef struct _server_instance_
{
   int brokerNo; // registered brokerNo
   char name[255];  
} software_instance_t;

application_data_t * softwareInstance;
//lwm2m_context_t * common_context;

static uint8_t prv_software_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP);
static uint8_t prv_software_write(uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP);
static uint8_t prv_software_execute(uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP);
								  

	

lwm2m_object_t * get_object_software(int brokerNo, char* name,lwm2m_context_t * context)
{
    /*
     * The get_object_firmware function create the object itself and return a pointer to the structure that represent it.
     */
	
	 
    lwm2m_object_t * containerObj;

    containerObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
	
    if (NULL != containerObj)
    {
		int i;
		application_data_t* targetP;
		
        memset(containerObj, 0, sizeof(lwm2m_object_t));

        /*objectID : 1025*/
        containerObj->objID = LWM2M_SOFTWARE_UPDATE_OBJECT_ID;
       
		for (i=0 ; i < NUM_INSTANCE ; i++){
			targetP = (application_data_t*)lwm2m_malloc(sizeof(application_data_t));
			if (NULL == targetP) return NULL;
			memset(targetP, 0, sizeof(application_data_t));
			targetP->instanceId = i;
			targetP->state      = IDLE;
			targetP->result     = INIT;
			
			strcpy(targetP->role,"End Device");
			strcpy(targetP->device_name,name);
			strcpy(targetP->container_supported,"Yes");
			
			
			strcpy(targetP->download_cmd, "\0");
			//strcpy(targetP->package_url, NULL);
			
			
			strcpy(targetP->pid, "none");
			targetP->duration   = 0.0;
			containerObj->instanceList = LWM2M_LIST_ADD(containerObj->instanceList, targetP);
		}
		
		
		lwm2m_list_t * instanceP;

		for (instanceP = containerObj->instanceList; instanceP != NULL ; instanceP = instanceP->next)
		{
			fprintf(stdout, "the instance is created as /%d/%d, \n", containerObj, instanceP->id);
		}
		
		printf("the endpoint name is %s\n", name);
		printf("the broker number is %d\n", brokerNo);
		 
        containerObj->readFunc    = prv_software_read;
        containerObj->writeFunc   = prv_software_write;
        containerObj->executeFunc = prv_software_execute;
        //containerObj->userData    = lwm2m_malloc(sizeof(application_data_t));
		printf("object is set\n");
        /*
         * Also some user data can be stored in the object with a private structure containing the needed variables
         */
        
    }
	printf("return established object\n");
    return containerObj;
}


/*void display_software_object(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    application_data_t * data = (application_data_t *)object->instanceList;
    fprintf(stdout, "  /%u: Software object:\r\n", object->objID);
    if (NULL != data)
    {
        fprintf(stdout, "    state: %u, supported: %s, result: %u\r\n",
                data->state, data->supported?"true":"false", data->result);
    }
#endif
}*/


void display_software_object(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    fprintf(stdout, "  /%u: Test object, instances:\r\n", object->objID);
    application_data_t * instance = (application_data_t *)object->instanceList;
    while (instance != NULL)
    {
        fprintf(stdout, "    /%u/%u: shortId: %u, ",
                object->objID, instance->instanceId,
                instance->instanceId);
        instance = (application_data_t *)instance->next;
    }
#endif
}


/*void free_object_software(lwm2m_object_t * objectP)
{
    if (NULL != objectP->userData)
    {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    if (NULL != objectP->instanceList)
    {
        lwm2m_free(objectP->instanceList);
        objectP->instanceList = NULL;
    }
    lwm2m_free(objectP);
	free(softwareInstance);
}*/

void free_object_software(lwm2m_object_t * object)
{
    LWM2M_LIST_FREE(object->instanceList);
    if (object->userData != NULL)
    {
        lwm2m_free(object->userData);
        object->userData = NULL;
    }
    lwm2m_free(object);
}



static uint8_t prv_software_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	
    if (instanceId > MAX_NUM_INSTANCE)
    {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
		*dataArrayP = lwm2m_data_new(12);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = 12;
        (*dataArrayP)[0].id = RES_O_DEVICE_ROLE;
        (*dataArrayP)[1].id = RES_O_DEVICE_NAME;
        (*dataArrayP)[2].id = RES_O_CONTAINER_SUPPORTED;
		(*dataArrayP)[3].id = RES_O_PKG_NAME;
		(*dataArrayP)[4].id = RES_O_PKG_STATUS;
		(*dataArrayP)[5].id = RES_O_STATUS;
		//(*dataArrayP)[6].id = RES_O_TRIGGER;
		(*dataArrayP)[6].id = RES_M_PID;
		//(*dataArrayP)[8].id = RES_M_KILL;
		(*dataArrayP)[7].id = RES_M_DOWNLOAD_CMD;
		(*dataArrayP)[8].id = RES_M_PACKAGE_URI;
		//(*dataArrayP)[11].id = RES_M_UPDATE;
		(*dataArrayP)[9].id = RES_M_STATE;
		(*dataArrayP)[10].id = RES_M_UPDATE_RESULT;
		(*dataArrayP)[11].id = RES_M_DURATION;
    }

    i = 0;
    do
    {
        switch ((*dataArrayP)[i].id)
        {	
		case RES_O_DEVICE_ROLE:
			lwm2m_data_encode_string(data->role, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
		case RES_O_DEVICE_NAME:
			lwm2m_data_encode_string(data->device_name, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
        case RES_O_CONTAINER_SUPPORTED:
			lwm2m_data_encode_string(data->container_supported, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
        case RES_O_PKG_NAME:
			lwm2m_data_encode_string(data->image_name, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;

        case RES_O_PKG_STATUS:
            lwm2m_data_encode_string(data->image_status, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
		case RES_O_TRIGGER:
            result = COAP_205_CONTENT;
            break;
			
		case RES_O_STATUS:
			lwm2m_data_encode_string(data->container_status, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
		case RES_M_PID:
			lwm2m_data_encode_string(data->pid, *dataArrayP + i);
            result = COAP_205_CONTENT;
			break;
		
		case RES_M_KILL:
			result = COAP_205_CONTENT;
			break;
		
        case RES_M_DOWNLOAD_CMD:
            lwm2m_data_encode_string(data->download_cmd, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
		case RES_M_PACKAGE_URI:
            lwm2m_data_encode_string(data->package_url, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;

        case RES_M_UPDATE:
            result = COAP_205_CONTENT;
			break;
		
		case RES_M_STATE:
            lwm2m_data_encode_int(data->state, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
		case RES_M_UPDATE_RESULT:
			lwm2m_data_encode_int(data->result, *dataArrayP + i);
            result = COAP_205_CONTENT;
			break;
		
		case RES_M_DURATION:
			lwm2m_data_encode_float(data->duration, *dataArrayP + i);
            result = COAP_205_CONTENT;
			break;
		
        default:
            result = COAP_404_NOT_FOUND;
        }

        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

static uint8_t prv_software_write(uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);

    if (instanceId > MAX_NUM_INSTANCE)
    {
        return COAP_404_NOT_FOUND;
    }

    i = 0;

    do
    {
        switch (dataArray[i].id)
        {
		case RES_O_PKG_NAME:
			printf("Write the image name\n");
			
			strncpy(data->image_name, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->image_name[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			is_image_installed(objectP,instanceId);
			break;
		
		case RES_O_PKG_STATUS:
			
			strncpy(data->image_status, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->image_status[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			break;
		
		case RES_O_STATUS:
			
			strncpy(data->container_status, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->container_status[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			break;
			
		case RES_M_PID:
			
			strncpy(data->pid, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->pid[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			break;
		 
		
        case RES_M_PACKAGE_URI:
            if (dataArray[i].value.asBuffer.length>=URL_MAX_LENGTH || dataArray[i].value.asBuffer.length<=0)
			{
				result = COAP_400_BAD_REQUEST;
			}
			else
			{
				//FD_SET(STDIN_FILENO, &writefds);
				if (common_context != NULL){
					fireResourceChanged(common_context, stateURI, &DOWNLOADING);
					fireResourceChanged(common_context, resultURI, &IDLE);
				}else{
					printf("common text is null\n");
				}
				
				strncpy(data->package_url, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
				data->package_url[dataArray[i].value.asBuffer.length] = 0x00;
				//data->state = 1;
				
				fprintf(stdout, "\n\t SOFTWARE DOWNLOADING\r\n\n");
				//////////////////////////////////////////////////////////////////
				exec_download(objectP,instanceId);
				//////////////////////////////////////////////////////////////////
				fprintf(stdout, "\n\t SOFTWARE DOWNLOADED\r\n\n");
				result = COAP_204_CHANGED;
				
			}
		
            break;
			
		case RES_M_STATE:
			
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->state)))
            {
                result = COAP_400_BAD_REQUEST;
            }
			result = COAP_204_CHANGED;
		
			break;
			
		case RES_M_UPDATE_RESULT:
		
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->result)))
            {
                result = COAP_400_BAD_REQUEST;
            }
			result = COAP_204_CHANGED;
			
			break;
			

        case RES_M_DOWNLOAD_CMD:
             if (dataArray[i].value.asBuffer.length>=CMD_MAX_LENGTH || dataArray[i].value.asBuffer.length<=0)
			{
				result = COAP_400_BAD_REQUEST;
			}
			else
			{
				strncpy(data->download_cmd, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
				data->download_cmd[dataArray[i].value.asBuffer.length] = 0x00;
				result = COAP_204_CHANGED;
			}
			
			break;

        default:
            result = COAP_405_METHOD_NOT_ALLOWED;
        }

        i++;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
}


static uint8_t prv_software_execute(uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP)
{
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
   
	
    int i = 0;
	int ret;
	
    // this is a single instance object
    if (instanceId > MAX_NUM_INSTANCE)
    {
        return COAP_404_NOT_FOUND;
    }

    if (length != 0) return COAP_400_BAD_REQUEST;

    // for execute callback, resId is always set.

	switch (resourceId)
	{
		
	case RES_O_TRIGGER:

		if (strcmp(data->image_status,image_on_board)==0)
		{
			printf("\t\n Triggering Image \n\t");
			ret = container_trigger(objectP,instanceId);
			if (ret!=0)
				return COAP_400_BAD_REQUEST;
			
			
		}else{
			// firmware update already running
			printf("Empty Image\n");
			return COAP_400_BAD_REQUEST;
		}
		
		break;
	
	case RES_M_UPDATE:

		if (data->state == DOWNLOADED)
		{
			fireResourceChanged(common_context, stateURI, &UPDATING);
			fprintf(stdout, "\n\t SOFTWARE UPDATE\r\n\n");
			ret = exec_run(objectP,instanceId);
		}else{
			// firmware update already running
			printf("really a bad request\n");
			return COAP_400_BAD_REQUEST;
		}
		
		break;
	
	case RES_M_KILL:
		
		fprintf(stdout, "\n\t Stopping Running Process\r\n\n");
		
		if (data->pid == "none"){
			fprintf(stdout, "\n\t The process is not running");
			return COAP_400_BAD_REQUEST;
		}
		else{
			int ret;
			ret = container_stop(objectP,instanceId);
			if(ret == -1)
				return COAP_400_BAD_REQUEST;
		}
		
		break;
		
	default:
		return COAP_405_METHOD_NOT_ALLOWED;
	}
	return COAP_204_CHANGED;
	
}




/*****************************/
/****Stop Container***********/
/*****************************/
// compose a command to stop the running container
int container_stop(lwm2m_object_t * objectP, uint16_t instanceId){
	
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	//application_data_t * data = (application_data_t*)(objectP->userData);
	char *exec_cmd;
	int ret;
	int cmdlen = strlen(data->pid)+64;
	if (strcmp(data->pid,NONE)==0){
		return -1;
	}
	exec_cmd = malloc(cmdlen);
	snprintf(exec_cmd, cmdlen, "docker stop %s", data->pid);
	return thread_creation(data,exec_cmd, stop_thread);
}

// thread function of stop process
void *stop_thread(void *arg){

	app_control_data_t *obj = (app_control_data_t*) arg;
	
	int ret = system(obj->cmd);
	if (ret == 0){
		//obj->appObject->container_status = IDLE;
		//obj->appObject->result = INIT;
		strcpy(obj->appObject->pid,"none");
		printf("the object is successfully stoped");
		fireResourceChanged(common_context, statusURI, EXITED);
		fireResourceChanged(common_context, pidURI, NONE);
		//return 1;
	}else{
		//obj->appObject->state = UNSUPPORTED_PACKAGE_TYPE;
		//obj->appObject->result = 0;
		fireResourceChanged(common_context, resultURI, &system_running_error);
		printf("failures happen during stopping the process");
		//return 0;
	}
	free(obj->cmd);
	free(obj);
	return NULL;
}


/**************************************/
/****downloading relevant functions****/
/**************************************/

// compose a whole command to download 
int exec_download(lwm2m_object_t * objectP,uint16_t instanceId) {
	char *exec_cmd;
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	//application_data_t * data = (application_data_t*)(objectP->userData);
	
	/*if (data->download_cmd == NULL){
		return COAP_500_INTERNAL_SERVER_ERROR;
	}*/
	char *download_cmd ="docker pull";
	printf("Downloading from %s\n", data->package_url);
	
	int cmdlen = strlen(download_cmd)+strlen(data->package_url)+64;
	exec_cmd = malloc(cmdlen);
	snprintf(exec_cmd, cmdlen, "%s %s", download_cmd,data->package_url);
	data->state = DOWNLOADING;
	data->result = INIT;
	//return download_cmd(data,exec_cmd); 
	return thread_creation(data,exec_cmd, download_thread);
}


// details in download thread
void *download_thread(void *arg) {
	time(&start_t);
	app_control_data_t *obj = (app_control_data_t*) arg;
	int ret;
	printf("Downloading cmd: %s\n", obj->cmd);
	ret = system(obj->cmd);
	//obj->appObject->result = (ret >> 8);
	printf("return value of download is %d\n", ret);
	switch((ret >> 8)){
		case -1: 
			obj->appObject->state = ERROR;
			break;
		case 0:
			obj->appObject->state = DOWNLOADED;
			fireResourceChanged(common_context, stateURI, &DOWNLOADED);
			fireResourceChanged(common_context, istatusURI, image_on_board);
			
			fprintf(stdout, "\n\t DOWNLOAD SUCCESS, FETCHING PACKAGE NAME & VERSION\r\n\n");
			printf("the package url to be parsed is %s\n",obj->appObject->package_url);
			retrive_package_info(obj->appObject->package_url,obj->appObject->name,obj->appObject->version);
			break;
		case NO_SPACE:
			obj->appObject->result = NO_SPACE;
			break;
		case CONNECT_LOSS:
			obj->appObject->result = CONNECT_LOSS;
			break;
		case INTEGRITY_FALIURE:
			obj->appObject->result = INTEGRITY_FALIURE;
			break;
		case UNSUPPORTED_PACKAGE_TYPE:
			obj->appObject->result = UNSUPPORTED_PACKAGE_TYPE;
			break;
		case 1:
			obj->appObject->result = INVALID_URI;
			break;
		case UPDATE_FAILURE:
			obj->appObject->result = UPDATE_FAILURE;	
			break;
		case UNSUPPORTED_PROTOCOL:
			obj->appObject->result = UNSUPPORTED_PROTOCOL;
			break;
	}
	time(&end_t);
	obj->appObject->duration = difftime(end_t, start_t);
	free(obj->cmd);
	free(obj);
	return NULL;
}


/*******************************/
/****update related function****/
/*******************************/

//compose a execute command
int exec_run(lwm2m_object_t * objectP,uint16_t instanceId) {
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	//application_data_t * data = (application_data_t*)(objectP->userData);
	char *cmd;
	int cmdLen; 
	char pid[1024];
	int ret;

	printf("Running %s\n", data->package_url);
	//path = get_current_dir_name();
	
	
	if (strlen(data->download_cmd) == 0){
		
		cmdLen = strlen(data->package_url) + 512;
		cmd = malloc(cmdLen);
		if (strlen(data->package_url) != 0) {
		printf("docker run -it -d %s\n",data->package_url);
		
		snprintf(cmd, cmdLen, "docker run -it -d --privileged %s",data->package_url);
		//ret = exec_run_fork(cmd,data->pid);
		ret = thread_creation(data, cmd, exec_thread);
		//free(cmd);
		return 0;
		
		} else {
		return COAP_500_INTERNAL_SERVER_ERROR;
		}
	}
	else{
		char *exec_cmd = data->download_cmd;
		ret = thread_creation(data, cmd, exec_thread);
		return 0;
	} 
	

}	

//details in the execute thread
void *exec_thread(void *arg){
	
	app_control_data_t *obj = (app_control_data_t*) arg;
	char *cmd = (char *)obj->cmd;
	char *proc_pid = (char *)obj->appObject->pid;
	
	printf("Running command : %s\n", cmd);
	int i = 0;
	int a = 0;
	FILE *fp;
	char tmp_pid[128];
	char full_pid[128];
	char short_pid[128];
	int size;
	memset(full_pid,0,strlen(full_pid));
	/* Open the update command for reading. */
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		strcpy(proc_pid , "none"); 
		//return 0;
	}else{
		printf("read pipe\n");
		while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
			
			strcat(full_pid, tmp_pid);
			printf("%s",tmp_pid);
			
		}
		printf("the full process id is %s\n",full_pid);
		//*(full_pid + a) = 0;
		strncpy(short_pid,full_pid,12);
		*(short_pid + 12) = 0;
		printf("the short process id is %s\n",short_pid);
		 /* close */
		pclose(fp);
		int ret = exec_check(short_pid, proc_pid,obj->appObject->image_name);
		if(ret == 0){
			obj->appObject->state = IDLE;
			obj->appObject->result = UPDATE_SUCCESS;
			fireResourceChanged(common_context, stateURI, &IDLE);
			fireResourceChanged(common_context, resultURI, &update_success);
			fireResourceChanged(common_context, pidURI, short_pid);
			fireResourceChanged(common_context, statusURI, RUNNING);
			
			
		}else{
			obj->appObject->state = IDLE;
			obj->appObject->result = UPDATE_FAILURE;
			fireResourceChanged(common_context, stateURI, &IDLE);
			fireResourceChanged(common_context, resultURI, &update_failure);
			fireResourceChanged(common_context, pidURI, NONE);
			fireResourceChanged(common_context, statusURI, EXITED);
			//strcpy(data->pid, "none");
		} 
	}
	free(obj->cmd);
	free(obj);
	return NULL;
	//return ret;
}

// exec check after executing 
int exec_check(char* short_pid,char* proc_pid, char* image_name){
	
	char check_pid[128]={};
	char final_pid[128]={};
	char tmp_pid[128]={};
	char check_cmd[] = "docker ps --filter ancestor=";
	strcat(check_cmd,image_name);
	strcat(check_cmd," --filter status=running -lq");
	int i;
	int a = 0;
	FILE *fp;
	int ret=0;
	//memset(check_pid,0,strlen(check_pid));
	/* Open the check command for reading. */
	printf("Check command : %s\n", check_cmd);
	
	fp = popen(check_cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		strcpy(proc_pid , "none"); 
		ret = 0;
	}else{
		
		while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
			strcat(check_pid, tmp_pid);
			printf("%s",tmp_pid);
		}
		
		if (strlen(check_pid)<=0){
			ret = 1;
		}
		
		
		
		/*strncpy(final_pid,check_pid,12);
		*(final_pid + 12) = 0;
		
		
		
		
		printf("the running process is %s\n",short_pid);
		printf("the check process id is %s\n",final_pid);
		
		
		if(strcmp(short_pid,final_pid) == 0){
			printf("the process is successfully running\n");
			strcpy(proc_pid,short_pid);
			//return 1;
		}else{
			printf("errors happen during execution\n");
			strcpy(proc_pid,"none");
			ret = 1;
		}
		*/
		//strcpy(pid, execv(cmd));
		//check the 
		printf("the pid of proceeding container is %s\n",proc_pid);
		
	}
	
	
	 /* close */
	pclose(fp);
	return ret;
}


/*assistance functions*/

/*retrieve package name & version*/
void retrive_package_info (char *source_url,char *name, char* version){
	char *token;
	
	//char *package_url = (char *)malloc(sizeof(source_url));
	char package_url[1024];
	strcpy(package_url,source_url);
	printf("!!pakcage url %s will be splited!!\n",package_url);
	int ret = url_integrity_check(package_url,':');
	
	if (ret == 0){
		printf("the version of this pakcage is latest\n");
		strcpy(name, package_url);
		strcpy(version, "latest");
	}else{
		
		// token point to image name
		token = strtok(package_url, ":");
		strcpy(name, token);
		printf("package name=[%s]\n", name);
		// token point to image tag
		token = strtok(NULL, ":");
		strcpy(version, token);
		printf("package version=[%s]\n", version);
		
	}
	
	//free(package_url);
}


int url_integrity_check(char* a_str, const char a_delim){
	/*check if the string contains a element*/
	char* pPosition = strchr(a_str,a_delim);
	if (pPosition == NULL){
		return 0;
	}
	return 1;
}


int thread_creation(application_data_t *data, char *cmd, void* thread_func){
	int ret;
	pthread_t thread;
	app_control_data_t *controlObject = malloc(sizeof(app_control_data_t));
	if (controlObject != NULL){
		controlObject->cmd = cmd;
		controlObject->appObject = data;
		ret = pthread_create(&thread, NULL, thread_func, controlObject);
		if (ret != -1){
			pthread_detach(thread);
		}
		else{
			free(controlObject);
		}
	}else{
		ret = -1;
	}
	
	return ret;
}

void fireResourceChanged(lwm2m_context_t * context, char *url_name, void* resource_value)
{
        int valueLength;
		char value[15];
        lwm2m_uri_t uri;
		printf("length is %d\n",strlen(url_name));
        if (lwm2m_stringToUri(url_name,strlen(url_name), &uri))
        {
			valueLength = sprintf(value, "%s", resource_value);
            handle_value_changed(context, &uri, value, valueLength);
        }
    
}


/**********************************************************/
/********image installation check**************************/
int is_image_installed(lwm2m_object_t * objectP,uint16_t instanceId) {
	
	printf("Check if the image is installed?\n");
	
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	if (data->image_name==NULL){
		return -1;
	}
	char check_cmd[256]; 
	strcpy(check_cmd,"docker images ");
	strcat(check_cmd,data->image_name);
	strcat(check_cmd," -q");

	return thread_creation(data,check_cmd,image_check_thread);
}


int image_check_thread(void *arg){
	
	app_control_data_t *obj = (app_control_data_t*) arg;

	char output[1024]={};
	char tmp[128];
	FILE* fp;
	int ret=0;
	printf("Image Check : %s\n", obj->cmd);
	
	fp = popen(obj->cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" ); 
		ret = -1;
	}else{
		while (fgets(tmp, sizeof(tmp)-1, fp) != NULL) {
			strcat(output, tmp);
			printf("%s",output);
		}
		
		printf("the length of the output is %d\n",strlen(output));
	
		if (strlen(output) > 0){
			strcpy(obj->appObject->image_status,image_on_board);
			printf("%s\n",obj->appObject->image_status);
			fireResourceChanged(common_context, istatusURI, image_on_board);
			//check if it is activated
			char check_cmd[1024];
			//snprintf(check_cmd,1024,"docker ps --filter ancestor=%s --filter status=running --filter status=paused -lq",obj->appObject->image_name);
			strcpy(obj->cmd,check_cmd);
			ret = image_activ_check_thread(obj);
		
		}else{
			strcpy(obj->appObject->image_status,image_off_board);
			fireResourceChanged(common_context, istatusURI, image_off_board);
			fireResourceChanged(common_context, statusURI, EXITED);
			fireResourceChanged(common_context, pidURI, NONE);
			ret = 1;
		}
		
	}
	
	pclose(fp);
	return ret;
	
	
	
}
/*****************************************************/
/************image running check**********************/
int is_image_activated(lwm2m_object_t * objectP,uint16_t instanceId) {
	
	printf("Check if the image is run as a container?\n");
	
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	
	if (data->image_name==NULL){
		return 1;
	}
	char check_cmd[1024];
	//strcat(check_cmd,data->image_name);
	//strcat(check_cmd," --filter status=running -lq");

	return thread_creation(data,check_cmd,image_activ_check_thread);
	
}

int image_activ_check_thread(void *arg){
	int ret;
	app_control_data_t *obj = (app_control_data_t*) arg;
	ret = running_check(obj);
	if(ret == 2){
		paused_check(obj);
	}
	
}

int running_check(app_control_data_t *obj){
	char check_pid[128]={};
	char final_pid[128];
	char tmp_pid[128];
	int ret=0;
	
	FILE *fp;
	/* Open the check command for reading. */
	snprintf(obj->cmd,1024,"docker ps --filter ancestor=%s --filter status=running -lq",obj->appObject->image_name);
	printf("RUNNING : %s\n", obj->cmd);
	
	fp = popen(obj->cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		strcpy(obj->appObject->pid,NONE);
		fireResourceChanged(common_context, pidURI, NONE);
		ret = 1;
	}else{
		while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
		
			strcat(check_pid, tmp_pid);
			printf("%s",tmp_pid);
		}
	
		if(strlen(check_pid)==0){
			printf("No running container, checking paused containers...\n");
			ret = 2;
		}
		else{
			strncpy(final_pid,check_pid,12);
			*(final_pid + 12) = 0;
			strcpy(obj->appObject->container_status,RUNNING);
			fireResourceChanged(common_context, statusURI, RUNNING);
			strcpy(obj->appObject->pid,final_pid);
			fireResourceChanged(common_context, pidURI, final_pid);
			printf("the running pid is %s\n",check_pid);
			ret = 0;
		}
		
	}
	pclose(fp);
	return ret;
}

int paused_check(app_control_data_t *obj){
	char check_pid[128]={};
	char final_pid[128];
	char tmp_pid[128];
	int ret=0;
	
	FILE *fp;
	/* Open the check command for reading. */
	
	snprintf(obj->cmd,1024,"docker ps --filter ancestor=%s --filter status=paused -lq",obj->appObject->image_name);
	printf("PAUSED CHECK : %s\n", obj->cmd);
	fp = popen(obj->cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		strcpy(obj->appObject->pid,NONE);
		fireResourceChanged(common_context, pidURI, NONE);
		ret = 1;
	}else{
		while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
		
			strcat(check_pid, tmp_pid);
			printf("%s",tmp_pid);
		}
		
		if(strlen(check_pid)==0){
			printf("There is no container paused now");
			strcpy(obj->appObject->container_status,EXITED);
			fireResourceChanged(common_context, statusURI, EXITED);
			strcpy(obj->appObject->pid,NONE);
			fireResourceChanged(common_context, pidURI, NONE);
			ret = 2;
			
		}
		else{
			strncpy(final_pid,check_pid,12);
			*(final_pid + 12) = 0;
			
			strcpy(obj->appObject->container_status,PAUSED);
			fireResourceChanged(common_context, statusURI, PAUSED);
			strcpy(obj->appObject->pid,final_pid);
			fireResourceChanged(common_context, pidURI, final_pid);
			printf("the paused pid is %s\n",check_pid);
			ret = 0;
		}
		
	}
	pclose(fp);
	return ret;
}		
	


/*********************************************************/
/************Container Trigger On****************************/
int container_trigger(lwm2m_object_t * objectP,uint16_t instanceId) {
	
	char check_cmd[100];
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	if (strlen(data->image_name)==0){
		return -1;
	}
	
	printf("The status of the container at present : %s and %s\n",data->container_status,data->pid);
	if (strcmp(data->container_status,EXITED)==0 && strcmp(data->pid,NONE)==0){
		
		printf("No container of this image is running\n");
		
		if (strlen(data->download_cmd)!=0){
			strcpy(check_cmd,data->download_cmd);
		}else{
			strcpy(check_cmd,"docker run -it -d --privileged "); 
			strcat(check_cmd,data->image_name);
		}
		
		printf("running command : %s\n", check_cmd);
	}else if(strcmp(data->container_status,PAUSED)==0 && strcmp(data->pid,NONE)!=0){
		
		printf("The container of this image is paused\n");
		strcpy(check_cmd,"docker unpause "); 
		strcat(check_cmd,data->pid);
		printf("unpause command : %s\n", check_cmd);
		
	}else if(strcmp(data->container_status,RUNNING)==0 && strcmp(data->pid,NONE)!=0){
		
		printf("The container of this image is running\n");
		strcpy(check_cmd,"docker pause "); 
		strcat(check_cmd,data->pid);
		printf("pause command : %s\n", check_cmd);
		
	}else{
		printf("Errors happen when run the container");
		return -1;
	}
	
	return thread_creation(data,check_cmd,trigger_thread);
}



int trigger_thread(void *arg){
	
	app_control_data_t *obj = (app_control_data_t*) arg;
	char *proc_pid = (char *)obj->appObject->pid;
	char *image_name = (char *)obj->appObject->pid;
	char *container_status = (char *)obj->appObject->container_status;
	int ret;
	FILE *fp;
	
	if (strcmp(container_status,EXITED)==0 && strcmp(proc_pid,NONE)==0){
		
		printf("start process\n");
		
		char check_pid[128]={};
		char short_pid[128]={};
		char tmp_pid[128]={};
		
		
		/* Open the check command for reading. */
		
		
		fp = popen(obj->cmd, "r");
		if (fp == NULL) {
			printf("Failed to run command\n" );
			strcpy(obj->appObject->pid,NONE);
			fireResourceChanged(common_context, pidURI, NONE);
			ret = 1;
		}else{
			while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
				strcat(check_pid, tmp_pid);
				printf("%s",tmp_pid);
			}
			
			printf("the check pid is %s\n",check_pid);
		
			if(strlen(check_pid)==0){
				
				strcpy(obj->appObject->container_status,EXITED);
				fireResourceChanged(common_context, statusURI, EXITED);
				strcpy(obj->appObject->pid,NONE);
				fireResourceChanged(common_context, pidURI, NONE);
				 
				ret = 1;
			}
			else{
				fprintf(stdout, "\n\t START CONTAINER \r\n\n");
				strncpy(short_pid,check_pid,12);
				*(short_pid + 12) = 0;
				printf("the check process id is %s\n",short_pid);
				
				ret = trigger_check(short_pid,proc_pid);

			}	
		}
		pclose(fp);
	}else if(strcmp(container_status,PAUSED)==0 && strcmp(proc_pid,NONE)!=0){
		
		int ret = system(obj->cmd);
		if(ret==0){
			fprintf(stdout, "\n\t UNPAULSED CONTAINER \r\n\n");
			strcpy(obj->appObject->container_status,RUNNING);
			fireResourceChanged(common_context, statusURI, RUNNING);
			
		}
		
	}else if(strcmp(container_status,RUNNING)==0){
		
		int ret = system(obj->cmd);
		if(ret==0){
			fprintf(stdout, "\n\t STOP CONTAINER \r\n\n");
			strcpy(obj->appObject->container_status,PAUSED);
			fireResourceChanged(common_context, statusURI, PAUSED);
			
		}
	}else{
		printf("bad request\n");
		ret = 1;
	}
	return ret;
}

int trigger_check(char* short_pid,char* proc_pid){
	
	char check_pid[128]={};
	char final_pid[128]={};
	char tmp_pid[128]={};
	char check_cmd[] = "docker ps --filter id=";
	strcat(check_cmd,short_pid);
	strcat(check_cmd," -q");
	
	int i;
	int a = 0;
	FILE *fp;
	int ret=1;
	//memset(check_pid,0,strlen(check_pid));
	/* Open the check command for reading. */
	printf("Check command : %s\n", check_cmd);
	
	fp = popen(check_cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		strcpy(proc_pid , "none"); 
		ret=1;
	}else{
		while (fgets(tmp_pid, sizeof(tmp_pid)-1, fp) != NULL) {
			strcat(check_pid, tmp_pid);
			printf("%s",tmp_pid);
		}
		
		if(strlen(check_pid)==0){
			fireResourceChanged(common_context, statusURI, EXITED);
			fireResourceChanged(common_context, pidURI, NONE);
			ret = 1;
		}
		else{
			fireResourceChanged(common_context, statusURI, RUNNING);
			strcpy(proc_pid,short_pid);
			fireResourceChanged(common_context, pidURI, proc_pid);
			ret = 0;
		}
		
		printf("the pid of proceeding container is %s\n",proc_pid);
	}
	
	 /* close */
	pclose(fp);
	return ret;
}


