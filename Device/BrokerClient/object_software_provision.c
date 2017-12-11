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

// ---- private object "SOFTWARE PROVISION" specific defines ----
// Resource Id's:

#define RES_M_REGISTRATION_UPDATE       0
#define RES_M_NUM_CLEINTS               1
#define RES_M_PACKAGE_URI               2     
#define RES_M_GROUP_UPDATE              3       
#define RES_M_UPDATE_PROCESS            4
#define RES_M_SUCESSFUL_NODES           5
#define RES_M_FAILED_NODES              6
#define RES_M_GROUP_UPDATE_DURATION     7
#define NUM_INSTANCE                    1
#define MAX_NUM_INSTANCE                5

char db_addr[1024];
char deviceName[32];
char collection_name[32];

char db_name[32];

//lwm2m_context_t * common_context;

static uint8_t prv_software_provision_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP);
static uint8_t prv_software_provision_write(uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP);
static uint8_t prv_software_provision_execute(uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP);
	

lwm2m_object_t * get_object_software_provision(int brokerNo, char *dbName, char* name,lwm2m_context_t * context, char *db_url)
{
    /*
     * The get_object_firmware function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * containerObj;

    containerObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
	
	
	snprintf(db_name,1024,dbName);
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
		software_provision_t* targetP;
		
        memset(containerObj, 0, sizeof(lwm2m_object_t));

        /*objectID : 1025*/
        containerObj->objID = LWM2M_SOFTWARE_PROVISION_OBJECT_ID;
       
		for (i=0 ; i < NUM_INSTANCE ; i++){
			targetP = (software_provision_t*)lwm2m_malloc(sizeof(software_provision_t));
			if (NULL == targetP) return NULL;
			memset(targetP, 0, sizeof(software_provision_t));
			targetP->num_clients = 0;
			//targetP->package_url  = NULL;
			targetP->num_processed_nodes   = 0;
			targetP->num_successful_nodes  = 0;
			targetP->num_failed_nodes      = 0;
			
			containerObj->instanceList = LWM2M_LIST_ADD(containerObj->instanceList, targetP);
			
			
			//initialize database
			/*fprintf(stdout,"connect to the %s\n",db_url);
			if (strlen(db_url)!=0){
				mongoc_collection_t *collection;
				int mongo_ret = db_connection(db_addr,db_name,collection_name,collection);
				if (mongo_ret==0){
					
					fprintf(stdout,"successfuly connect to the database\n");
		
					//query a document
					bson_t * query = bson_new ();
					BSON_APPEND_INT32 (query, DEVICE_NO, targetP->device_no);	
					
					snprintf(find_uri,1024,"%s/?appname=find-example",db_addr);
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
									  BCON_UTF8 (name),
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
						BSON_APPEND_UTF8 (doc, DEVICE_ROLE,"End Device");
						BSON_APPEND_UTF8 (doc, DEVICE_NAME,name);
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
			}*/
			
			
			
		}
		
		
		lwm2m_list_t * instanceP;

		for (instanceP = containerObj->instanceList; instanceP != NULL ; instanceP = instanceP->next)
		{
			fprintf(stdout, "the instance is created as /%d/%d, \n", containerObj, instanceP->id);
		}
		
		printf("the endpoint name is %s\n", name);
		printf("the broker number is %d\n", brokerNo);
		 
        containerObj->readFunc    = prv_software_provision_read;
        containerObj->writeFunc   = prv_software_provision_write;
        containerObj->executeFunc = prv_software_provision_execute;
        //containerObj->userData    = lwm2m_malloc(sizeof(software_provision_t));
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
    software_provision_t * data = (software_provision_t *)object->instanceList;
    fprintf(stdout, "  /%u: Software object:\r\n", object->objID);
    if (NULL != data)
    {
        fprintf(stdout, "    state: %u, supported: %s, result: %u\r\n",
                data->state, data->supported?"true":"false", data->result);
    }
#endif
}*/


