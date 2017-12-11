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
 *    Julien Vermillard - dinitial implementation
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
#include <libmongoc-1.0/mongoc.h>
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
#define RES_O_DEVICE_NO                 0
#define RES_O_DEVICE_ROLE               1
#define RES_O_DEVICE_NAME               2  //RW String
#define RES_O_CONTAINER_SUPPORTED       3  //RW String
#define RES_O_PKG_NAME                  4  //RW String
#define RES_O_PKG_STATUS                5  //RW String
#define RES_M_DOWNLOAD_CMD  			6  
#define RES_O_TRIGGER					7  //E  Opaque
#define RES_O_STATUS					8  //R  String
#define RES_M_PID                       9  //R  String
#define RES_M_KILL						10  //E  Opaque  
//RW String
#define RES_M_PACKAGE_URI               11  //RW String
#define RES_M_UPDATE                    12 //E  Opaque
#define RES_M_STATE                     13 //RW Integer
#define RES_M_UPDATE_RESULT             14 //RW Integer
#define RES_M_DURATION                  15 //RW Float

#define NUM_INSTANCE                    2
#define MAX_NUM_INSTANCE                8



char noURL[9]="/1026/0/0";
char droleURL[9]="/1026/0/1";
char ddeviceURI[9]="/1026/0/2";
char dsupportURI[9]="/1026/0/3";
char dinameURI[9]="/1026/0/4";
char distatusURI[9]="/1026/0/5";
char dcmdURI[9]="/1026/0/6";
char dtriggerURI[9]="/1026/0/7";
char dstatusURI[9]="/1026/0/8";
char dpidURI[9]="/1026/0/9";
char dkillURI[10]="/1026/0/10";

char durlURI[10]="/1026/0/11";
char dupdateURI[10]="/1026/0/12";
char dstateURI[10]="/1026/0/13";
char dresultURI[10]="/1026/0/14";
char ddurationURI[10]="/1026/0/15";

char *dimage_on_board="on board";
char *dimage_off_board="off board";

int dIDLE=0;
int dDOWNLOADING=1;
int dDOWNLOADED=2;
int dUPDATING=3;

int dinit=INIT;
int dupdate_success=UPDATE_SUCCESS;
int dupdate_failure=UPDATE_FAILURE;
int dsystem_running_error=SYSTEM_RUNNING_ERROR;
char dRUNNING[10]="running";
char dPAUSED[10]="paused";
char dEXITED[10]="exited";



//char OFF[10] = "OFF";
//char ON[10] = "ON";
char dNONE[1024]= "none";
char db_addr[1024];
char deviceName[32];

char db_name[32];
char collection_name[32];


//lwm2m_context_t * common_context;

static uint8_t prv_device_update_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP);
static uint8_t prv_device_update_write(uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP);
static uint8_t prv_device_update_execute(uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP);
	

