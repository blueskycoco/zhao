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
int send_web(int msgid,char *url,char *post_message,int timeout)
{
	char request[1024]={0};
	int result=0;
	printf(LOG_PREFX"send web %s %s\n",url,post_message);
	char *rcv=http_post(url,post_message,timeout);
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
}
void get_param(char *input,char *message,char *commandid,int *websiteid,int *timeout,char *lampcode,char *text)
{
	int j=0;
	int i=strlen(input)-1;
	memset(lampcode,'\0',sizeof(lampcode));
	memset(text,'\0',sizeof(text));
	strcpy(text,"message=");
	strcpy(lampcode,"lampCode=");
	memset(message,'\0',sizeof(message));
	memset(commandid,'\0',sizeof(commandid)); 				
	strcpy(message,"message=");
	strcpy(commandid,"commandId=");
	*timeout=-1;
	*websiteid=-1;
	//01;w;s;1;12345678;9f5d7b29-1c03-4c29-90af-ae7cd354b6ae;2;3
	*timeout=atoi(strrchr(input,';')+1);
	 while(input[i]!=';' && i>0)
	 	i--;
	 i--;
	 *websiteid=input[i]-48;
	 i=i-2;
	 j=i;
	 while(input[i]!=';' && i>0)
	 	i--;
	 memcpy(commandid+strlen("commandId="),input+i+1,j-i);	 
	 memcpy(message+strlen("message="),input,i);
	 i--;
	 j=i;
	 while(input[i]!=';' && i>0)
	 	i--;
	 memcpy(lampcode+strlen("lampCode="),input+i+1,j-i);	
	 //i--;
	 //j=i;
	 //while(input[i]!=';' && i>0)
	 //	i--;
	 memcpy(text+strlen("message="),input,i);
}
int get_server_cmd(int msgid,char *url,char *lampcode)
{
	int result=0;
	char text_out[512]={0};
	char request[1024]={0};
	strcpy(request,url);
	strcat(request,lampcode);
	char *rcv=http_get(request,10);
	printf(LOG_PREFX"===> %s\n",request);
	if(rcv!=NULL)
	{
		printf("<=== %s\n",rcv);
			
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
		free(rcv);
	}
	return result;
}
int main(int argc, char *argv[])
{

	pid_t fpid;	
	int status;
	int msgid = -1;
	unsigned char *rec_result=NULL;
	char res[256]={0};
	char record_file[256]={0};
	char playback_file[256]={0};
	struct msg_st data;	
	char err_msg[256]={0};
	char text_out[256]={0};
	char last_check=0;
	char file_name[256]={0};
	char operation=0;//0 for read ,1 for write
	int offs=0;
	int len;
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
				if(!get_server_cmd(msgid,url,lampcode))
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
		char lampcode[256]={0};
		char text[256]={0};
		char commandid[256]={0};
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
						memset(message,0,256);
						memset(commandid,0,256);
						memset(lampcode,0,256);
						memset(text,0,256);
						get_param(data.text,message,commandid,&websiteid,&timeout,lampcode,text);
						printf(LOG_PREFX"input %s\nmessage %s\ncommandid %s\nwebsiteid %d\ntimeout %d\nlampcode %s\ntext %s\n",
							data.text,message,commandid,websiteid,timeout,lampcode,text);
						if(strstr(g_url[websiteid],"synch")!=0||strstr(g_url[websiteid],"achieve")!=0)
						{
							if(send_web(msgid,g_url[websiteid],lampcode,text,timeout))
								strcat(errorMsg,";done");
							else
								strcat(errorMsg,";failed");

						}
						else
						{
							if(send_web(msgid,g_url[websiteid],commandid,message,timeout))
								strcat(errorMsg,";done");
							else
								strcat(errorMsg,";failed");
						}
							//if(strstr(g_url[websiteid],"achieve")!=0)
							//send_msg(msgid,TYPE_WEB_TO_MAIN,WEB_TO_MAIN,errorMsg);
						
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

