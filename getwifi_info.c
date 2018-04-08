#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <math.h>
#include "cJSON.h"
#define MAX_BUFFER (1024*200)
#define  MAX_ARRAY  1024
typedef int BOOL;

enum RouteType
{
	WPA_RTYPE = 0,	
	WPA2_RTYPE,
	WEP_RTYPE,
	NONE_RTYPE,
};

enum Loglevel{
	LOG_CRIT_S = 0,
	LOG_ERR_S,
	LOG_WARNING_S,
	LOG_INFO_S,
	LOG_DEBUG_S,
};

enum Returnlevel{
	CT_ERROR_NO_ERROR = 0,
	CT_ERROR_NO_MEMORY,
	CT_ERROR_BAD_PARAMETER,
	CT_ERROR_NOT_SUPPORT,
	CT_ERROR_NO_RESPONSE,
	CT_ERROR_MALLOC_ERROR,
	CT_ERROR_COMMON_ERROR,
};

typedef struct ssidsInfo_s
{
	char ESSID[MAX_ARRAY];
	char password[MAX_ARRAY];
	int type;
	BOOL profile_flag;
	int signal;
	int encrypt;
} ssidsInfo_t;

char * GetKeyStr(char *buf, char *frontStr, char *behindStr, char *keyBuf, int keyBufLen)
{
	char *pStart = NULL;
	char *pEnd = NULL;

	if(!buf || !frontStr || !behindStr || !keyBuf)
	{
		printf("%s(%d):bad param,%p#%p#%p#%p#!!!\n" , __FUNCTION__ , __LINE__,
			buf, frontStr, behindStr, keyBuf);
		return NULL;
	}

	if(NULL == (pStart = strstr(buf, frontStr)))
	{
		//printf("%s(%d):can't find front  str!!!\n" , __FUNCTION__ , __LINE__);
		return NULL;
	}
	pStart = pStart + strlen(frontStr);

	if(NULL == (pEnd = strstr(pStart, behindStr)))
	{
		if(strlen(pStart) > (keyBufLen - 1))
		{
			printf("%s(%d):no enough space!!!\n" , __FUNCTION__ , __LINE__);
			return NULL;
		}
		strcpy(keyBuf, pStart);

		return pStart;            
	}

	/*if key buf len little than key str*/
	if(keyBufLen <= (pEnd - pStart))
	{
		printf("%s(%d):ket buf is too little      !!!\n" , __FUNCTION__ , __LINE__);
		return NULL;
	}
	memcpy(keyBuf, pStart, pEnd - pStart);

	return pStart;
}

int  RunScriptCommand(char *command, char *retbuf, int bufsize)
{
	FILE   *stream;

	if(!command)
	{
		printf("%s(%d) : command is null\n" , __FUNCTION__ , __LINE__);
		return 2;
	}

	sprintf(command, "%s &", command);
	printf("%s(%d):\n##command is:%s\n" , __FUNCTION__ , __LINE__, command);
	stream = popen(command, "r");

	if(NULL != retbuf)
	{
		fread( retbuf, sizeof(char), bufsize,  stream); 
	}
	if(retbuf)
	{
		//printf("%s(%d):\n##retBuf :%s\n" , __FUNCTION__ , __LINE__, retbuf);
	}

	pclose(stream);

	return 0;
}


int  getPSK(char *ssid, char *password, char *PSK)
{
	char keyBuf[MAX_ARRAY] = {0};
	char command[MAX_ARRAY] = {0};
	char retBuf[MAX_ARRAY] = {0};
	char *guard_p = NULL;

	memset(command, 0, sizeof(command));
	memset(retBuf, 0, sizeof(retBuf));
	memset(keyBuf, 0, sizeof(keyBuf));

	sprintf(command, "wpa_passphrase  \"%s\"   %s", ssid, password);
	RunScriptCommand(command, retBuf, sizeof(retBuf));

	if(NULL == (guard_p = strstr(retBuf, "psk")))
	{
		syslog(LOG_ERR , "%s(%d) :find psk err\n" , __FUNCTION__ , __LINE__);
		return CT_ERROR_COMMON_ERROR;
	}
	GetKeyStr(guard_p + 3, "psk=", "\n", keyBuf, sizeof(keyBuf));

	strcpy(PSK, keyBuf);

	return CT_ERROR_NO_ERROR;
}