lwm2m_object_t * get_object_device_update(int brokerNo, char *dbName, char* name,lwm2m_context_t * context, char *db_url)
{
    /*
     * The get_object_firmware function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * containerObj;

    containerObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
	snprintf(db_name,32,dbName);
	snprintf(db_addr,1024,db_url);
	snprintf(deviceName,32,name);
	snprintf(collection_name,32,name);
	
	char find_uri[1024] = {};
	char insert_uri[1024] = {};
	char update_uri[1024] = {};
	char delete_uri[1024] = {};
	char count_uri[1024] = {};
	
	
    if (NULL != containerObj)
    {
		int i;
		application_data_t* targetP;
		
        memset(containerObj, 0, sizeof(lwm2m_object_t));

        /*objectID : 1025*/
        containerObj->objID = DEVICE_UPDATE_OBJECT_ID;
       
		for (i=0 ; i < NUM_INSTANCE ; i++){
			targetP = (application_data_t*)lwm2m_malloc(sizeof(application_data_t));
			if (NULL == targetP) return NULL;
			memset(targetP, 0, sizeof(application_data_t));
			targetP->instanceId = i;
			targetP->device_no  = i;
			targetP->state      = dIDLE;
			targetP->result     = INIT;
			
			strcpy(targetP->role,"End Device");
			strcpy(targetP->container_supported,"Yes");
			strcpy(targetP->container_status, "exited");
			strcpy(targetP->pid, "none");
			targetP->duration   = 0.0;
			containerObj->instanceList = LWM2M_LIST_ADD(containerObj->instanceList, targetP);
			
			
			//initialize database
			fprintf(stdout,"connect to the %s\n",db_url);
			if (strlen(db_url)!=0){
				mongoc_collection_t *collection;
				int mongo_ret = db_connection(db_url,db_name,collection_name,collection);
				if (mongo_ret==0){
					
					fprintf(stdout,"successfuly connect to the database\n");
		
					//query a document
					bson_t * query = bson_new ();
					BSON_APPEND_INT32 (query, DEVICE_NO, targetP->device_no);	
					
					snprintf(find_uri,1024,"%s/?appname=find-example",db_url);
					fprintf(stdout,"query request : %s\n",find_uri);
					int query_ret=doc_query(find_uri,db_name,collection_name, query);
					
					bson_t * doc;
					if (query_ret == 0){
						fprintf(stdout,"successfuly find the query list\n");
						
						//Update
						query = bson_new ();
						BSON_APPEND_INT32 (query, DEVICE_NO, targetP->device_no);	
						bson_t * update = BCON_NEW ("$set",
									  "{",
									  DEVICE_NO,
									  BCON_INT32 (targetP->device_no),
									  DEVICE_ROLE,
									  BCON_UTF8 ("End Device"),
									  DEVICE_NAME,
									  BCON_UTF8 (""),
									  CONTAINER_SUPPORTED,
									  BCON_UTF8 ("yes"),
									  PKG_NAME,
									  BCON_UTF8 (""),
									  PKG_STATUS,
									  BCON_UTF8 (""),
									  DOWNLOAD_CMD,
									  BCON_UTF8 (""),
									  
									  STATUS,
									  BCON_UTF8 (dEXITED),
									  PID,
									  BCON_UTF8 (dNONE),

									  PACKAGE_URI,
									  BCON_UTF8 (""),
									  
									  STATE,
									  BCON_INT32 (0),
									  UPDATE_RESULT,
									  BCON_INT32 (0),
									  DURATION,
									  BCON_DOUBLE (0.0),
									  
									  "}");
						//update a document 
									  
						snprintf(update_uri,1024,"%s/?appname=update-example",db_url);
						fprintf(stdout,"update request : %s\n",update_uri);
									  
						int update_ret = doc_update(update_uri, db_name,collection_name,query, update);
						
						if (update_ret == 0){
							fprintf(stdout,"update document successful\n");
						}else{
							fprintf(stdout,"update document failed\n");
						}
					}else{
						
						//bson_t* document = BCON_NEW ();
						doc = bson_new ();
						BSON_APPEND_INT32(doc,DEVICE_NO,targetP->device_no);
						BSON_APPEND_INT32 (doc, OBJECT_ID,1025);
						BSON_APPEND_INT32 (doc, INSTANCE_ID,0);
						BSON_APPEND_UTF8 (doc, DEVICE_ROLE,"End Device");
						BSON_APPEND_UTF8 (doc, DEVICE_NAME,"");
						BSON_APPEND_UTF8 (doc, CONTAINER_SUPPORTED,"yes");
						BSON_APPEND_UTF8 (doc, PKG_NAME,"");
						BSON_APPEND_UTF8 (doc, PKG_STATUS,"");
						BSON_APPEND_UTF8 (doc, DOWNLOAD_CMD,"");
						BSON_APPEND_UTF8 (doc, STATUS,dEXITED);
						BSON_APPEND_UTF8 (doc, PID,dNONE);
						BSON_APPEND_UTF8 (doc, PACKAGE_URI,"");
						BSON_APPEND_INT32(doc,STATE,0);
						BSON_APPEND_INT32(doc,UPDATE_RESULT,0);
						BSON_APPEND_DOUBLE(doc,DURATION,0.0);
						
						printf("insert a new document\n");
						snprintf(insert_uri,1024,"%s/?appname=insert-example",db_url);
						fprintf(stdout,"update request : %s\n",insert_uri);
						doc_insert(insert_uri,db_name,collection_name, doc);
					}
				}
			}
			
			
			
		}
		
		
		lwm2m_list_t * instanceP;

		for (instanceP = containerObj->instanceList; instanceP != NULL ; instanceP = instanceP->next)
		{
			fprintf(stdout, "the instance is created as /%d/%d, \n", containerObj, instanceP->id);
		}
		
		printf("the endpoint name is %s\n", name);
		printf("the broker number is %d\n", brokerNo);
		 
        containerObj->readFunc    = prv_device_update_read;
        containerObj->writeFunc   = prv_device_update_write;
        containerObj->executeFunc = prv_device_update_execute;
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


void display_object_device_update(lwm2m_object_t * object)
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

void free_object_device_update(lwm2m_object_t * object)
{
    LWM2M_LIST_FREE(object->instanceList);
    if (object->userData != NULL)
    {
        lwm2m_free(object->userData);
        object->userData = NULL;
    }
    lwm2m_free(object);
}



static uint8_t prv_device_update_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	
	//initialize database
	fprintf(stdout,"connect to the %s\n",db_addr);
	mongoc_collection_t *collection;
	//int mongo_ret = db_connection(db_addr,db_name,collection_name,collection);
	int mongo_ret = 1;
	bson_t * query;
	
	
    if (instanceId > MAX_NUM_INSTANCE)
    {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
		*dataArrayP = lwm2m_data_new(13);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = 13;
     
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
		(*dataArrayP)[12].id = RES_O_DEVICE_NO;
    }

    i = 0;
    do
    {
        switch ((*dataArrayP)[i].id)
        {	
		
		case RES_O_DEVICE_NO:
			
			lwm2m_data_encode_int(data->device_no, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
		
		case RES_O_DEVICE_ROLE:
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->role,doc_read_str(db_addr,db_name,collection_name,query,DEVICE_ROLE,data->role));
			}
			
			lwm2m_data_encode_string(data->role, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
		case RES_O_DEVICE_NAME:
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->device_name,doc_read_str(db_addr,db_name,collection_name,query,DEVICE_NAME,data->device_name));
			}
		
			lwm2m_data_encode_string(data->device_name, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
        case RES_O_CONTAINER_SUPPORTED:
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->container_supported,doc_read_str(db_addr,db_name,collection_name,query,CONTAINER_SUPPORTED,data->container_supported));
			}
			
			lwm2m_data_encode_string(data->container_supported, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
        case RES_O_PKG_NAME:
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->image_name,doc_read_str(db_addr,db_name,collection_name,query,PKG_NAME,data->image_name));
			}
			
			lwm2m_data_encode_string(data->image_name, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;

        case RES_O_PKG_STATUS:
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->image_status,doc_read_str(db_addr,db_name,collection_name,query,PKG_STATUS,data->image_status));
			}
			
            lwm2m_data_encode_string(data->image_status, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
		case RES_O_TRIGGER:
            result = COAP_205_CONTENT;
            break;
			
		case RES_O_STATUS:
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->container_status,doc_read_str(db_addr,db_name,collection_name,query,STATUS,data->container_status));
			}
			
			lwm2m_data_encode_string(data->container_status, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
		case RES_M_PID:
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->pid,doc_read_str(db_addr,db_name,collection_name,query,PID,data->pid));
			}
			
			lwm2m_data_encode_string(data->pid, *dataArrayP + i);
            result = COAP_205_CONTENT;
			break;
		
		case RES_M_KILL:
		
			result = COAP_205_CONTENT;
			break;
		
        case RES_M_DOWNLOAD_CMD:
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->download_cmd,doc_read_str(db_addr,db_name,collection_name,query,DOWNLOAD_CMD,data->download_cmd));
			}
			
            lwm2m_data_encode_string(data->download_cmd, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
		case RES_M_PACKAGE_URI:
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				strcpy(data->package_url,doc_read_str(db_addr,db_name,collection_name,query,PACKAGE_URI,data->package_url));
			}
			
            lwm2m_data_encode_string(data->package_url, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;

        case RES_M_UPDATE:
            result = COAP_205_CONTENT;
			break;
		
		case RES_M_STATE:
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				data->state=doc_read_int(db_addr,db_name,collection_name,query,STATE,data->state);
			}
			
            lwm2m_data_encode_int(data->state, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
		case RES_M_UPDATE_RESULT:
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				data->result=doc_read_int(db_addr,db_name,collection_name,query,UPDATE_RESULT,data->result);
			}
			
			lwm2m_data_encode_int(data->result, *dataArrayP + i);
            result = COAP_205_CONTENT;
			break;
		
		case RES_M_DURATION:
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_INT32(query, DEVICE_NO, data->device_no);
				data->duration=doc_read_double(db_addr,db_name,collection_name,query,DURATION,data->duration);
			}
			
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

