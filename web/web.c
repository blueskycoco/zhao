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
	if(rcv!=NULL)
	{	
		int len=strlen(rcv);
		printf("<=== %s\n",rcv);
		char *res_code=NULL;
		char *res_message=NULL;
		res_code=doit_data(rcv,(char *)"res_code");
		res_message=doit_data(rcv,(char *)"message");
		printf("res_code\t %s\r\n",res_code);
		printf("message\t\t %s\r\n", res_message);
		if(res_code!=NULL)
		{
			free(res_code);
		}
		if(res_message!=NULL)
		{
			free(res_message);
		}
		result=1;
		free(rcv);
	}
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
		return -1;
	}

	printf("url:\t\t %s\r\napp_id:\t\t %s\r\napp_key:\t %s\nurl2:\t\t %s\n\n",
			argv[1],argv[2],argv[3],argv[4]);		

	if(upload_data(argv[1],argv[2],argv[3],argv[4],99))
		printf("xfer data ok\n");
	else
		printf("xfer data failed\n");

	return 0;
}

