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


static const char *BROKER="broker";
static const char *END="End Device";

static const char *ROLE= "role";
static const char *USER_INPUT = "usrInput";
static const char *URL = "resURL";
static const char *VALUE = "resValue";
static const char *SEND_FLAG = "sendFlag";
static const char *PACKAGE_URL = "container url";
static const char *REQUEST_FLAG = "reqFlag";
static const char *NUM_CLIENTS = "numClients";
static const char *CLIENT_NUM = "clientNo";
static const char *CLIENT_LIST = "clientList";
static const char *NUM_NODE2UPDATE = "num_node2update";
static const char *NUM_PROCESSED_NODE = "num_processed_node";
static const char *NUM_SUCCESSFUL_NODE = "num_successful_node";
static const char *NUM_FAILED_NODE = "num_failed_node";
static const char *TIME_CONSUMPTION = "broker time consumption";
static const char *CLIENT_UPDATE_RESULT = "Client Update Result";


static const char *DEVICE_NO = "DEVICE NO";
static const char *OBJECT_ID = "OBJECT ID";
static const char *INSTANCE_ID = "INSTANCE ID";
static const char *DEVICE_ROLE = "DEVICE ROLE";
static const char *DEVICE_NAME = "DEVICE NAME";
static const char *CONTAINER_SUPPORTED = "CONTAINER SUPPORTED?";
static const char *PKG_NAME = "IMAGE NAME";
static const char *PKG_STATUS = "IMAGE STATUS";
static const char *DOWNLOAD_CMD = "EXECUTE CMD";
static const char *TRIGGER = "TRIGGER";
static const char *STATUS = "CONTAINER STATUS";
static const char *PID = "CONTAINER ID";
static const char *KILL = "KILL";
static const char *PACKAGE_URI = "CONTAINER URL";
static const char *UPDATE = "UPDATE";
static const char *STATE = "UPDATE STATE";
static const char *UPDATE_RESULT  = "UPDATE RESULT";
static const char *DURATION = "UPDATE DURATION";

static const char *find = "/?appname=find-example";
static const char *insert = "/?appname=insert-example";
static const char *update = "/?appname=update-example";
static const char *delete = "/?appname=delete-example";
static const char *count = "/?appname=count-example";


static const char roleURI[9]="/1025/0/0";
static const char deviceURI[9]="/1025/0/1";
static const char supportURI[9]="/1025/0/2";
static const char inameURI[9]="/1025/0/3";
static const char istatusURI[9]="/1025/0/4";
static const char cmdURI[9]="/1025/0/5";
static const char triggerURI[9]="/1025/0/6";
static const char statusURI[9]="/1025/0/7";
static const char pidURI[9]="/1025/0/8";
static const char killURI[9]="/1025/0/9";

static const char urlURI[10]="/1025/0/10";
static const char updateURI[10]="/1025/0/11";
static const char stateURI[10]="/1025/0/12";
static const char resultURI[10]="/1025/0/13";
static const char durationURI[10]="/1025/0/14";

static const char *NEW="new";
static const char *OLD="old";
static const char *READ="read";
static const char *WRITE="write";
static const char *EXECUTE="exec";
static const char *LIST="list";
static const char *GROUP_UPDATE="group update";


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
	char registry_addr[64];
	
} application_data_t;


typedef struct _software_provision_
{
	
	struct _software_provision_ * next;
	uint16_t instanceId;
	
	int num_clients;
	char package_url[1024];
	int num_processed_nodes;
	int num_successful_nodes;
	int num_failed_nodes;
	double duration;
	
} software_provision_t;

typedef struct {
	application_data_t *appObject;
	char* cmd;
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

void fireResourceChanged(lwm2m_context_t * context, const char *url_name, void* resource_value);

char *getip4addr();
#endif