static uint8_t prv_device_update_write(uint16_t instanceId,
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
	
	//initialize database
	fprintf(stdout,"connect to the %s, database: %s, collection: %s\n",db_addr, db_name, collection_name); 
	mongoc_collection_t *collection;
	int mongo_ret = db_connection(db_addr,db_name,collection_name,collection);
	bson_t * query;
	bson_t * update;

    do
    {
        switch (dataArray[i].id)
        {
		case RES_O_DEVICE_NO:
		
		
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->device_no)))
            {
                result = COAP_400_BAD_REQUEST;
            }
			result = COAP_204_CHANGED;
			
			break;
			
		case RES_O_DEVICE_NAME:
			
			printf("Write the image role\n");
			
			strncpy(data->device_name, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->device_name[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
		
			break;
			
		case RES_O_DEVICE_ROLE:
		
			printf("Write the image name\n");
			
			strncpy(data->role, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->role[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
		
			break;
			 
		case RES_O_PKG_NAME:
		
			printf("write the image name\n");
			strncpy(data->image_name, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->image_name[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();
				
				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (dinameURI),
									VALUE, BCON_UTF8 (data->image_name),
									
								"}");
				
				
				int update_ret = doc_update(db_addr,db_name,collection_name,query,update);
				if (update_ret == 0){
					printf("update success\n");
				}else{
					printf("update failure\n");
				}
				
			}
			
	
			//is_image_installed(objectP,instanceId);
			break;
		
		case RES_O_PKG_STATUS:
		
			strncpy(data->image_status, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->image_status[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();
	
				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (distatusURI),
									VALUE, BCON_UTF8 (data->image_status),
									
								"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
			}
			
			break;
		
		case RES_O_STATUS:
			
			strncpy(data->container_status, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->container_status[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();
				
				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (dstatusURI),
									VALUE, BCON_UTF8 (data->container_status),
									
								"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
			}
			
			break;
			
		case RES_M_PID:
			
			strncpy(data->pid, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->pid[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();
				
				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (dpidURI),
									VALUE, BCON_UTF8 (data->pid),
								"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
			}
			
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
					fireResourceChanged(common_context, dstateURI, &dDOWNLOADING);
					fireResourceChanged(common_context, dresultURI, &dIDLE);
				}else{
					printf("common text is null\n");
				}
				
				strncpy(data->package_url, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
				data->package_url[dataArray[i].value.asBuffer.length] = 0x00;
				//data->state = 1;
				
				//fprintf(stdout, "\n\t SOFTWARE dDOWNLOADING\r\n\n");
				
				//exec_download(objectP,instanceId);
				
				if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();
				
				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (durlURI),
									VALUE, BCON_UTF8 (data->package_url),
								"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
				}
				
				fprintf(stdout, "\n\t SOFTWARE dDOWNLOADED\r\n\n");
				result = COAP_204_CHANGED;
				
			}
		
            break;
			
		case RES_M_STATE:
			
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->state)))
            {
                result = COAP_400_BAD_REQUEST;
            }
			result = COAP_204_CHANGED;
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();
				
				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (dstateURI),
									VALUE, BCON_INT32 (data->state),
								"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
			}
		
			break;
			
		case RES_M_UPDATE_RESULT:
		
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->result)))
            {
                result = COAP_400_BAD_REQUEST;
            }
			result = COAP_204_CHANGED;
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();
				
				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (dresultURI),
									VALUE, BCON_INT32 (data->result),
								"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
			}
			
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
				
				if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();

				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (WRITE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (dcmdURI),
									VALUE, BCON_UTF8 (data->download_cmd),
								"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
				}
			}
			
			break;

        default:
            result = COAP_405_METHOD_NOT_ALLOWED;
        }

        i++;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
}