/*********************************************
*make the config file for connect AP
**********************************************/
int makeRouteConfigFile(char *ssid, char *password, int RouteType, char *path)
{
	FILE *fp;
	char confBuf[MAX_ARRAY] = {0};
	char pskBuf[MAX_ARRAY] = {0};

	if(!ssid || !path || !password)
	{
		syslog(LOG_ERR , "%s(%d) :param err\n" , __FUNCTION__ , __LINE__);
		return CT_ERROR_BAD_PARAMETER;
	}

	fp=fopen(path, "w");   
	if(fp==NULL)                       
	{
		syslog(LOG_ERR , "%s(%d) :open file err\n" , __FUNCTION__ , __LINE__);
		return CT_ERROR_COMMON_ERROR;
	}

	if(WEP_RTYPE != RouteType)
	{
		memset(pskBuf,0 ,sizeof(pskBuf));
		if(CT_ERROR_NO_ERROR != getPSK(ssid, password, pskBuf))
		{
			syslog(LOG_ERR , "%s(%d) :get psk err\n" , __FUNCTION__ , __LINE__);
			return CT_ERROR_COMMON_ERROR;
		}
	}

	/*creat the config file*/
	memset(confBuf, 0, sizeof(confBuf));
	if(WPA2_RTYPE == RouteType)
	{
		sprintf(confBuf, "network={\nssid=\"%s\"\nproto=RSN\nkey_mgmt=WPA-PSK\npairwise=CCMP TKIP\ngroup=CCMP TKIP\npsk=%s\n}\n", 
			ssid, pskBuf);
	}
	else if(WPA_RTYPE == RouteType)
	{
		sprintf(confBuf, "network={\nssid=\"%s\"\nscan_ssid=1\nkey_mgmt=WPA-EAP WPA-PSK  IEEE8021X NONE\nproto=WPA\npairwise=TKIP CCMP\ngroup=CCMP TKIP WEP104 WEP40\npsk=%s\n}\n", 
			ssid, pskBuf);
	}
	else if(WEP_RTYPE == RouteType)
	{
		sprintf(confBuf, "network={\nssid=\"%s\"\nscan_ssid=1\nkey_mgmt=NONE\nwep_key0=\"%s\"\nwep_tx_keyidx=0\n}\n", 
			ssid, password);
	}
	else
	{
		syslog(LOG_WARNING , "%s(%d):no  support route type      !!!\n" , __FUNCTION__ , __LINE__);
		fclose(fp);

		return CT_ERROR_COMMON_ERROR;
	}

	syslog(LOG_DEBUG , "%s(%d):the confBuf is:%s\n" , __FUNCTION__ , __LINE__, confBuf);

	fwrite(confBuf, strlen(confBuf) , 1, fp);		//write to file
	fclose(fp);

	return CT_ERROR_NO_ERROR;
}





