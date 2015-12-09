#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>  
#include <arpa/inet.h>
#include <netdb.h>  
#include <string.h>  
#include "cJSON.h"
#include "weblib.h"
#include "web_interface.h"

char *send_web_get(char *url,char *post_message,int timeout)
{
	char request[1024]={0};
	int result=0;
	sprintf(request,"%s?JSONStr=%s",url,post_message);
	printf(LOG_PREFX"send web %s\n",request);
	//char *rcv=http_post(url,post_message,timeout);
	char *rcv=http_get(request,timeout);
	if(rcv!=NULL)
		printf(LOG_PREFX"rcv %s\n",rcv);
	else
		printf(LOG_PREFX"no rcv got\n");
	return rcv;
}
char *send_web_post(char *url,char *post_message,int timeout)
{
	char request[1024]={0};
	int result=0;
	sprintf(request,"JSONStr=%s",post_message);
	printf(LOG_PREFX"send web %s\n",request);
	char *rcv=http_post(url,request,timeout);
	//char *rcv=http_get(request,timeout);
	if(rcv!=NULL)
		printf(LOG_PREFX"rcv %s\n",rcv);
	else
		printf(LOG_PREFX"no rcv got\n");
	return rcv;
}

int upload_data(char *type,char *uid,char *url,char *ipaddr,char *port,char *id0,char *data0,char *id1,char *data1,char *time,int timeout)
{
	int result=0,i,j;
	char text_out[512]={0};
	char *post_message=NULL,*rcv=NULL;
	//get device uid,ip,port,cap data,cap time send to server
	post_message=add_item(NULL,ID_DGRAM_TYPE,type);
	post_message=add_item(post_message,ID_DEVICE_UID,uid);
	post_message=add_item(post_message,ID_DEVICE_IP_ADDR,ipaddr);//"192.168.1.63");
	post_message=add_item(post_message,ID_DEVICE_PORT,port);//"6547");
	if(atoi(type)==2)
	{	
		post_message=add_item(post_message,id0,data0);
		post_message=add_item(post_message,id1,data1);
		post_message=add_item(post_message,ID_DEVICE_CAP_TIME,time);
	}
	else if(atoi(type)==4)
	{	
		post_message=add_item(post_message,id0,data0);
	}

	printf(LOG_PREFX"<GET>%s\n",post_message);
#if 0
	j=0;
	for(i=0;i<strlen(post_message);i++)
	{
		if(post_message[i]=='\n'||post_message[i]=='\r'||post_message[i]=='\t')
			j++;
	}
	char *out1=malloc(strlen(post_message)-j+1);
	memset(out1,'\0',strlen(post_message)-j+1);
	j=0;
	for(i=0;i<strlen(post_message);i++)
	{
		if(post_message[i]!='\r'&&post_message[i]!='\n'&&post_message[i]!='\t')		
		{
			out1[j++]=post_message[i];
		}
	}
#endif
	if(atoi(type)==2)
	rcv=send_web_post(url,post_message,timeout);
	else
	rcv=send_web_post(url,post_message,timeout);

	free(post_message);
	//free(out1);
	if(rcv!=NULL)
	{	
		int len=strlen(rcv);
		//rcv[len-1]='\0';
		printf(LOG_PREFX"<=== %s\n",rcv);
		//if(strncmp(rcv,"ok",strlen("ok"))==0 ||strncmp(rcv,"200",strlen("200"))==0) 
		{
			printf(LOG_PREFX"send ok\n");
			char *starttime=NULL;
			char *tmp=NULL;
			//strcpy(rcv,"{\"30\":\"230FFEE9981283737D\",\"210\":\"2015-08-27 14:43:57.0\",\"211\":\"???,????,???,313131\",\"212\":\"??\",\"213\":\"??\",\"104\":\"2015-09-18 11:53:58\",\"201\":[],\"202\":[]}");
			if(atoi(type)==5)
			{
				starttime=doit_data(rcv,(char *)"210");
				tmp=doit_data(rcv,(char *)"211");
				printf("201 %s\r\n",doit(rcv,"201"));
				printf("202 %s\r\n",doit(rcv,"202"));
			}
			else if(atoi(type)==6)
			{
				starttime=doit_data(rcv,(char *)"101");
				tmp=doit_data(rcv,(char *)"102");
			}
			if(starttime!=NULL)
			{
				printf("%s\r\n",starttime);
				free(starttime);
			}
			if(tmp!=NULL)
			{
				printf("%s\r\n",tmp);
				free(tmp);
			}
			result=1;
		}
		free(rcv);
	}
	return result;
}

//get cmd http://101.200.236.69:8080/lamp/lamp/commond/wait?lampCode=aaaa
//ack http://101.200.236.69:8080/lamp/lamp/commond/response?commondId=f1d51484-8daf-47a3-a490-f56461d3ce23&isSuccess=true
int main(int argc,char *argv[])
{
	if(atoi(argv[1])==2)
	{
		printf("type %s\r\nuid %s\r\nipaddr %s\nport %s\nid0 %s\ndata0 %s\nid1 %s\ndata1 %s\ntime %s\nurl %s\ntimeout %d\n",
			argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],argv[9],argv[10],atoi(argv[11]));		
		if(upload_data(argv[1],argv[2],argv[10],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],argv[9],atoi(argv[11])))
			printf("send data ok\n");
		else
			printf("send data failed\n");
	}
	else
	{
		if(atoi(argv[1])==4)
		{
			printf("type %s\r\nuid %s\r\nipaddr %s\nport %s\nurl %s\ntimeout %d\n92 string:%s\n",
				argv[1],argv[2],argv[3],argv[4],argv[5],atoi(argv[6]),argv[7]);		
			if(upload_data(argv[1],argv[2],argv[5],argv[3],argv[4],argv[5],argv[6],NULL,NULL,NULL,atoi(argv[7])))
				printf("send data ok\n");
			else
				printf("send data failed\n");

		}
		else
		{
			printf("type %s\r\nuid %s\r\nipaddr %s\nport %s\nurl %s\ntimeout %d\n",
				argv[1],argv[2],argv[3],argv[4],argv[5],atoi(argv[6]));		
			if(upload_data(argv[1],argv[2],argv[5],argv[3],argv[4],NULL,NULL,NULL,NULL,NULL,atoi(argv[6])))
				printf("send data ok\n");
			else
				printf("send data failed\n");
		}
	}
	return 0;
}