static uint8_t prv_device_update_execute(uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP)
{
	application_data_t * data = (application_data_t*)lwm2m_list_find(objectP->instanceList, instanceId);
   
	
    int i = 0;
	int ret;
	
	//initialize database
	fprintf(stdout,"connect to the %s\n",db_addr);
	mongoc_collection_t *collection;
	int mongo_ret = db_connection(db_addr,db_name,collection_name,collection);
	bson_t * query;
	bson_t * update;
	
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

	
		printf("\t\n Triggering Image \n\t");
		//ret = container_trigger(objectP,instanceId);
		
		if (mongo_ret == 0){
			query = bson_new ();
			BSON_APPEND_UTF8(query, ROLE, "broker");
			update = bson_new ();

			
			update = BCON_NEW ("$set","{",
								REQUEST_FLAG, BCON_UTF8 (NEW),
								USER_INPUT, BCON_UTF8 (EXECUTE),
								CLIENT_NUM, BCON_INT32 (data->device_no),
								URL , BCON_UTF8 (dtriggerURI),
							"}");
			
			
			int update_ret = doc_update(db_addr,db_name,collection_name,query,update);
			if(update_ret != 0)
					return COAP_400_BAD_REQUEST;
		}
			
		
		
		break;
	
	case RES_M_UPDATE:

	
		//fireResourceChanged(common_context, dstateURI, &dUPDATING);
		fprintf(stdout, "\n\t SOFTWARE UPDATE\r\n\n");
		//ret = exec_run(objectP,instanceId);
		if (mongo_ret == 0){
			query = bson_new ();
			BSON_APPEND_UTF8(query, ROLE, "broker");
			update = bson_new ();

			update = BCON_NEW ("$set","{",
								REQUEST_FLAG, BCON_UTF8 (NEW),
								USER_INPUT, BCON_UTF8 (EXECUTE),
								CLIENT_NUM, BCON_INT32 (data->device_no),
								URL , BCON_UTF8 (dupdateURI),
							"}");
			
			int update_ret = doc_update(db_addr,db_name,collection_name,query,update);
			if(update_ret != 0)
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
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, "broker");
				update = bson_new ();

				update = BCON_NEW ("$set","{",
									REQUEST_FLAG, BCON_UTF8 (NEW),
									USER_INPUT, BCON_UTF8 (EXECUTE),
									CLIENT_NUM, BCON_INT32 (data->device_no),
									URL , BCON_UTF8 (dkillURI),
								"}");
					
				int update_ret = doc_update(db_addr,db_name,collection_name,query,update);
				if(update_ret != 0)
					return COAP_400_BAD_REQUEST;
			}
			
		}
		
		break;
		
	default:
		return COAP_405_METHOD_NOT_ALLOWED;
	}
	return COAP_204_CHANGED;
	
}









