/*******************************************************************************
 *
 * Copyright (c) 2014 Bosch Software Innovations GmbH, Germany.
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
 *    Bosch Software Innovations GmbH - Please refer to git log
 *
 *******************************************************************************/
/*
 * lwm2mclient.h
 *
 *  General functions of lwm2m test client.
 *
 *  Created on: 22.01.2015
 *  Author: Achim Kraus
 *  Copyright (c) 2015 Bosch Software Innovations GmbH, Germany. All rights reserved.
 */

#ifndef LWM2MCLIENT_H_
#define LWM2MCLIENT_H_

#include "liblwm2m.h"
#include <libmongoc-1.0/mongoc.h>
extern int g_reboot;

extern lwm2m_context_t * common_context;
/*
 * object_device.c
 */
lwm2m_object_t * get_object_device(void);
void free_object_device(lwm2m_object_t * objectP);
uint8_t device_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);
void display_device_object(lwm2m_object_t * objectP);
/*
 * object_firmware.c
 */
lwm2m_object_t * get_object_firmware(void);
void free_object_firmware(lwm2m_object_t * objectP);
void display_firmware_object(lwm2m_object_t * objectP);

/*
 * object_software.c
 */
#define LWM2M_SOFTWARE_UPDATE_OBJECT_ID 1025
lwm2m_object_t * get_object_software(int brokerNo, char* name, lwm2m_context_t * context);
void free_object_software(lwm2m_object_t * objectP);
void display_software_object(lwm2m_object_t * objectP);
/*
 * object_device_update.c
 */
#define DEVICE_UPDATE_OBJECT_ID 1026
lwm2m_object_t * get_object_device_update(int brokerNo, char *dbName, char* name, lwm2m_context_t * context, char *db_url);
void free_object_device_update(lwm2m_object_t * object);
void display_object_device_update(lwm2m_object_t * objectP);

/*
 * object_location.c
 */
lwm2m_object_t * get_object_location(void);
void free_object_location(lwm2m_object_t * object);
void display_location_object(lwm2m_object_t * objectP);
/*
 * object_test.c
 */
#define TEST_OBJECT_ID 1024
lwm2m_object_t * get_test_object(void);
void free_test_object(lwm2m_object_t * object);
void display_test_object(lwm2m_object_t * objectP);

/*
 * object_server.c
 */
lwm2m_object_t * get_server_object(int serverId, const char* binding, int lifetime, bool storing);
void clean_server_object(lwm2m_object_t * object);
void display_server_object(lwm2m_object_t * objectP);
void copy_server_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 * object_connectivity_moni.c
 */
lwm2m_object_t * get_object_conn_m(void);
void free_object_conn_m(lwm2m_object_t * objectP);
uint8_t connectivity_moni_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);

/*
 * object_connectivity_stat.c
 */
extern lwm2m_object_t * get_object_conn_s(void);
void free_object_conn_s(lwm2m_object_t * objectP);
extern void conn_s_updateTxStatistic(lwm2m_object_t * objectP, uint16_t txDataByte, bool smsBased);
extern void conn_s_updateRxStatistic(lwm2m_object_t * objectP, uint16_t rxDataByte, bool smsBased);

/*
 * object_access_control.c
 */
lwm2m_object_t* acc_ctrl_create_object(void);
void acl_ctrl_free_object(lwm2m_object_t * objectP);
bool  acc_ctrl_obj_add_inst (lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t acObjectId, uint16_t acObjInstId, uint16_t acOwner);
bool  acc_ctrl_oi_add_ac_val(lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t aclResId, uint16_t acValue);
/*
 * lwm2mclient.c
 */
void handle_value_changed(lwm2m_context_t* lwm2mH, lwm2m_uri_t* uri, const char * value, size_t valueLength);
/*
 * system_api.c
 */
void init_value_change(lwm2m_context_t * lwm2m);
void system_reboot(void);

/*
 * object_security.c
 */
lwm2m_object_t * get_security_object(int serverId, const char* serverUri, char * bsPskId, char * psk, uint16_t pskLen, bool isBootstrap);
void clean_security_object(lwm2m_object_t * objectP);
char * get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
void display_security_object(lwm2m_object_t * objectP);
void copy_security_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);
/*
 * mongodb_op.c
 */
int db_connection(char *uri_str, char *db_name, char *collection_name, mongoc_collection_t *collection);
int doc_query(char *client_url, char *db_name, char *collection_name,bson_t *query);
int doc_insert(char *client_url, char *db_name, char *collection_name,bson_t* doc);
int doc_update(char *update_uri, char *db_name, char *collection_name,bson_t* query, bson_t* update);
int doc_delete(char *delete_uri, char *db_name, char *collection_name,bson_t* query);
int doc_count(char *count_uri, char *db_name, char *collection_name,bson_t* query);
char* doc_read_str(char *read_uri, char *db_name, char *collection_name,bson_t* query, const char *key, char* value);
int doc_read_int(char *read_uri, char *db_name, char *collection_name,bson_t* query, const char *key, int32_t value);
double doc_read_double(char *read_uri, char *db_name, char *collection_name,bson_t* query, const char *key, double value);
int JSON2BSON();

/*
 * object_software_provisioning.c
 */
# define LWM2M_SOFTWARE_PROVISION_OBJECT_ID 1027
lwm2m_object_t * get_object_software_provision(int brokerNo, char *dbName, char* name,lwm2m_context_t * context, char *db_url);
void free_object_software_provision(lwm2m_object_t * objectP);
void display_object_software_provision(lwm2m_object_t * objectP);

#endif /* LWM2MCLIENT_H_ */
