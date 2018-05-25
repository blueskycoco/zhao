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

void dump_info(char *text)
{
	char *out=NULL;cJSON *item_json;
	int i;

	item_json=cJSON_Parse(text);
	if (!item_json) {printf(LOG_PREFX"Error data before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{

		cJSON *data;
		data=cJSON_GetObjectItem(item_json,"img_height");
		if(data)
			printf("img_height\t %d\n",data->valueint);					
		else
			printf("can not find img_height\n");
		data=cJSON_GetObjectItem(item_json,"img_width");
		if(data)
			printf("img_width\t %d\n",data->valueint);					
		else
			printf("can not find img_width\n");
		data=cJSON_GetObjectItem(item_json,"img_id");
		if(data)
			printf("img_id\t\t %s\n",data->valuestring);					
		else
			printf("can not find img_id\n");
		data=cJSON_GetObjectItem(item_json,"url");
		if(data)
			printf("url\t\t %s\n",data->valuestring);					
		else
			printf("can not find url\n");
		data=cJSON_GetObjectItem(item_json,"face");
		if(data) {
			int face_cnt = cJSON_GetArraySize(data);
			for (i=0; i<face_cnt; i++) {
				printf("FACE[%d]\r\n", i);
				cJSON *face = cJSON_GetArrayItem(data, i);
				
				cJSON *attribute = cJSON_GetObjectItem(face, "attribute");
				cJSON *tmp = cJSON_GetObjectItem(attribute, "age");
				printf("\tage\t\t\t %d\r\n", tmp->valueint);
				tmp = cJSON_GetObjectItem(attribute, "gender");
				printf("\tgender\t\t\t %s\r\n", tmp->valuestring);
				tmp = cJSON_GetObjectItem(attribute, "lefteye_opendegree");
				printf("\tlefteye_opendegree\t %d\r\n", tmp->valueint);
				tmp = cJSON_GetObjectItem(attribute, "mouth_opendegree");
				printf("\tmouth_opendegree\t %d\r\n", tmp->valueint);
				
				tmp = cJSON_GetObjectItem(attribute, "pose");
				cJSON *tmp1 = cJSON_GetObjectItem(tmp, "tilting");
				printf("\ttilting\t\t\t %d\r\n", tmp1->valueint);
				tmp1 = cJSON_GetObjectItem(tmp, "raise");
				printf("\traise\t\t\t %d\r\n", tmp1->valueint);
				tmp1 = cJSON_GetObjectItem(tmp, "turn");
				printf("\tturn\t\t\t %d\r\n", tmp1->valueint);
				
				tmp = cJSON_GetObjectItem(attribute, "righteye_opendegree");
				printf("\trighteye_opendegree\t %d\r\n", tmp->valueint);
				tmp = cJSON_GetObjectItem(face, "face_id");
				printf("\tface_id\t\t\t %s\r\n", tmp->valuestring);
				
				tmp = cJSON_GetObjectItem(face, "position");
				tmp1 = cJSON_GetObjectItem(tmp, "center");
				cJSON *tmp2 = cJSON_GetObjectItem(tmp1, "x");
				printf("\tcenter.x\t\t %f\r\n", tmp2->valuedouble);
				tmp2 = cJSON_GetObjectItem(tmp1, "y");
				printf("\tcenter.y\t\t %f\r\n", tmp2->valuedouble);
				tmp1 = cJSON_GetObjectItem(tmp, "eye_left");
				tmp2 = cJSON_GetObjectItem(tmp1, "x");
				printf("\teye_left.x\t\t %f\r\n", tmp2->valuedouble);
				tmp2 = cJSON_GetObjectItem(tmp1, "y");
				printf("\teye_left.y\t\t %f\r\n", tmp2->valuedouble);
				tmp1 = cJSON_GetObjectItem(tmp, "eye_right");
				tmp2 = cJSON_GetObjectItem(tmp1, "x");
				printf("\teye_right.x\t\t %f\r\n", tmp2->valuedouble);
				tmp2 = cJSON_GetObjectItem(tmp1, "y");
				printf("\teye_right.y\t\t %f\r\n", tmp2->valuedouble);
				
				tmp1 = cJSON_GetObjectItem(tmp, "height");
				printf("\theight\t\t\t %f\r\n", tmp1->valuedouble);
				tmp1 = cJSON_GetObjectItem(tmp, "width");
				printf("\twidth\t\t\t %f\r\n", tmp1->valuedouble);
				tmp = cJSON_GetObjectItem(face, "tag");
				printf("\ttag\t\t %s\r\n", tmp->valuestring);
			}
		}
		else
			printf("can not find face\n");
		cJSON_Delete(item_json);	
	}
}
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
		if (atoi(res_code) == 0) {
			dump_info(rcv);	
		} else
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

