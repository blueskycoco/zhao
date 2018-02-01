#include "cap.h"
#include "xfer.h"
#include "log.h"
#include "misc.h"
#include "netlib.h"
#include "mycurl.h"
#include <sys/msg.h>
#include <sys/epoll.h>
pthread_mutex_t mutex;
extern struct share_memory *g_share_memory;
#define UPLOAD_PROCESS "[XFER] "

int xfer_init()
{
	pthread_mutex_init(&mutex, NULL);
	return 0;
}
int cap_gprs(int fd, char **out)
{
	int efd,i,j,cnt=0,res=0,ofs=0;
	char buff[1024] = {0};
	struct epoll_event event;
	struct epoll_event *events;
	efd = epoll_create1 (0);
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl (efd, EPOLL_CTL_ADD, fd, &event);
	events = calloc (64, sizeof(event));
	for(;;) {
		int n;
		n = epoll_wait (efd, events, 64, 5000);
		if(n > 0) {
			for (i=0; i<n; i++) {
				if (events[i].data.fd == fd &&
						(events[i].events & EPOLLIN)) {
					int length = read(events[i].data.fd, buff+ofs, sizeof(buff)-ofs);

					if(length > 0) {
						//printf("GPRS read %d bytes\n",length);
						//for(j=0; j<length; j++) {
						//	printf("%c", buff[ofs+j]);
						//}
						//printf("\n");
						//process_serial(buff,length);
						ofs += length;
					}
					break;
				}
			}
			if (strstr(buff, "\"}") != NULL) {
				printfLog(UPLOAD_PROCESS"buff %s\r\n", buff);
				int out_len = strlen(strchr(buff,'{'))+1;
				*out = (char *)malloc(out_len);
				memset(*out,0,out_len);
				memcpy(*out, strchr(buff,'{'), out_len-1);
				printfLog(UPLOAD_PROCESS"out_len %d, out %s\r\n", out_len,*out);
				res =1;
				break;			
			}

			if (strstr(buff, "ok") != NULL) {
				*out=(char *)malloc(3);
				memset(*out,'\0',3);
				strcpy(*out,"ok");
				res=1;
				break;
			}
		} else {
			printfLog(UPLOAD_PROCESS"GPRS No data whthin 5 seconds.\n");
			cnt++;
			if (cnt >= 5)
				break;
		}
	}
	free (events);
	//close (fd);
	if (*out != NULL)
		printfLog(UPLOAD_PROCESS"out %s",*out);
	return res;
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
		//printfLog(UPLOAD_PROCESS"send wifi %s\n",request);
		*out=http_post(url,request,timeout);
		if(*out!=NULL)
		{
			//printfLog(UPLOAD_PROCESS"<==%s\n",*out);
			g_share_memory->network_state=1;
		}
		else
		{
			//printfLog(UPLOAD_PROCESS"<==NULL\n");
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
		strcat(gprs_string,"\n");
#if 0
		for(j=0;j<3;j++){
			printfLog(UPLOAD_PROCESS"send gprs %s\n",gprs_string);
			write(g_share_memory->fd_gprs, gprs_string, strlen(gprs_string));	
			i=0;
			while(1)
			{
				if(read(g_share_memory->fd_gprs, &ch, 1)==1)
				{
					//printfLog(UPLOAD_PROCESS"%c",ch);
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
#else
	printfLog(UPLOAD_PROCESS"send gprs %s\n",gprs_string);
	write(g_share_memory->fd_gprs, gprs_string, strlen(gprs_string));	
	cap_gprs(g_share_memory->fd_gprs, out);
#endif
	}	
	pthread_mutex_unlock(&mutex);
	return;
}

