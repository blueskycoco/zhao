#include "cap.h"
#include "xfer.h"
#include "log.h"
#include "misc.h"
#include "netlib.h"
pthread_mutex_t mutex;
extern struct share_memory *g_share_memory;
#define UPLOAD_PROCESS "[XFER] "

int xfer_init()
{
	if((g_share_memory->fd_gprs=open_com_port("/dev/ttySP2"))<0)
	{
		printfLog(UPLOAD_PROCESS"open_port gprs error");
		return -1;
	}
	if(set_opt(g_share_memory->fd_gprs,115200,8,'N',1)<0)
	{
		printfLog(UPLOAD_PROCESS"set_opt gprs error");
		close(g_share_memory->fd_gprs);
		return -1;
	}
	pthread_mutex_init(&mutex, NULL);
	return 0;
}
void send_web_post(char *url,char *buf,int timeout,char **out)
{
	char request[1024]={0};
	char length_string[30]={0};
	int i,ltimeout=0;
	char rcv[512]={0},ch;
	pthread_mutex_lock(&mutex);
	if(g_share_memory->send_by_wifi)
	{
		sprintf(request,"JSONStr=%s",buf);
		printfLog(UPLOAD_PROCESS"send web %s\n",request);
		*out=http_post(url,request,timeout);
		if(*out!=NULL)
		{
			printfLog(UPLOAD_PROCESS"<==%s\n",*out);
			g_share_memory->network_state=1;
		}
		else
		{
			printfLog(UPLOAD_PROCESS"<==NULL\n");
			g_share_memory->network_state=0;
		}
	}
	else
	{
		char gprs_string[1024]={0};
		int j=0;
		strcpy(gprs_string,"POST /saveData/airmessage/messMgr.do HTTP/1.1\r\nHOST: 123.57.26.24:8080\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\n");
		sprintf(length_string,"Content-Length:%d\r\n\r\nJSONStr=",strlen(buf)+8);
		strcat(gprs_string,length_string);
		strcat(gprs_string,buf);
		for(j=0;j<3;j++){
			printfLog(UPLOAD_PROCESS"send gprs %s\n",gprs_string);
			write(g_share_memory->fd_gprs, gprs_string, strlen(gprs_string));	
			i=0;
			while(1)
			{
				if(read(g_share_memory->fd_gprs, &ch, 1)==1)
				{
					printfLog(UPLOAD_PROCESS"%c",ch);
					if(ch=='}')
					{
						rcv[i++]=ch;
						*out=(char *)malloc(i+1);
						memset(*out,'\0',i+1);
						memcpy(*out,rcv,i);
						pthread_mutex_unlock(&mutex);
						g_share_memory->network_state=1;
						return;
					}
					else if(ch=='{')
						i=0;
					else if(ch=='o')
					{
						if(read(g_share_memory->fd_gprs,&ch,1)==1)
							if(ch=='k')
							{
								*out=(char *)malloc(3);
								memset(*out,'\0',3);
								strcpy(*out,"ok");
								memset(rcv,'\0',512);
								strcpy(rcv,"ok");
								pthread_mutex_unlock(&mutex);
								g_share_memory->network_state=1;
								return;
							}
					}

					rcv[i]=ch;
					i++;
				}
				else
				{
					if(timeout!=-1)
					{
						ltimeout++;
						usleep(1000);
						if(ltimeout>=timeout*1000)
						{
							printfLog(UPLOAD_PROCESS"gprs timeout %d\n",j);						
							*out=NULL;
							ltimeout=0;
							if(j==2)
							{
								pthread_mutex_unlock(&mutex);
								return;
							}
							g_share_memory->network_state=0;
							break;
						}
					}
				}
			}	
		}	
	if(rcv!=NULL)
		printfLog(UPLOAD_PROCESS"rcv %s\n\n",rcv);
	else
		printfLog(UPLOAD_PROCESS"no rcv got\n\n");
	}	
	pthread_mutex_unlock(&mutex);
	return;
}

