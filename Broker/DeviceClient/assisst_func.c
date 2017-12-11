#include <stdio.h>
#include <string.h>

#include "object_software.h"

/* Custom Functions */
int broker_discovery(char* broker_id, char *serverIP)
{
  FILE *fp;
  char path[1035];
  
  char BrokerName[100] = {0};
  char Command[200] = {0};
  snprintf(BrokerName, 100, "%s",broker_id);
  snprintf(Command, 200, "avahi-browse -rtp _%s._sub._coap._udp",broker_id);

  printf("Looking for %s\r\n", BrokerName);
  
  /* Open the command for reading. */
  fp = popen(Command, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    return 1;
  }

  /* Read the output a line at a time - output it. */
  
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    printf("%s", path);
    
    if(path[0]=='=')
    {
      char* isIPv4 = strstr(path, "IPv4;");
      char* isBroker = strstr(path, BrokerName);
      
      if(isIPv4 && isBroker)
      {
        printf("BROKER FOUND\r\n");
        
        char* portPos = strstr(path, ";5683;");
        portPos--;
        
        /* from Port work backwards until reach ; */
        char tempIP[50] = {0};
        
        while(*portPos!=';')
        {
          tempIP[strlen(tempIP)]=*portPos;
          portPos--;
        }
        
        int i=0;
        int j=0;
        for(i=strlen(tempIP)-1;i>=0;i--)
        {
          serverIP[j]=tempIP[i];
          j++;
        }
        
        printf("IP is: %s\r\n", serverIP);
        
      }
    }
  }

  /* close */
  pclose(fp);
  return 0;
}

/*void main(){
	char serverIP[50];
	char broker_id[50];
	strcpy(serverIP,"none");
	strcpy(broker_id,"broker");
	
	broker_discovery(broker_id, serverIP);
	printf("serverIP is %s\n",serverIP);
}*/