void display_object_software_provision(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    fprintf(stdout, "  /%u: Test object, instances:\r\n", object->objID);
    software_provision_t * instance = (software_provision_t *)object->instanceList;
    while (instance != NULL)
    {
        fprintf(stdout, "    /%u/%u: shortId: %u, ",
                object->objID, instance->instanceId,
                instance->instanceId);
        instance = (software_provision_t *)instance->next;
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

void free_object_software_provision(lwm2m_object_t * object)
{
    LWM2M_LIST_FREE(object->instanceList);
    if (object->userData != NULL)
    {
        lwm2m_free(object->userData);
        object->userData = NULL;
    }
    lwm2m_free(object);
}



static uint8_t prv_software_provision_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
	software_provision_t * data = (software_provision_t*)lwm2m_list_find(objectP->instanceList, instanceId);
	
	//initialize database
	fprintf(stdout,"connect to the %s,database %s, collection %s\n",db_addr,db_name,collection_name);
	mongoc_collection_t *collection;
	int mongo_ret = db_connection(db_addr,db_name,collection_name,collection);
	bson_t * query;
	
	if (mongo_ret==0){
		printf("\r\n connected to database \r\n");
	}else{
		printf("\r\n failed connected to database \r\n");
	}
	
	
    if (instanceId > MAX_NUM_INSTANCE)
    {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
		*dataArrayP = lwm2m_data_new(13);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = 6;
     
		(*dataArrayP)[0].id = RES_M_NUM_CLEINTS;
        (*dataArrayP)[1].id = RES_M_PACKAGE_URI;
        (*dataArrayP)[2].id = RES_M_UPDATE_PROCESS;
		(*dataArrayP)[3].id = RES_M_SUCESSFUL_NODES;
		(*dataArrayP)[4].id = RES_M_FAILED_NODES;
		(*dataArrayP)[5].id = RES_M_GROUP_UPDATE_DURATION;
    }

    i = 0;
    do
    {
        switch ((*dataArrayP)[i].id)
        {	
		
		case RES_M_NUM_CLEINTS:
		
			printf("\t\n READ NUM OF CLIENTS\n\t");
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				data->num_clients = doc_read_int(db_addr,db_name,collection_name,query,NUM_CLIENTS,data->num_clients);
			}
			
			printf ("the number of registered client is %d\n",data->num_clients);
			lwm2m_data_encode_int(data->num_clients, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
		case RES_M_PACKAGE_URI:
		
			printf("\t\n READ PACKAGE_URL\n\t");
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				printf("\t\n READING FROM DATABASE\n\t");
				strcpy(data->package_url,doc_read_str(db_addr,db_name,collection_name,query,PACKAGE_URL,data->package_url));
				printf("\t\n READ FROM DATABASE\n\t");
			}
		
			lwm2m_data_encode_string(data->package_url, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
		
        case RES_M_UPDATE_PROCESS:
		
			printf("\t\n READ NUM OF PROCESSED NODES\n\t");
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				data->num_processed_nodes=doc_read_int(db_addr,db_name,collection_name,query,NUM_PROCESSED_NODE,data->num_processed_nodes);
			}
			
			lwm2m_data_encode_int(data->num_processed_nodes, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
		case RES_M_SUCESSFUL_NODES:
			
			printf("\t\n READ NUM OF NODES SUCCESSFULLY UPDATED\n\t");
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				data->num_successful_nodes=doc_read_int(db_addr,db_name,collection_name,query,NUM_SUCCESSFUL_NODE,data->num_successful_nodes);
			}
			
			lwm2m_data_encode_int(data->num_successful_nodes, *dataArrayP + i);
            result = COAP_205_CONTENT;
			
            break;
			
		case RES_M_FAILED_NODES:
		
			printf("\t\n READ NUM OF NODES FAILED TO UPDATE\n\t");
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				data->num_failed_nodes=doc_read_int(db_addr,db_name,collection_name,query,NUM_FAILED_NODE,data->num_failed_nodes);
			}
			
			lwm2m_data_encode_int(data->num_failed_nodes, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
			
		case RES_M_GROUP_UPDATE_DURATION:
		
			printf("\t\n READ GROUP UPDATE TIME CONSUMPTION\n\t");
		
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				data->duration=doc_read_double(db_addr,db_name,collection_name,query,TIME_CONSUMPTION,data->duration);
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

static uint8_t prv_software_provision_write(uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
	software_provision_t * data = (software_provision_t*)lwm2m_list_find(objectP->instanceList, instanceId);

    if (instanceId > MAX_NUM_INSTANCE)
    {
        return COAP_404_NOT_FOUND;
    }

    i = 0;
	
	//initialize database
	fprintf(stdout,"connect to the %s,database %s, collection %s\n",db_addr,db_name,collection_name);
	mongoc_collection_t *collection;
	int mongo_ret = db_connection(db_addr,db_name,collection_name,collection);
	bson_t * query;
	bson_t * update;
	
	if (mongo_ret==0){
		printf("\r\n connected to database \r\n");
	}else{
		printf("\r\n failed connected to database \r\n");
	}

    do
    {
        switch (dataArray[i].id)
        {
			
		case RES_M_NUM_CLEINTS:
		
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->num_clients)))
            {
                result = COAP_400_BAD_REQUEST;
            }else{
				result = COAP_204_CHANGED;
			}
			
			break;
			 
		case RES_M_PACKAGE_URI:
			
			printf("\t\n WRITE PACKAGE URL TO BROKER\n\t");
		
			strncpy(data->package_url, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
			data->package_url[dataArray[i].value.asBuffer.length] = 0x00;
			
			result = COAP_204_CHANGED;
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				
				update = BCON_NEW ("$set","{",
				REQUEST_FLAG, BCON_UTF8 (NEW),
				PACKAGE_URL, BCON_UTF8 (data->package_url),
												
				"}");
				
				
				doc_update(db_addr,db_name,collection_name,query,update);
			}

			//is_image_installed(objectP,instanceId);
			break;	
		
		case RES_M_UPDATE_PROCESS:
		
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->num_processed_nodes)))
            {
                result = COAP_400_BAD_REQUEST;
            }else{
				result = COAP_204_CHANGED;
			}
			
			break;
		
		case RES_M_SUCESSFUL_NODES:
		
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->num_successful_nodes)))
            {
                result = COAP_400_BAD_REQUEST;
            }else{
				result = COAP_204_CHANGED;
			}
			
			break;
			
		case RES_M_FAILED_NODES:
		
			if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t *)&(data->num_failed_nodes)))
            {
                result = COAP_400_BAD_REQUEST;
            }else{
				result = COAP_204_CHANGED;
			}
			
			break;
		
		case RES_M_GROUP_UPDATE_DURATION:
		
			if (1 != lwm2m_data_decode_float(dataArray + i, (double *)&(data->duration)))
            {
                result = COAP_400_BAD_REQUEST;
            }else{
				result = COAP_204_CHANGED;
			}
			
			break;

        default:
            result = COAP_405_METHOD_NOT_ALLOWED;
			break;
        }

        i++;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
}


