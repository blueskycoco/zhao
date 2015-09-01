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

//get cmd http://101.200.236.69:8080/lamp/lamp/commond/wait?lampCode=aaaa
//ack http://101.200.236.69:8080/lamp/lamp/commond/response?commondId=f1d51484-8daf-47a3-a490-f56461d3ce23&isSuccess=true
int main(int argc,char *argv[])
{
	char *res=NULL;
	char *rcv=http_get(argv[2]/*"http://101.200.236.69:8080/lamp/lamp/commond/wait?lampCode=aaaa"*/,atoi(argv[1]));
	if(rcv!=NULL)
	{
		printf("%s\n",rcv);
		res=doit(rcv,"code");
		if(res)
		{
			printf(LOG_PREFX"code is %s\n",res);
			free(res);
		}
		free(rcv);
	}
	rcv=http_get("http://101.200.236.69:8080/lamp/lamp/commond/response?commondId=f1d51484-8daf-47a3-a490-f56461d3ce23&isSuccess=true",atoi(argv[1]));
	if(rcv!=NULL)
	{
		printf("%s\n",rcv);
		res=doit(rcv,"errorMsg");
		if(res)
		{
			printf(LOG_PREFX"errorMsg is %s\n",res);
			free(res);
		}
		free(rcv);
	}
	//rcv=http_post("http://101.200.236.69:8080/lamp/device/register","macAddress=xxxx");
	//printf("%s\n",rcv);
	//free(rcv);
	return 0;
	//return htpp_get(argc,argv);
}

