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

int upload_data(char *url,char *appid,char *appkey,char *url2, int timeout)
{
	int result=0;
	char *message=NULL,*rcv=NULL;
	message = (char *)malloc(strlen(url)+strlen(appid)+strlen(appkey)
			+strlen(url2)+32);
	sprintf(message, "%s?app_id=%s&app_key=%s&url=%s",url,appid,appkey,url2);
	printf("<GET> %s\n",message);
	rcv=http_get(message,timeout);

	free(message);
	if (rcv) {
		printf("rcv %s\r\n", rcv);
		free(rcv);
	}
#if 0
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
#endif
	return result;
}

/*get cmd http://api.eyekey.com/face/Check/checking?
  app_id=f89ae61fd63d4a63842277e9144a6bd2&
  amp;app_key=af1cd33549c54b27ae24aeb041865da2&
  url=http%3A%2F%2Fpicview01.baomihua.com%2Fphotos%2F20120713%2
  Fm_14_634778197959062500_40614445.jpg
  */
//ack {"message":"从 url [null] 获取图片出错","res_code":"1011"}
int main(int argc,char *argv[])
{
	if (argc != 5) {
		printf("input param not 5\r\n");
		return;
	}
	
	printf("url:\t\t %s\r\napp_id:\t\t %s\r\napp_key:\t %s\nurl2:\t\t %s\n\n",
		argv[1],argv[2],argv[3],argv[4]);		
	
	if(upload_data(argv[1],argv[2],argv[3],argv[4],9))
		printf("send data ok\n");
	else
		printf("send data failed\n");
	
	return 0;
}

