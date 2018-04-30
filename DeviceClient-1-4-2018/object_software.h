#ifndef OBJECT_SOFTWARE_H
#define OBJECT_SOFTWARE_H

#include "object_software.h"
#include "liblwm2m.h"
#include "lwm2mclient.h"

/*#define IDLE 		0
#define DOWNLOADING 1
#define DOWNLOADED  2
#define UPDATING    3*/
#define ERROR       4

#define INIT 					 0
#define UPDATE_SUCCESS 			 1
#define NO_SPACE 		 		 2	
#define CONNECT_LOSS      		 3
#define INTEGRITY_FALIURE 		 4
#define UNSUPPORTED_PACKAGE_TYPE 5
#define INVALID_URI              6
#define UPDATE_FAILURE           7
#define UNSUPPORTED_PROTOCOL     8
#define SYSTEM_RUNNING_ERROR     9



#define URL_MAX_LENGTH               1024  
#define PACKAGE_DEFAULT_SIZE          100 
#define CMD_MAX_LENGTH                256



typedef struct _application_data_
{
	
	struct _application_data_ * next;
	uint16_t instanceId;
	
	int device_no;
	char role[50];
	char device_name[255];
	char container_supported[10];
	char image_name[255];
	char image_status[10];
	char container_status[10];
	char pid[1024];
	
	char download_cmd[CMD_MAX_LENGTH];
	char package_url[URL_MAX_LENGTH];
	uint8_t state;
    uint8_t result;
	float duration;
	
	char brokerNo;
	char name[64];
	char version[64];
	
} application_data_t;

typedef struct {
	application_data_t *appObject;
	char *cmd;
}app_control_data_t;

//download functions
int exec_download(lwm2m_object_t * objectP,uint16_t instanceId);
int download_cmd(application_data_t *data, char *cmd);
void *download_thread(void *arg);

//execute functions
int exec_run(lwm2m_object_t * objectP,uint16_t instanceId);
int exec_cmd(application_data_t * data, char *cmd);
void *exec_thread(void *arg);
int exec_check(char* short_pid,char* proc_pid, char* image_name);

void retrive_package_info (char *source_url,char *name, char* version);
int url_integrity_check(char* a_str, const char a_delim);
char** str_split(char* a_str, const char a_delim);

void *stop_thread(void *arg);

int thread_creation(application_data_t *data, char *cmd, void* thread_func);

/*assisst_func.c*/
int broker_discovery(char* broker_id, char *serverIP);

int container_stop(lwm2m_object_t * objectP, uint16_t instanceId);

int image_activ_check_thread(void *arg);

int image_check_thread(void *arg);

int is_image_activated(lwm2m_object_t * objectP,uint16_t instanceId);
int is_image_installed(lwm2m_object_t * objectP,uint16_t instanceId);
int container_trigger(lwm2m_object_t * objectP,uint16_t instanceId);
int trigger_thread(void *arg);
int trigger_check(char* short_pid,char* proc_pid);

void fireResourceChanged(lwm2m_context_t * context, char *url, void* resource_value);
#endif