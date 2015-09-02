#include "web_interface.h"
#include "weblib.h"
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fnmatch.h> 
#include <signal.h>
#include <sys/ipc.h>  
#include <sys/shm.h>  

char g_url[10][256]={0};
char *g_lampcode=NULL;
int send_msg(int msgid,unsigned char msg_type,unsigned char id,unsigned char *text)
{
	struct msg_st data;
	data.msg_type = msg_type;
	data.id=id;
	memset(data.text,'\0',512);
	printf(LOG_PREFX"send msg\n");
	printf(LOG_PREFX"MSG_TYPE %d\n",msg_type);
	printf(LOG_PREFX"MSG_ID %d\n",id);
	if(text!=NULL)
	{
		memcpy(data.text,text,strlen(text));
		printf(LOG_PREFX"MSG_TEXT %s\n",text);
	}
	if(msgsnd(msgid, (void*)&data, sizeof(struct msg_st)-sizeof(long int), IPC_NOWAIT) == -1)  
	{  
		fprintf(stderr, LOG_PREFX"msgsnd failed %s\n",strerror(errno));
		system("ipcs -q");
	}
	printf(LOG_PREFX"send msg done\n");
}
char *send_web(char *url,char *post_message,int timeout)
{
	char request[1024]={0};
	int result=0;
	sprintf(request,"%s?a=%s",url,post_message);
	printf(LOG_PREFX"send web %s\n",request);
	//char *rcv=http_post(url,post_message,timeout);
	char *rcv=http_get(request,timeout);
	if(rcv!=NULL)
		printf(LOG_PREFX"rcv %s\n",rcv);
	else
		printf(LOG_PREFX"no rcv got\n");
#if 0
	if(rcv!=NULL)
	{
		printf("%s\n",rcv);
		char res=doit_ack(rcv,"success");
		if(res)
		{
			printf(LOG_PREFX"code is %d\n",res);
			result=1;
			if(strstr(url,"achieve")!=0)
			{
				char *data=doit_data(rcv,"data");
				if(data)
				{
					send_msg(msgid,TYPE_WEB_TO_MAIN,WEB_TO_MAIN,data);
					free(data);
				}
			}
		}
		free(rcv);
	}
	return result;
#endif
	return rcv;
}
//"192.168.1.23;7865;60;0.2321;61;2.2321;2015-08-21 14:30;3"
void get_param(char *input,char *ipaddr,char *port,char *id0,char *data0,char *id1,char *data1,char *time,int *websiteid,int *timeout)
{
	int i=0,j=0;
	while(input[i]!=';' && input[i]!='\0')
		ipaddr[j++]=input[i++];
	i++;
	j=0;
	while(input[i]!=';' &&input[i]!='\0')
		port[j++]=input[i++];
	i++;
	j=0;
	while(input[i]!=';' &&input[i]!='\0')
		id0[j++]=input[i++];
	i++;
	j=0;
	while(input[i]!=';' &&input[i]!='\0')
		data0[j++]=input[i++];
	i++;
	j=0;
	while(input[i]!=';' &&input[i]!='\0')
		id1[j++]=input[i++];
	i++;
	j=0;
	while(input[i]!=';' &&input[i]!='\0')
		data1[j++]=input[i++];
	i++;
	j=0;
	while(input[i]!=';' &&input[i]!='\0')
		time[j++]=input[i++];
	i++;
	*websiteid=input[i]-48;
	*timeout=atoi(strrchr(input,';')+1);
}
int ask_re_send(int msgid,char *url)
{

}
int sync_server(int msgid,char *url)
{

}
int upload_data(int msgid,char *url,char *ipaddr,char *port,char *id0,char *data0,char *id1,char *data1,char *time,int timeout)
{
	int result=0;
	char text_out[512]={0};
	char *post_message=NULL,*rcv=NULL;
	//get device uid,ip,port,cap data,cap time send to server
	post_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
	post_message=add_item(post_message,ID_DEVICE_UID,g_lampcode);
	post_message=add_item(post_message,ID_DEVICE_IP_ADDR,ipaddr);//"192.168.1.63");
	post_message=add_item(post_message,ID_DEVICE_PORT,port);//"6547");
	post_message=add_item(post_message,id0,data0);
	post_message=add_item(post_message,id1,data1);
	post_message=add_item(post_message,ID_DEVICE_CAP_TIME,time);
	printf(LOG_PREFX"<post>%s\n",post_message);
	rcv=send_web(url,post_message,timeout);
	free(post_message);
	if(rcv!=NULL)
	{
		printf(LOG_PREFX"<=== %s\n",rcv);
		if(strncmp(rcv,"ok",strlen("ok"))==0 ||strncmp(rcv,"200",strlen("200"))==0) 
		{
			printf(LOG_PREFX"send ok\n");
			result=1;
		}
		#if 0
		char *commandid=doit(rcv,"commondId");
		char *message=doit(rcv,"message");
		memset(text_out,'\0',sizeof(text_out));

		if(message && commandid)
		{
			result=1;
			if(message[strlen(message)-1]==';')
			{
				strcpy(text_out,strchr(message,';')+1);
				strcat(text_out,"w;s;");
			}
			else
			{
				int i=0;
				while(message[i]!=';' && message[i]!='\0')
					i++;
				memcpy(text_out,message+i+1,3);
				strcat(text_out,"w;");			
				strcat(text_out,message+i+1+3);
				strcat(text_out,";");
			}
			strcat(text_out,lampcode);
			strcat(text_out,";");
			strcat(text_out,commandid);			
			send_msg(msgid,TYPE_WEB_TO_MAIN,WEB_TO_MAIN,text_out);
		}
		if(message)
			free(message);
		if(commandid)
			free(commandid);	
		#endif
		free(rcv);
	}
	return result;
}
int main(int argc, char *argv[])
{
	pid_t fpid;	
	int status;
	int msgid = -1;
	struct msg_st data;	
	char *url0;
	key_t shmid,shmidc;  

	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);  
	if(msgid == -1)  
	{  
		fprintf(stderr, LOG_PREFX"msgget failed with error: %d\n", errno);  
		exit(-1);  
	}
	else
		printf(LOG_PREFX"msgid %d\n",msgid);	
	if((shmid = shmget(IPC_PRIVATE, 256, PERM)) == -1 )
	{
		fprintf(stderr, LOG_PREFX"Create Share Memory Error:%s/n/a", strerror(errno));  
		exit(1);  
	}  
	shmidc = shmget(IPC_PRIVATE, 256, PERM);
	url0 = (char *)shmat(shmid, 0, 0);
	g_lampcode = (char *)shmat(shmidc, 0, 0);
	fpid=fork();
	if(fpid==0)
	{
		char *url=(char *)shmat(shmid, 0, 0);
		char *lampcode=(char *)shmat(shmidc, 0, 0);
		while(1)
		{
			if(strlen(url)!=0 && strlen(lampcode)!=0)
			{
				//if(!get_server_cmd(msgid,url,lampcode))
					sleep(1);
			}
			else
			{
				//printf(LOG_PREFX"wait for main configure url...\n");
				sleep(1);
			}
		}
	}
	else if(fpid>0)
	{
		char ipaddr[256]={0};
		char port[256]={0};
		char id0[256]={0};
		char id1[256]={0};
		char data0[256]={0};
		char data1[256]={0};
		char time[256]={0};
		char errorMsg[256]={0};
		int websiteid=0;
		int timeout=0;
		int i=0,j=0;
		char message[256]={0};
		while(1)
		{
			//printf(LOG_PREFX"waiting MainCtlSystem cmd...\n");
			memset(data.text,0,512);
			if(msgrcv(msgid, (void*)&data, sizeof(struct msg_st)-sizeof(long int), TYPE_MAIN_TO_WEB , 0)>=0)
			{			
				memset(errorMsg,'\0',256);
				strcpy(errorMsg,data.text);
				if(data.id==MAIN_TO_WEB)
				{
					printf(LOG_PREFX"<<web>> %s\n",data.text);
					if(data.text[0]=='f' && data.text[1]=='f')
					{
						int m=3;
						for(i=0;i<10;i++)
						{
							j=m;
							while(data.text[m]!=';' && data.text[m]!='\0')
								m++;
							if(data.text[m]=='\0')
							{
								memset(g_lampcode,'\0',256);
								memcpy(g_lampcode,data.text+j,m-j);
								break;
							}
							else
							{								
								memset(g_url[i],'\0',256);
								memcpy(g_url[i],data.text+j,m-j);
								m++;
							}
							printf(LOG_PREFX"url %d %s\n",i,g_url[i]);
						}
						strcpy(url0,g_url[0]);//last is lampcode
						printf(LOG_PREFX"g_lampcode %s\n",g_lampcode);
						strcat(errorMsg,";done");
						send_msg(msgid,TYPE_WEB_TO_MAIN,WEB_TO_MAIN,errorMsg);
					}
					else
					{
						memset(data0,0,256);
						memset(id0,0,256);
						memset(data1,0,256);
						memset(id1,0,256);
						memset(time,0,256);
						memset(ipaddr,0,256);
						memset(port,0,256);
						get_param(data.text,ipaddr,port,id0,data0,id1,data1,time,&websiteid,&timeout);
						printf(LOG_PREFX"input %s\nipaddr %s\nport %s\nid0 %s\ndata0 %s\nid1 %s\ndata1 %s\ntime %s\nwebsiteid %d\ntimeout %d\n",
								data.text,ipaddr,port,id0,data0,id1,data1,time,websiteid,timeout);
						if(upload_data(msgid,g_url[websiteid],ipaddr,port,id0,data0,id1,data1,time,timeout))
							strcpy(errorMsg,"send data ok\n");
						else
							strcpy(errorMsg,"send data failed\n");
						send_msg(msgid,TYPE_WEB_TO_MAIN,WEB_TO_MAIN,errorMsg);

					}
				}
				else
				{
					strcat(errorMsg,";errorID");
					send_msg(msgid,TYPE_WEB_TO_MAIN,WEB_TO_MAIN,errorMsg);
				}
			}
			else
			{
				msgid = msgget((key_t)1234, 0666 | IPC_CREAT);  
				if(msgid == -1)  
				{  
					fprintf(stderr, LOG_PREFX"msgget failed with error: %d\n", errno);  
					sleep(1);
				}
				//else
				//printf(LOG_PREFX"msgid %d\n",msgid);
			}	
		}
	}
	waitpid(fpid, &status, 0);	
	return 0;
}