static uint8_t prv_software_provision_execute(uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP)
{
	software_provision_t * data = (software_provision_t*)lwm2m_list_find(objectP->instanceList, instanceId);
   
	
    int i = 0;
	int ret;
	
	//initialize database
	fprintf(stdout,"connect to the %s,database %s, collection %s\n",db_addr,db_name,collection_name);
	mongoc_collection_t *collection;
	int mongo_ret = db_connection(db_addr,db_name,collection_name,collection);
	bson_t * query;
	bson_t * update;
	if (mongo_ret==0){
		printf("\r\n connected to database \r\n");
	}else{
		printf("\r\n failed connected to database \r\n");
	}
	
    // this is a single instance object
    if (instanceId > MAX_NUM_INSTANCE)
    {
        return COAP_404_NOT_FOUND;
    }

    if (length != 0) return COAP_400_BAD_REQUEST;

    // for execute callback, resId is always set.

	switch (resourceId)
	{
		
	case RES_M_REGISTRATION_UPDATE:

		
		printf("\t\n REGISTRATION Update\n\t");
		//ret = container_trigger(objectP,instanceId);
		
		if (mongo_ret == 0){
			query = bson_new ();
			BSON_APPEND_UTF8(query, ROLE, BROKER);
			update = bson_new ();
			
			update = BCON_NEW ("$set","{",
				
				USER_INPUT, BCON_UTF8 (LIST),
				REQUEST_FLAG, BCON_UTF8 (NEW),
												
				"}");
			
			doc_update(db_addr,db_name,collection_name,query,update);
		}else{
			return COAP_400_BAD_REQUEST;
		}
			
				
		break;
		
		
	case RES_M_GROUP_UPDATE:

		if (strlen(data->package_url)!=0)
		{
			printf("\t\n Group Update\n\t");
			//ret = container_trigger(objectP,instanceId);
			
			if (mongo_ret == 0){
				query = bson_new ();
				BSON_APPEND_UTF8(query, ROLE, BROKER);
				update = bson_new ();
				
				update = BCON_NEW ("$set","{",
				
				USER_INPUT, BCON_UTF8 (GROUP_UPDATE),
				REQUEST_FLAG, BCON_UTF8 (NEW),
												
				"}");
				
				doc_update(db_addr,db_name,collection_name,query,update);
			}else{
				return COAP_400_BAD_REQUEST;
			}
			
		}else{
			// firmware update already running
			printf("Empty Image\n");
			return COAP_400_BAD_REQUEST;
		}
		
		break;
	
	default:
		return COAP_405_METHOD_NOT_ALLOWED;
		break;
	}
	return COAP_204_CHANGED;
	
}