int main(int argc, char *argv[])
{
	char *buf_p = NULL;
	ssidsInfo_t *ssidsInfo_p = NULL;
	ssidsInfo_t *ssidsInfo_guard_p = NULL;
	int ssidNum = 0;
	char *guardP = NULL;
	char cell[2*MAX_ARRAY] = {0};
	char ESSID[MAX_ARRAY] = {0};
	char signal[16] = {'\0'};
	char lock[4] = {'\0'};
	char command[MAX_ARRAY] =  {0};
#if 0
	if(!netNodeName || !ssidsInfo || !Num)
	{
		syslog(LOG_WARNING , "%s(%d):bad param      !!!\n" , __FUNCTION__ , __LINE__);
		return CT_ERROR_BAD_PARAMETER;
	}
#endif
       
	if(NULL == (buf_p = malloc(MAX_BUFFER)))
	{
		syslog(LOG_ERR_S , "%s(%d):malloc err!!!\n" , __FUNCTION__ , __LINE__);
	}
	guardP = buf_p;

	sprintf(command, "iwlist %s  scanning", "wlan0");

	RunScriptCommand(command,buf_p, MAX_BUFFER);

	//sleep(1);
	//RunScriptCommand(command,buf_p, MAX_BUFFER);

	/*search  ESSID  &  assignment*/
	while(1)
	{	
//		ssidNum++;		//try fix bug...
		memset(cell, 0, sizeof(cell));
		memset(ESSID, 0, sizeof(ESSID));

		if(NULL == (guardP = GetKeyStr(guardP, "Cell", "Cell", cell, sizeof(cell))))
		{
//			*Num = ssidNum - 1;		//try fix bug..
			//*Num = ssidNum;	
			printf("parse cell done...\n");
			break;
		//	printf("...\n");
		}
	//	printf("guardp content is %s\n",guardP);
//		if(NULL == (ssidsInfo_p = (ssidsInfo_t *)realloc(ssidsInfo_p, ssidNum * sizeof(ssidsInfo_t))))//allocation the memory
		if(NULL == (ssidsInfo_p = (ssidsInfo_t *)realloc(ssidsInfo_p, (ssidNum+1) * sizeof(ssidsInfo_t))))//allocation the memory
		{
		//	syslog(LOG_ERR , "%s(%d):relloc err!!!\n" , __FUNCTION__ , __LINE__);

			if(buf_p)
			{
				free(buf_p);
				buf_p = NULL;
			}

			return CT_ERROR_MALLOC_ERROR;
		}

//		ssidsInfo_guard_p = ssidsInfo_p + ssidNum - 1;		//try fix bug...
		ssidsInfo_guard_p = ssidsInfo_p + ssidNum;		
		memset(ssidsInfo_guard_p, 0, sizeof(ssidsInfo_t));

		/*find the ESSID*/
	//	printf("cell content is %s\n",cell);
		if(NULL != GetKeyStr(cell, "ESSID:\"", "\"", ESSID, sizeof(ESSID)))
		{
			if(strlen(ESSID)==0){			//try fix ESSID is NULL issue..
				continue;			//important !!!!
			}
		//	printf("\033[36m current wifi ssid is %s\n",ESSID);
			if(strlen(ESSID) < sizeof(ssidsInfo_p->ESSID) )
			{
				strcpy(ssidsInfo_guard_p->ESSID, ESSID);
			}else{
				strncpy(ssidsInfo_guard_p->ESSID, ESSID, 32);
			}
		} 
		
	  	if(NULL != GetKeyStr(cell, "Quality=", "/", signal, sizeof(signal)))
	  	{
	  		if(strlen(signal)==0){			//try fix ESSID is NULL issue..
	  			continue;			//important !!!!
	  		}
	  		ssidsInfo_guard_p->signal = atoi(signal);
	  	//	printf("\033[32m cell signal tension is %d \033[0m\n",atoi(signal));
			memset(signal,'\0',sizeof(signal));
	  	} 
		if(NULL != GetKeyStr(cell, "Encryption key:", " ", lock, sizeof(lock)))
		{
		//	printf("\033[33m current wifi lock is %s \033[0m\n",lock);
		//	printf("lock len is %d\n",strlen(lock));
			lock[strlen(lock)-1] = '\0';
			if(strlen(lock)==0)			
				ssidsInfo_guard_p->encrypt = 0;
			if(strcmp(lock,"on") == 0)
				ssidsInfo_guard_p->encrypt = 1;
			else
				ssidsInfo_guard_p->encrypt = 0;
		//	printf("\033[33m current wifi lock is %d \033[0m\n",ssidsInfo_guard_p->encrypt);
			memset(lock,'\0',sizeof(lock));
		} 

		

		/*find the route type*/
		if(NULL != strstr(cell, "WPA2"))
		{
			ssidsInfo_guard_p->type = WPA2_RTYPE;
		}
		else if(NULL != strstr(cell ,"WPA"))
		{
			ssidsInfo_guard_p->type = WPA_RTYPE;
		}
		else
		{
			ssidsInfo_guard_p->type = WEP_RTYPE;
		}
		ssidNum++;		//try fix bug...

	//	printf("%s(%d):##num:%d##ESSID:%s##TYPE:%d\n" , __FUNCTION__ , __LINE__,
	//		ssidNum, ssidsInfo_guard_p->ESSID, ssidsInfo_guard_p->type);
	}

	printf("current wifi count is %d\n", ssidNum);
	int i;
	cJSON *root = NULL;
	cJSON *streamInfoJ = NULL;
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "wifi_count",ssidNum);
	cJSON_AddItemToObject(root, "wifilist", (streamInfoJ = cJSON_CreateArray()));
	//	cJSON *portItemJ = NULL;
	//	cJSON *privateDataJ = NULL;
	for(i = 0;i < ssidNum;i++)
	{
			ssidsInfo_guard_p =  ssidsInfo_p + i;
		//	printf("\033[33m current wifi ssid				is %s \033[0m\n",ssidsInfo_guard_p->ESSID);
		//	printf("\033[33m current wifi signal extension	is %d \033[0m\n",ssidsInfo_guard_p->signal);
		//	printf("\033[33m current wifi lock				is %d \033[0m\n",ssidsInfo_guard_p->encrypt);
		//	printf("\033[33m current wifi type				is %d \033[0m\n",ssidsInfo_guard_p->type);
		//	printf("\r\n");
			cJSON *row = cJSON_CreateObject();
			cJSON_AddNumberToObject(row,"ssidNum",i+1);
			cJSON_AddStringToObject(row,"ssid",ssidsInfo_guard_p->ESSID);
			cJSON_AddNumberToObject(row,"signal",ssidsInfo_guard_p->signal);
			cJSON_AddNumberToObject(row,"encrypt",ssidsInfo_guard_p->encrypt);
			cJSON_AddNumberToObject(row,"type",ssidsInfo_guard_p->type);
			cJSON_AddItemToArray(streamInfoJ,row);
	

	//	memset(ssidsInfo_guard_p->password, 0, MAX_ARRAY);

	//	if(CT_ERROR_NO_ERROR == ProfileKV_Query_single(pkv, ssidsInfo_guard_p->ESSID, ssidsInfo_guard_p->password))
	//	{
	//		ssidsInfo_guard_p->profile_flag = TRUE;
	//	}
	}
	printf("root is %s\n",cJSON_Print(root));
	char *out = cJSON_PrintUnformatted(root);
	
	cJSON_Delete(root);
//	return out;

	if(buf_p)
	{
		free(buf_p);
		buf_p = NULL;
	}

//	*ssidsInfo = ssidsInfo_p;

	return 0;
}


