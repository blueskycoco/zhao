#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>
#include <time.h>
#include <string.h>  
#include <errno.h>  
#include <sys/msg.h>  
#include <signal.h>
#include <fnmatch.h> 
#include <termios.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
//#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#define ETH_NAME "ra0"
//#include <linux/sockios.h>
#include <ifaddrs.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fnmatch.h> 
#include <signal.h>
#include <sys/ipc.h>  
#include <sys/shm.h>  
#include <linux/rtc.h>
#include <sys/time.h>
#include "cJSON.h"
#include "weblib.h"
#include "web_interface.h"
//#include <iostream>
#include <unistd.h>

#define RTCDEV "/dev/rtc0"
#define START_BYTE 0x6C
#define CAP_TO_ARM 0xAA
#define ARM_TO_CAP 0xBB
#define VERIFY_BYTE	0x0003
#define RESEND_BYTE	0x0002
#define TIME_BYTE	0x0001
#define ERROR_BYTE	0xFF
#define URL "http://101.200.182.92:8080/saveData/airmessage/messMgr.do"
//#define URL "http://16.168.0.3:8080/saveData/airmessage/messMgr.do"
#define FILE_PATH	"/home/user/history/"
#define MAIN_PROCESS 						"[MainSystem]:"
#define SUB_PROCESS 						"[ChildSystem]:"
//char server_time[20]={0};
void set_time(int year,int mon,int day,int hour,int minute,int second);
void write_data(int fd,unsigned int Index,int data);
void switch_pic(int fd,unsigned char Index);
void dump_curr_time(int fd);
void save_sensor_alarm_info();
int g_upload=0;
char ip[20]={0};
char cur_date[15]={0};
pthread_mutex_t mutex;
char *post_message=NULL,can_send=0,*warnning_msg=NULL;
char *server_time;
char g_uuid[256]={0};
char send_by_wifi=1;
int fd_gprs=0;
#define ALARM_NONE		0
#define ALARM_BELOW 	1
#define ALARM_UP		2
#define ALARM_UNINSERT 	4

#define CAP_PROCESS "[CAP_PROCESS]"
#define STATE_IDLE 	0
#define STATE_6C 	1
#define STATE_AA 	2
#define STATE_MESSAGE_TYPE 3
#define STATE_MESSAGE_LEN 4
#define STATE_MESSAGE 5
#define STATE_CRC 6
typedef struct _sensor_alarm {
	char co;
	char co2;
	char hcho;
	char shidu;
	char temp;
	char pm25;
	char co_send;
	char co2_send;
	char hcho_send;
	char shidu_send;
	char temp_send;
	char pm25_send;
}sensor_alarm;

typedef struct _sensor_times {
	char co;
	char co2;
	char hcho;
	char shidu;
	char temp;
	char pm25;
}sensor_times;
#define CONFIG_FILE "sensor_alarm.cfg"
sensor_alarm sensor;
sensor_times sensortimes;
char history_co_time[100000][20];
char history_co_data[100000][10];
long g_history_co_cnt=0;
char history_co2_time[100000][20];
char history_co2_data[100000][10];
long g_history_co2_cnt=0;
char history_hcho_time[100000][20];
char history_hcho_data[100000][10];
long g_history_hcho_cnt=0;
char history_shidu_time[100000][20];
char history_shidu_data[100000][10];
long g_history_shidu_cnt=0;
char history_temp_time[100000][20];
char history_temp_data[100000][10];
long g_history_temp_cnt=0;
char history_pm25_time[100000][20];
char history_pm25_data[100000][10];
long g_history_pm25_cnt=0;
//*******************************************************************
//
// 名称: CRC_check
// 说明: 
// 功能: 16位CRC校验
// 调用: 无
// 输入: Data 校验数据，Data_length 数据长度
// 返回值: CRC 校验结果
// ***************************************************************************
unsigned int CRC_check(unsigned char *Data,unsigned char Data_length)
{
	unsigned int mid=0;
	unsigned char times=0,Data_index=0;
	unsigned int CRC=0xFFFF;
	while(Data_length)
	{
		CRC=Data[Data_index]^CRC;
		for(times=0;times<8;times++)
		{
			mid=CRC;
			CRC=CRC>>1;
			if(mid & 0x0001)
			{
				CRC=CRC^0xA001;
			}
		}
		Data_index++;
		Data_length--;
	}
	return CRC;
}
#define UPLOAD_PROCESS "[UPLOAD_PROCESS]"
void send_web_post(char *url,char *buf,int timeout,char **out)
{
	char request[1024]={0};
	char length_string[30]={0};
	int result=0,i,ltimeout=0;
	char rcv[512]={0},ch;
	pthread_mutex_lock(&mutex);
	if(send_by_wifi)
	{		
		sprintf(request,"JSONStr=%s",buf);
		printf(UPLOAD_PROCESS"send web %s\n",request);
		*out=http_post(url,request,timeout);
		if(*out!=NULL)
			printf(UPLOAD_PROCESS"<==%s\n",*out);
		else
			printf(UPLOAD_PROCESS"<==NULL\n");
	}
	else
	{
		//rcv=(char *)malloc(256);
		//memset(rcv,'\0',256);
		//char *gprs_string=(char *)malloc(strlen(buf)+strlen("POST /saveData/airmessage/messMgr.do HTTP/1.1\r\nHOST: 101.200.182.92:8080\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\n")+30);
		//memset(gprs_string,'\0',strlen(buf)+strlen("POST /saveData/airmessage/messMgr.do HTTP/1.1\r\nHOST: 101.200.182.92:8080\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\n")+30);
		char gprs_string[1024]={0};
		int j=0;
		strcpy(gprs_string,"POST /saveData/airmessage/messMgr.do HTTP/1.1\r\nHOST: 101.200.182.92:8080\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\n");
		sprintf(length_string,"Content-Length:%d\r\n\r\nJSONStr=",strlen(buf)+8);
		strcat(gprs_string,length_string);
		strcat(gprs_string,buf);
		for(j=0;j<3;j++){
			printf(UPLOAD_PROCESS"send gprs %s\n",gprs_string);
			write(fd_gprs, gprs_string, strlen(gprs_string));	
			i=0;
			while(1)
			{
				if(read(fd_gprs, &ch, 1)==1)
				{
					printf("%c",ch);
					if(ch=='}')
					{
						rcv[i++]=ch;
						*out=(char *)malloc(i+1);
						memset(*out,'\0',i+1);
						memcpy(*out,rcv,i);//strcpy(*out,rcv);
						pthread_mutex_unlock(&mutex);
						return;
					}
					else if(ch=='{')
						i=0;
					else if(ch=='o')
					{
						if(read(fd_gprs,&ch,1)==1)
							if(ch=='k')
							{
								*out=(char *)malloc(3);
								memset(*out,'\0',3);
								strcpy(*out,"ok");
								memset(rcv,'\0',512);
								strcpy(rcv,"ok");
								pthread_mutex_unlock(&mutex);
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
							printf("gprs timeout %d\n",j);						
							*out=NULL;
							ltimeout=0;
							if(j==2)
							{
								pthread_mutex_unlock(&mutex);
								return;
							}
							break;
						}
					}
				}
			}	
		}	
		//free(gprs_string);
	if(rcv!=NULL)
		printf(UPLOAD_PROCESS"rcv %s\n\n",rcv);
	else
		printf(UPLOAD_PROCESS"no rcv got\n\n");
	}
	
	pthread_mutex_unlock(&mutex);
	return;
}
char *send_web_get(char *url,char *buf,int timeout)
{
	char request[1024]={0};
	int result=0;
	char *rcv=NULL;
#if 1
	sprintf(request,"%s?JSONStr=%s",url,buf);
	printf(UPLOAD_PROCESS"send web %s\n",request);
	rcv=http_get(request,timeout);
#else
	sprintf(request,"JSONStr=%s",buf);
	printf(LOG_PREFX"send web %s\n",request);
	rcv=http_post(url,request,timeout);
#endif
	if(rcv!=NULL)
		printf(UPLOAD_PROCESS"rcv %s\n",rcv);
	else
		printf(UPLOAD_PROCESS"no rcv got\n");
	return rcv;
}
int GetIP_v4_and_v6_linux(int family,char *address,int size)
{
	struct ifaddrs *ifap0,*ifap;
	char buf[NI_MAXHOST];
	//char *interface = "ra0";
	struct sockaddr_in *addr4;
	struct sockaddr_in6 *addr6;
	int ret;
	if(NULL == address)
	{
		printf("in address");  
		return -1;
	}
	if(getifaddrs(&ifap0))
	{
		printf("getifaddrs error");  
		return -1;
	}
	for(ifap = ifap0;ifap!=NULL;ifap=ifap->ifa_next)
	{


		if(strcmp(ETH_NAME,ifap->ifa_name)!=0) continue; 
		if(ifap->ifa_addr == NULL) continue;
		if((ifap->ifa_flags & IFF_UP) == 0) continue;
		if(family!=ifap->ifa_addr->sa_family) continue;

		if(AF_INET == ifap->ifa_addr->sa_family)
		{ 

			addr4 = (struct sockaddr_in *)ifap->ifa_addr;
			if(NULL != inet_ntop(ifap->ifa_addr->sa_family,(void *)&(addr4->sin_addr),buf,NI_MAXHOST))
			{
				if(size <=strlen(buf)) break;
				strcpy(address,buf);
				printf("address %s\n",address);
				freeifaddrs(ifap0);
				return 0;
			}
			else 
			{
				printf("inet_ntop error\r\n");
				break;  
			}
		}
		else if(AF_INET6 == ifap->ifa_addr->sa_family)
		{
			addr6 = (struct sockaddr_in6*) ifap->ifa_addr;
			if(IN6_IS_ADDR_MULTICAST(&addr6->sin6_addr))
			{
				continue;
			}
			if(IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr))
			{
				continue;
			}
			if(IN6_IS_ADDR_LOOPBACK(&addr6->sin6_addr))
			{
				continue;
			}
			if(IN6_IS_ADDR_UNSPECIFIED(&addr6->sin6_addr))
			{
				continue;
			}
			if(IN6_IS_ADDR_SITELOCAL(&addr6->sin6_addr))
			{
				continue;
			}
			if(NULL != inet_ntop(ifap->ifa_addr->sa_family,(void *)&(addr6->sin6_addr),buf,NI_MAXHOST))
			{
				if(size <= strlen(buf)) break;
				strcpy(address,buf);
				freeifaddrs(ifap0);
				return 0;
			}
			else break; 
		} 
	}
	freeifaddrs(ifap0);
	return -1;
}
void save_to_file(char *date,char *message)
{
	FILE *fp;
	char file_path[256]={0};
	char data[512]={0};
	strcpy(file_path,FILE_PATH);
	memcpy(file_path+strlen(FILE_PATH),date,10);
	strcat(file_path,".dat");
	fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		fp=fopen(file_path,"w");
		if(fp==NULL)
		{
			printf("can not create %s\r\n",file_path);
			return;
		}	
	}
	else
	{
		fclose(fp);
		fp=fopen(file_path, "a");
	}
	strcpy(data,date+11);
	//strcpy(data,date+13);
	strcat(data,"\n");
	fwrite(data,strlen(data),1,fp);
	memset(data,'\0',512);
	strcpy(data,message);
	strcat(data,"\n");
	fwrite(data,strlen(data),1,fp);
	fclose(fp);
}
void get_uuid()
{
	FILE *fp=fopen("/home/user/uuid.txt","r");
	if(fp!=NULL)
	{
		if(fread(g_uuid,256,1,fp)<0)
			strcpy(g_uuid,"1234abcd");
		fclose(fp);
	}
	else
		strcpy(g_uuid,"1234abcd");
	g_uuid[strlen(g_uuid)-1]='\0';
	printf("uuid is %s\n",g_uuid);
}
#define RESEND_PROCESS "[RESEND_PROCESS]"
void resend_history_done(char *begin,char *end)
{
	char *resend_done=NULL;						
	resend_done=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_RE_DATA);
	resend_done=add_item(resend_done,ID_DEVICE_UID,g_uuid);
	resend_done=add_item(resend_done,ID_DEVICE_IP_ADDR,ip);
	resend_done=add_item(resend_done,ID_DEVICE_PORT,"9517");
	resend_done=add_item(resend_done,ID_RE_START_TIME,begin);
	resend_done=add_item(resend_done,ID_RE_STOP_TIME,end);
	char *rcv=NULL;
	send_web_post(URL,resend_done,9,&rcv);
	free(resend_done);
	resend_done=NULL;
	if(rcv!=NULL)
	{	
		int len=strlen(rcv);
		free(rcv);
		rcv=NULL;
	}	
}
void resend_history(char *date_begin,char *date_end)
{
	FILE *fp;
	int cnt=0,month_b,year_b,day_b,month_e,year_e,day_e,hour_e,minute_e,max_day;
	char year_begin[5]={0};
	char year_end[5]={0};
	char month_begin[3]={0};
	char month_end[3]={0};
	char day_begin[3]={0};
	char day_end[3]={0};
	char hour_end[3]={0};
	char minute_end[3]={0};
	char file_path[256]={0};
	char data[512]={0};
	char date[32]={0};
	memcpy(year_begin,date_begin,4);
	memcpy(year_end,date_end,4);
	memcpy(month_begin,date_begin+5,2);
	memcpy(month_end,date_end+5,2);
	memcpy(day_begin,date_begin+8,2);
	memcpy(day_end,date_end+8,2);
	memcpy(hour_end,date_end+11,2);
	memcpy(minute_end,date_end+14,2);
	month_b=atoi(month_begin);
	year_b=atoi(year_begin);
	day_b=atoi(day_begin);
	month_e=atoi(month_end);
	year_e=atoi(year_end);
	day_e=atoi(day_end);
	hour_e=atoi(hour_end);
	minute_e=atoi(minute_end);
	printf(RESEND_PROCESS"year_b %04d,month_b %02d,day_b %02d,year_e %04d,month_e %02d,day_e %02d\r\n",year_b,month_b,day_b,year_e,month_e,day_e);
	while(1)
	{
		if(year_b<year_e || month_b<month_e || day_b<=day_e)
		{
			memset(file_path,'\0',256);
			memset(date,'\0',32);
			sprintf(date,"%04d-%02d-%02d",year_b,month_b,day_b);
			strcpy(file_path,FILE_PATH);
			memcpy(file_path+strlen(FILE_PATH),date,10);
			strcat(file_path,".dat");
			printf(RESEND_PROCESS"to open %s\r\n",file_path);
			fp = fopen(file_path, "r");
			if (fp != NULL)
			{
				int read=0,tmp_i=0;
				char * line = NULL;
				size_t len = 0;
				printf(RESEND_PROCESS"open file %s ok\r\n",file_path);
				while ((read = getline(&line, &len, fp)) != -1) 
				{	
					line[6]='3';//change resend history type from 2 to 3
					if(year_b==year_e && month_b==month_e && day_b==day_e)
					{//check time in file
						if((tmp_i%2)==0)
						{							
							char local_hour[3]={0},local_minute[3]={0};
							memcpy(local_hour,line,2);
							memcpy(local_minute,line+3,2);
							if((atoi(local_hour)*60+atoi(local_minute))>(hour_e*60+minute_e))
							{
								printf(RESEND_PROCESS"file_time %02d:%02d,end time %02d:%02d",atoi(local_hour),atoi(local_minute),hour_e,minute_e);
								free(line);
								fclose(fp);
								resend_history_done(date_begin,date_end);
								return;
							}
						}
						else
						{
							line[strlen(line)-1]='\0';							
							printf(RESEND_PROCESS"[rsend web]\n");
							while(1){
								char *rcv=NULL;
								send_web_post(URL,line,39,&rcv);
								if(rcv!=NULL)
								{	
									int len1=strlen(rcv);
									//printf(MAIN_PROCESS"<=== %s %d\n",rcv,len1);
									//printf(MAIN_PROCESS"send ok\n");
									if(strncmp(rcv,"ok",2)==0)
									{
										free(rcv);
										break;
									}
									free(rcv);
									rcv=NULL;
								}
							}
						}
					}
					else
					{
						if((tmp_i%2)!=0)
						{						
							line[strlen(line)-1]='\0';
							printf(RESEND_PROCESS"[rsend web]\n");
							while(1){
								char *rcv=NULL;
								send_web_post(URL,line,9,&rcv);
								if(rcv!=NULL)
								{	
									int len1=strlen(rcv);
									//printf(MAIN_PROCESS"<=== %s %d\n",rcv,len1);
									//printf(MAIN_PROCESS"send ok\n");
									if(strncmp(rcv,"ok",2)==0)
									{
										free(rcv);
										break;
									}
									free(rcv);
									rcv=NULL;
								}
							}
						}
					}
					tmp_i++;
				}
				free(line);

			}
			else
			{
				printf(RESEND_PROCESS"can not open %s\r\n",file_path);
				//break;
			}
			if(month_b==2)
				max_day=28;
			else if(month_b==1||month_b==3||month_b==5||month_b==7||month_b==8||month_b==10||month_b==12)
				max_day=31;
			else
				max_day=30;
			if(day_b==max_day)
			{
				if(month_b==12)
				{
					year_b++;
					month_b=0;
				}
				else
					month_b++;
				day_b=0;
			}
			else
				day_b++;			
		}
		else
		{
			printf(RESEND_PROCESS"end year_b %04d,month_b %02d,day_b %02d,year_e %04d,month_e %02d,day_e %02d\r\n",year_b,month_b,day_b,year_e,month_e,day_e);
			break;
		}
	}
	if(fp!=NULL)
		fclose(fp);
	resend_history_done(date_begin,date_end);
}
#define SYNC_PREFX "[SYNC_PROCESS]"
void sync_server(int fd,int resend)
{
	int i,j;
	char text_out[512]={0};
	char *sync_message=NULL,*rcv=NULL;
	if(resend)
		sync_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_ASK_RE_DATA);
	else
		sync_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_SYNC);
	sync_message=add_item(sync_message,ID_DEVICE_UID,g_uuid);
	sync_message=add_item(sync_message,ID_DEVICE_IP_ADDR,ip);
	sync_message=add_item(sync_message,ID_DEVICE_PORT,"9517");
	printf(SYNC_PREFX"<sync GET>%s\n",sync_message);
#if 0
	j=0;
	for(i=0;i<strlen(sync_message);i++)
	{
		if(sync_message[i]=='\n'||sync_message[i]=='\r'||sync_message[i]=='\t')
			j++;
	}
	char *out1=malloc(strlen(sync_message)-j+1);
	memset(out1,'\0',strlen(sync_message)-j+1);
	j=0;
	for(i=0;i<strlen(sync_message);i++)
	{
		if(sync_message[i]!='\r'&&sync_message[i]!='\n'&&sync_message[i]!='\t')		
		{
			out1[j++]=sync_message[i];
		}
	}
#endif
	send_web_post(URL,sync_message,9,&rcv);
	free(sync_message);
	//free(out1);
	if(rcv!=NULL&&strlen(rcv)!=0)
	{	
		int len=strlen(rcv);
		printf(SYNC_PREFX"<=== %s\n",rcv);
		printf(SYNC_PREFX"send ok\n");
		char *starttime=NULL;
		char *tmp=NULL;
		if(resend)
		{

			starttime=doit_data(rcv,(char *)"101");
			tmp=doit_data(rcv,(char *)"102");
			if(starttime!=NULL && tmp!=NULL)
			{
				printf(SYNC_PREFX"%s\r\n",tmp);
				printf(SYNC_PREFX"%s\r\n",starttime);
				resend_history(starttime,tmp);
				free(starttime);
				free(tmp);
			}
		}
		else
		{
			//strcpy(rcv,"{\"30\":\"1234abcd\",\"210\":\"2015-08-27 14:43:57.0\",\"211\":\"???,????,???,313131\",\"212\":\"??\",\"213\":\"??\",\"104\":\"2015-09-18 11:53:58\",\"201\":[],\"202\":[]}");
			//if(atoi(type)==5)
			//{
			char year[3]={0},month[3]={0},day[3]={0},hour[3]={0},minute[3]={0},second[3]={0};
			unsigned int crc=0;
			starttime=doit_data(rcv,(char *)"104");
			if(starttime!=NULL){
				server_time[0]=0x6c;server_time[1]=ARM_TO_CAP;
				server_time[2]=0x00;server_time[3]=0x01;server_time[4]=0x06;
				memcpy(year,starttime+2,2);
				memcpy(month,starttime+5,2);
				memcpy(day,starttime+8,2);
				memcpy(hour,starttime+11,2);
				memcpy(minute,starttime+14,2);
				memcpy(second,starttime+17,2);
				server_time[5]=atoi(year);server_time[6]=atoi(month);
				server_time[7]=atoi(day);server_time[8]=atoi(hour);
				server_time[9]=atoi(minute);server_time[10]=atoi(second);
				crc=CRC_check(server_time,11);
				server_time[11]=(crc&0xff00)>>8;server_time[12]=crc&0x00ff;
				write(fd,server_time,13);
				printf(SYNC_PREFX"SERVER TIME %s\r\n",starttime);
				//tmp=doit_data(rcv+4,(char *)"211");
				printf(SYNC_PREFX"211 %s\r\n",doit_data(rcv,"211"));
				printf(SYNC_PREFX"212 %s\r\n",doit_data(rcv,"212"));
				set_time(server_time[5]+2000,server_time[6],server_time[7],server_time[8],server_time[9],server_time[10]);
			}
			//else if(atoi(type)==6)
			//{
			//}
		}
		free(rcv);
		rcv=NULL;
	}
		return ;
}

void get_ip(char *ip)
{
	GetIP_v4_and_v6_linux(AF_INET,ip,16);
	printf("ip addrss %s\n", ip);
	return ;
}
#if 0
int get_uart(int fd_lcd,int fd)
{
	char *rcv=NULL,ch,state=STATE_IDLE,message_len=0;	
	char id[32]={0},data[32]={0},date[32]={0},error[32]={0};
	char message[10],i=0,to_check[20];
	struct timeval time1;
	int crc,j,message_type=0;
	fd_set fs_read;
	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);
	time1.tv_sec = 10;
	time1.tv_usec = 0;
	while(1)
	{
		//if(select(fd+1,&fs_read,NULL,NULL,&time1)>0)
		{
			if(read(fd,&ch,1)==1)
			{
				switch (state)
				{
					case STATE_IDLE:
						{
							if(ch==0x6c)
								state=STATE_6C;
						}
						break;
					case STATE_6C:
						{
							if(ch==0xaa)
							{
								state=STATE_AA;
								i=0;
							}						
						}
						break;
					case STATE_AA:
						{
							message_type=ch<<8;
							//printf("Get AA ==> %02x %02x",ch,message_type);
							i=0;
							state=STATE_MESSAGE_TYPE;
						}
						break;
					case STATE_MESSAGE_TYPE:
						{
							message_type|=ch;
							state=STATE_MESSAGE_LEN;
						}
						break;
					case STATE_MESSAGE_LEN:
						{
							message_len=ch;
							state=STATE_MESSAGE;
							i=0;
						}
						break;
					case STATE_MESSAGE:
						{
							if(i!=message_len)
							{
								message[i++]=ch;
							}
							else
							{
								state=STATE_CRC;
								crc=ch<<8;
								//printf(SUB_PROCESS"crc1 %02x\n",ch);
							}	
						}
						break;
					case STATE_CRC:
						{
							crc|=ch;
							//printf(SUB_PROCESS"crc2 %02x\n",ch);
							//printf(SUB_PROCESS"GOT 0x6c 0xaa %04x %02x ",message_type,message_len);
							for(i=0;i<message_len;i++)
							{
								//printf("%02x ",message[i]);
								to_check[5+i]=message[i];
							}
							//printf("%04x \r\n",crc);
							to_check[0]=0x6c;to_check[1]=0xaa;to_check[2]=(message_type>>8)&0xff;to_check[3]=message_type&0xff;
							to_check[4]=message_len;to_check[5+message_len]=(crc>>8)&0xff;
							to_check[5+message_len+1]=crc&0xff;
							//printf(SUB_PROCESS"CRC Get %02x <> Count %02x\r\n",crc,CRC_check(to_check,message_len+5));
							if(crc==CRC_check(to_check,message_len+5))
							{

								i=0;
								memset(id,'\0',sizeof(id));
								memset(data,'\0',sizeof(data));
								memset(date,'\0',sizeof(date));
								memset(error,'\0',sizeof(error));
								switch(message_type)
								{
									case TIME_BYTE:
										{
											sprintf(date,"20%02d-%02d-%02d %02d:%02d",to_check[i+5],to_check[i+6],to_check[i+7],to_check[i+8],to_check[i+9],to_check[i+10]);
											printf(CAP_PROCESS"date is %s\r\n",date);
											post_message=add_item(post_message,ID_DEVICE_CAP_TIME,date);
											if(warnning_msg!=NULL)
											{
												warnning_msg=add_item(warnning_msg,ID_DEVICE_CAP_TIME,date);
												send_web_post(URL,warnning_msg,9,&rcv);
												free(warnning_msg);
												warnning_msg=NULL;
												if(rcv!=NULL)
												{	
													int len=strlen(rcv);
													free(rcv);
													rcv=NULL;
												}	
											}
											can_send=1;
										}
										break;
									case ERROR_BYTE:
										{
											sprintf(error,"%dth sensor possible error",to_check[i+2]);
											post_message=add_item(post_message,ID_ALERT_CAP_FAILED,error);
										}
										break;
									case RESEND_BYTE:
										{
											write(fd,server_time,13);
										}
										break;
									default:
										{
											/*get cap data*/
											if(to_check[i+5]==0x65 && to_check[i+6]==0x72 && to_check[i+7]==0x72 && to_check[i+8]==0x6f && to_check[i+9]==0x72)
											{
												if(to_check[i+3]==atoi(ID_CAP_CO2) && !sensor.co2)
													sensor.co2=ALARM_UNINSERT;
												else if(to_check[i+3]==atoi(ID_CAP_CO) && !sensor.co)
													sensor.co=ALARM_UNINSERT;
												else if(to_check[i+3]==atoi(ID_CAP_HCHO) && !sensor.hcho)
													sensor.hcho=ALARM_UNINSERT;
												else if(to_check[i+3]==atoi(ID_CAP_SHI_DU) && !sensor.shidu)
													sensor.shidu=ALARM_UNINSERT;
												else if(to_check[i+3]==atoi(ID_CAP_TEMPERATURE)&& !sensor.temp)
													sensor.temp=ALARM_UNINSERT;
												else if(to_check[i+3]==atoi(ID_CAP_PM_25)&& !sensor.pm25)
													sensor.pm25=ALARM_UNINSERT;
												else 
													continue;
												//inform server
												sprintf(error,"%dth sensor possible error",to_check[i+3]);
												//char *warnning_msg=NULL;				
												sprintf(id,"%d",to_check[i+3]);
												warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
												warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
												warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
												warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
												warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
												warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,id);
												warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UNINSERT);
												//sprintf(date,"20%02d-%02d-%02d %02d:%02d",to_check[i+5],to_check[i+6],to_check[i+7],to_check[i+8],to_check[i+9],to_check[i+10]);
												//printf(CAP_PROCESS"date is %s\r\n",date);
												//warnning_msg=add_item(warnning_msg,ID_DEVICE_CAP_TIME,date);
												//rcv=send_web_post(URL,warnning_msg,9);
												//free(warnning_msg);
												//warnning_msg=NULL;
												//if(rcv!=NULL)
												//{	
												//	int len=strlen(rcv);
												//	free(rcv);
												//}	
												return 0;
											}
											else
											{
												float value=0;
												if(post_message==NULL)
												{
													post_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
													post_message=add_item(post_message,ID_DEVICE_UID,g_uuid);
													post_message=add_item(post_message,ID_DEVICE_IP_ADDR,ip);
													post_message=add_item(post_message,ID_DEVICE_PORT,"9517");	
												}
												sprintf(id,"%d",message_type);
												sprintf(data,"%d",to_check[i+5]<<8|to_check[i+6]);						
												if(to_check[i+7]!=0)
												{//have .
													int m;
													if(to_check[i+7]>strlen(data))//like 0.012
													{
														char tmp_buf[10]={0};
														int dist=to_check[i+7]-strlen(data);
														strcpy(tmp_buf,"0.");
														for(m=0;m<dist;m++)
															strcat(tmp_buf,"0");
														strcat(tmp_buf,data);
														strcpy(data,tmp_buf);
													}
													else//like 12.33
													{
														int left,right,number,n=1;
														number=(to_check[i+5]<<8)|to_check[i+6];
														for(m=0;m<to_check[i+7];m++)
															n=n*10;
														right=number%n;
														left=number/n;
														sprintf(data,"%d.%d",left,right);								
													}
													#if 0
													if(g_upload){
														int temp=1;
														for(i=0;i<to_check[i+7];i++)
															temp*=10;
														value=((float)(to_check[i+5]<<8|to_check[i+6]))/((float)temp);
													}
													#endif
												}	
												else
												{
													#if 0
													if(g_upload)
														value=(float)(to_check[i+5]<<8|to_check[i+6]);
													#endif
												}
												#if 0
												if(g_upload)
												{
													if(message_type==ID_CAP_CO2 && !sensor.co2)
													{
														if(value<MIN_CO2)
															sensortimes.co2++;
														else if(value>MAX_CO2)
															sensortimes.co2++;
														else
															sensortimes.co2=0;
														if(sensortimes.co2==12)
														{//need send server alarm

														}
													}
													else if(to_check[i+3]==ID_CAP_CO && !sensor.co)
														sensor.co=ALARM_UNINSERT;
													else if(to_check[i+3]==ID_CAP_HCHO && !sensor.hcho)
														sensor.hcho=ALARM_UNINSERT;
													else if(to_check[i+3]==ID_CAP_SHI_DU && !sensor.shidu)
														sensor.shidu=ALARM_UNINSERT;
													else if(to_check[i+3]==ID_CAP_TEMPERATURE&& !sensor.temp)
														sensor.temp=ALARM_UNINSERT;
													else if(to_check[i+3]==ID_CAP_PM_25&& !sensor.pm25)
														sensor.pm25=ALARM_UNINSERT;
													else 
														continue;
												}
												#endif
												//real time update cap data
												if(strncmp(id,ID_CAP_CO,strlen(ID_CAP_CO))==0)
												{
													write_data(fd_lcd,VAR_DATE_TIME_1,to_check[i+5]<<8|to_check[i+6]);
												}
												else if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
												{						
													write_data(fd_lcd,VAR_DATE_TIME_2,to_check[i+5]<<8|to_check[i+6]);
												}
												else if(strncmp(id,ID_CAP_HCHO,strlen(ID_CAP_HCHO))==0)
												{
													write_data(fd_lcd,VAR_DATE_TIME_3,to_check[i+5]<<8|to_check[i+6]);
												}
												else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(ID_CAP_TEMPERATURE))==0)
												{
													write_data(fd_lcd,VAR_DATE_TIME_4,to_check[i+5]<<8|to_check[i+6]);
												}
												else if(strncmp(id,ID_CAP_SHI_DU,strlen(ID_CAP_SHI_DU))==0)
												{
													write_data(fd_lcd,VAR_ALARM_TYPE_1,to_check[i+5]<<8|to_check[i+6]);
												}
												else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
												{
													write_data(fd_lcd,VAR_ALARM_TYPE_2,to_check[i+5]<<8|to_check[i+6]);
												}
												post_message=add_item(post_message,id,data);
												//printf(SUB_PROCESS"id %s data %s\r\n==>\n%s\n",id,data,post_message);
											}
										}
										break;
								}
							}
							else
							{
								printf(CAP_PROCESS"CRC error 0x%04X\r\n",CRC_check(to_check,message_len+5));
								for(i=0;i<message_len+7;i++)
									printf("0x%02x ",to_check[i]);
							}					
							if(can_send)
							{
								can_send=0;
								if(g_upload)
								{
									save_to_file(date,post_message);
									g_upload=0;
									//printf(SUB_PROCESS"send web %s",post_message);
									send_web_post(URL,post_message,9,&rcv);
									if(rcv!=NULL)
									{	
										int len=strlen(rcv);
										//printf(SUB_PROCESS"<=== %s %d\n",rcv,len);
										//printf(SUB_PROCESS"send ok\n");
										free(rcv);
										rcv=NULL;
									}			
								}
								free(post_message);
								post_message=NULL;
							}
							return 0;						
						}
					default:
						{
							i=0;
							state=STATE_IDLE;
						}	
				}
			}
		}
	}
}
#else
/*
  *save message to local file
  *send message to server
*/
void send_server_save_local(char *date,char *message,char save)
{
	char *rcv = NULL;
	if(save)
	{
		save_to_file(date,message);
		char *data=doit_data(message,ID_CAP_CO);
		if(data!=NULL)
		{
			memset(history_co_data[g_history_co_cnt],'\0',10);
			strcpy(history_co_data[g_history_co_cnt],data);					
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			g_history_co_cnt++;
		}
		data=doit_data(message,ID_CAP_CO2);
		if(data!=NULL)
		{
			memset(history_co2_data[g_history_co2_cnt],'\0',10);
			strcpy(history_co2_data[g_history_co2_cnt],data);					
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			g_history_co2_cnt++;
		}
		data=doit_data(message,ID_CAP_SHI_DU);
		if(data!=NULL)
		{
			memset(history_shidu_data[g_history_shidu_cnt],'\0',10);
			strcpy(history_shidu_data[g_history_shidu_cnt],data);					
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			g_history_shidu_cnt++;
		}
		data=doit_data(message,ID_CAP_HCHO);
		if(data!=NULL)
		{
			memset(history_hcho_data[g_history_hcho_cnt],'\0',10);
			strcpy(history_hcho_data[g_history_hcho_cnt],data);					
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			g_history_hcho_cnt++;
		}
		data=doit_data(message,ID_CAP_TEMPERATURE);
		if(data!=NULL)
		{
			memset(history_temp_data[g_history_temp_cnt],'\0',10);
			strcpy(history_temp_data[g_history_temp_cnt],data);					
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			g_history_temp_cnt++;
		}
		data=doit_data(message,ID_CAP_PM_25);
		if(data!=NULL)
		{
			memset(history_pm25_data[g_history_pm25_cnt],'\0',10);
			strcpy(history_pm25_data[g_history_pm25_cnt],data);					
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			g_history_pm25_cnt++;
		}
	}
	send_web_post(URL,message,9,&rcv);
	if(rcv != NULL)
	{	
		int len = strlen(rcv);
		free(rcv);
		rcv=NULL;
	}			
}
void clear_alarm(char *id,char *alarm_type)
{
	char *clear_msg=NULL;
	clear_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_CLEAR_WARNING);
	clear_msg=add_item(clear_msg,ID_DEVICE_UID,g_uuid);
	clear_msg=add_item(clear_msg,ID_DEVICE_IP_ADDR,ip);
	clear_msg=add_item(clear_msg,ID_DEVICE_PORT,"9517");						
	clear_msg=add_item(clear_msg,ID_ALARM_SENSOR,id);
	clear_msg=add_item(clear_msg,ID_ALARM_TYPE,alarm_type);
	send_server_save_local(NULL,clear_msg,0);
	free(clear_msg);
}
/*
  *build json from cmd
  *update cap data to lcd
  *create alarm and normal message to server
*/
char *build_message(int fd,int fd_lcd,char *cmd,int len,char *message)
{	
	int i=0;
	char id[32]={0},data[32]={0},date[32]={0},error[32]={0};
	unsigned int crc=(cmd[len-2]<<8)|cmd[len-1];
	int message_type=(cmd[2]<<8)|cmd[3];
	sprintf(id,"%d",message_type);
	//printf("crc 0x%04X,message_type %d,len %d \n",crc,message_type,len);
	if(crc==CRC_check(cmd,len-2))
	{
		i=0;
		switch(message_type)
		{
			case TIME_BYTE:
			{	//TIME_BYTE got ,we can send to server now
				sprintf(date,"20%02d-%02d-%02d %02d:%02d",cmd[5],cmd[6],cmd[7],cmd[8],cmd[9],cmd[10]);
				printf(CAP_PROCESS"date is %s\r\n",date);
				message=add_item(message,ID_DEVICE_CAP_TIME,date);
				if(warnning_msg!=NULL)
				{	//have alarm msg upload
					if(sensor.co2&&(!sensor.co2_send))
					{
						sprintf(error,"%sth sensor possible error",ID_CAP_CO2);
						warnning_msg=rm_item(warnning_msg,ID_ALARM_SENSOR);
						warnning_msg=rm_item(warnning_msg,ID_ALERT_CAP_FAILED);
						warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,ID_CAP_CO2);
						warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
						send_server_save_local(NULL,warnning_msg,0);
						sensor.co2_send=1;
					}
					if(sensor.co&&(!sensor.co_send))
					{
						sprintf(error,"%sth sensor possible error",ID_CAP_CO);
						warnning_msg=rm_item(warnning_msg,ID_ALARM_SENSOR);
						warnning_msg=rm_item(warnning_msg,ID_ALERT_CAP_FAILED);
						warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,ID_CAP_CO);
						warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
						send_server_save_local(NULL,warnning_msg,0);
						sensor.co_send=1;
					}
					if(sensor.hcho&&(!sensor.hcho_send))
					{
						sprintf(error,"%sth sensor possible error",ID_CAP_HCHO);
						warnning_msg=rm_item(warnning_msg,ID_ALARM_SENSOR);
						warnning_msg=rm_item(warnning_msg,ID_ALERT_CAP_FAILED);
						warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,ID_CAP_HCHO);
						warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
						send_server_save_local(NULL,warnning_msg,0);
						sensor.hcho_send=1;
					}
					if(sensor.shidu&&(!sensor.shidu_send))
					{	
						sprintf(error,"%sth sensor possible error",ID_CAP_SHI_DU);
						warnning_msg=rm_item(warnning_msg,ID_ALARM_SENSOR);
						warnning_msg=rm_item(warnning_msg,ID_ALERT_CAP_FAILED);
						warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,ID_CAP_SHI_DU);
						warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
						send_server_save_local(NULL,warnning_msg,0);
						sensor.shidu_send=1;
					}
					if(sensor.temp&&(!sensor.temp_send))
					{	
						sprintf(error,"%sth sensor possible error",ID_CAP_TEMPERATURE);
						warnning_msg=rm_item(warnning_msg,ID_ALARM_SENSOR);
						warnning_msg=rm_item(warnning_msg,ID_ALERT_CAP_FAILED);
						warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,ID_CAP_TEMPERATURE);
						warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
						send_server_save_local(NULL,warnning_msg,0);
						sensor.temp_send=1;
					}
					if(sensor.pm25&&(!sensor.pm25_send))
					{
						sprintf(error,"%sth sensor possible error",ID_CAP_PM_25);
						warnning_msg=rm_item(warnning_msg,ID_ALARM_SENSOR);
						warnning_msg=rm_item(warnning_msg,ID_ALERT_CAP_FAILED);
						warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,ID_CAP_PM_25);
						warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
						send_server_save_local(NULL,warnning_msg,0);
						sensor.pm25_send=1;
					}					
					printf(CAP_PROCESS"Upload alarm msg :\n");
					free(warnning_msg);
					warnning_msg=NULL;
				}
				if(g_upload)
				{
					g_upload=0;
					printf(CAP_PROCESS"Upload data msg :\n");
					send_server_save_local(date,message,1);
					
				}
				free(message);
				message=NULL;
				memset(date,'\0',32);
			}
			break;
			case VERIFY_BYTE:
			{
				//write(fd,server_time,13);
			}
			break;
			case RESEND_BYTE:
			{
				write(fd,server_time,13);
			}
			break;
			default:
			{
				/*get cap data*/
				if(cmd[5]==0x65 && cmd[6]==0x72 && cmd[7]==0x72 && cmd[8]==0x6f && cmd[9]==0x72)
				{	//check uninsert msg
					if(cmd[3]==atoi(ID_CAP_CO2) && !sensor.co2)
					{
						sensor.co2|=ALARM_UNINSERT;
						sensor.co2_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_CO) && !sensor.co)
					{
						sensor.co|=ALARM_UNINSERT;
						sensor.co_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_HCHO) && !sensor.hcho)
					{
						sensor.hcho|=ALARM_UNINSERT;
						sensor.hcho_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_SHI_DU) && !sensor.shidu)
					{
						sensor.shidu|=ALARM_UNINSERT;
						sensor.shidu_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& !sensor.temp)
					{
						sensor.temp|=ALARM_UNINSERT;
						sensor.temp_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_PM_25)&& !sensor.pm25)
					{
						sensor.pm25|=ALARM_UNINSERT;
						sensor.pm25_send=0;
					}
					else 
						return message;
					//inform server
					//sprintf(error,"%dth sensor possible error",message_type);
					warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
					//warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
					//warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,id);
					warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UNINSERT);
					memset(error,'\0',32);
					save_sensor_alarm_info();					
					return message;
				}
				else
				{	//normal data or beyond Min & Max data
					int value=0;
					//clear the uninsert alarm
					if(cmd[3]==atoi(ID_CAP_CO2) && (sensor.co2&ALARM_UNINSERT))
					{					
						clear_alarm(ID_CAP_CO2,ID_ALERT_UNINSERT);						
						sensor.co2&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_CO) && (sensor.co&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_CO,ID_ALERT_UNINSERT);
						sensor.co&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_HCHO) && (sensor.hcho&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_HCHO,ID_ALERT_UNINSERT);
						sensor.hcho&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_SHI_DU) && (sensor.shidu&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_SHI_DU,ID_ALERT_UNINSERT);
						sensor.shidu&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& (sensor.temp&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_UNINSERT);
						sensor.temp&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_PM_25)&& (sensor.pm25&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_PM_25,ID_ALERT_UNINSERT);
						sensor.pm25&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(message==NULL)
					{
						message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
						message=add_item(message,ID_DEVICE_UID,g_uuid);
						message=add_item(message,ID_DEVICE_IP_ADDR,ip);
						message=add_item(message,ID_DEVICE_PORT,"9517");	
					}
					//sprintf(id,"%d",message_type);
					sprintf(data,"%d",cmd[5]<<8|cmd[6]);						
					if(cmd[7]!=0)
					{//have .
						int m;
						if(cmd[7]>strlen(data))//like 0.012
						{
							char tmp_buf[10]={0};
							int dist=cmd[7]-strlen(data);
							strcpy(tmp_buf,"0.");
							for(m=0;m<dist;m++)
								strcat(tmp_buf,"0");
							strcat(tmp_buf,data);
							strcpy(data,tmp_buf);
						}
						else//like 12.33
						{
							int left,right,number,n=1;
							number=(cmd[5]<<8)|cmd[6];
							for(m=0;m<cmd[7];m++)
								n=n*10;
							right=number%n;
							left=number/n;
							sprintf(data,"%d.%d",left,right);								
						}
						//if(g_upload)
						{
							int temp=1;
							for(i=0;i<cmd[7];i++)
								temp*=10;
							value=((cmd[5]<<8|cmd[6]))/temp;
						}
					}	
					else
					{
						//if(g_upload)
							value=(cmd[5]<<8|cmd[6]);
					}
					//printf(CAP_PROCESS"Value %d\n",value);
					#if 1
					//if(g_upload)
					{
						if(cmd[3]==atoi(ID_CAP_CO2))
						{
							if(value<MIN_CO2)
								sensortimes.co2++;
							else if(value>MAX_CO2)
								sensortimes.co2++;
							else
							{
								sensortimes.co2=0;
								if(sensor.co2&ALARM_BELOW)
									clear_alarm(ID_CAP_CO2,ID_ALERT_BELOW);
								if(sensor.co2&ALARM_UP)
									clear_alarm(ID_CAP_CO2,ID_ALERT_UP);
								if(sensor.co2&ALARM_BELOW||sensor.co2&ALARM_UP)
								{
									sensor.co2=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.co2==MAX_COUNT_TIMES && !sensor.co2)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_CO2)
								{
									sensor.co2|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.co2|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.co2_send=0;
							}
						}
						if(cmd[3]==atoi(ID_CAP_CO))
						{
							if(value<MIN_CO)
								sensortimes.co++;
							else if(value>MAX_CO)
								sensortimes.co++;
							else
							{
								sensortimes.co=0;								
								if(sensor.co&ALARM_BELOW)
									clear_alarm(ID_CAP_CO,ID_ALERT_BELOW);
								if(sensor.co&ALARM_UP)
									clear_alarm(ID_CAP_CO,ID_ALERT_UP);
								if(sensor.co&ALARM_BELOW||sensor.co&ALARM_UP)
								{
									sensor.co=ALARM_NONE;
									save_sensor_alarm_info();	
								}
							}
							if(sensortimes.co==MAX_COUNT_TIMES && !sensor.co)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_CO)
								{
									sensor.co|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.co|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();
								sensor.co_send=0;								
							}
						}
						if(cmd[3]==atoi(ID_CAP_HCHO))
						{
							if(value<MIN_HCHO)
								sensortimes.hcho++;
							else if(value>MAX_HCHO)
								sensortimes.hcho++;
							else
							{
								sensortimes.hcho=0;
								if(sensor.hcho&ALARM_BELOW)
									clear_alarm(ID_CAP_HCHO,ID_ALERT_BELOW);
								if(sensor.hcho&ALARM_UP)
									clear_alarm(ID_CAP_HCHO,ID_ALERT_UP);	
								if(sensor.hcho&ALARM_BELOW||sensor.hcho&ALARM_UP)
								{
									sensor.hcho=ALARM_NONE;
									save_sensor_alarm_info();	
								}
							}
							if(sensortimes.hcho==MAX_COUNT_TIMES && !sensor.hcho)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_HCHO)
								{
									sensor.hcho|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.hcho|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.hcho_send=0;				
							}
						}
						if(cmd[3]==atoi(ID_CAP_SHI_DU))
						{
							if(value<MIN_SHIDU)
								sensortimes.shidu++;
							else if(value>MAX_SHIDU)
								sensortimes.shidu++;
							else
							{
								sensortimes.shidu=0;								
								if(sensor.shidu&ALARM_BELOW)
									clear_alarm(ID_CAP_SHI_DU,ID_ALERT_BELOW);
								if(sensor.shidu&ALARM_UP)
									clear_alarm(ID_CAP_SHI_DU,ID_ALERT_UP);
								if(sensor.shidu&ALARM_BELOW||sensor.shidu&ALARM_UP)
								{
									sensor.shidu=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.shidu==MAX_COUNT_TIMES && !sensor.shidu)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_SHIDU)
								{
									sensor.shidu|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.shidu|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.shidu_send=0;				
							}
						}
						if(cmd[3]==atoi(ID_CAP_TEMPERATURE))
						{
							if(value<MIN_TEMP)
								sensortimes.temp++;
							else if(value>MAX_TEMP)
								sensortimes.temp++;
							else
							{
								sensortimes.temp=0;
								if(sensor.temp&ALARM_BELOW)
									clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_BELOW);
								if(sensor.temp&ALARM_UP)
									clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_UP);
								if(sensor.temp&ALARM_BELOW||sensor.temp&ALARM_UP)
								{
									sensor.temp=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.temp==MAX_COUNT_TIMES&& !sensor.temp)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_TEMP)
								{
									sensor.temp|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.temp|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.temp_send=0;				
							}
						}
						if(cmd[3]==atoi(ID_CAP_PM_25))
						{
							if(value<MIN_PM25)
								sensortimes.pm25++;
							else if(value>MAX_PM25)
								sensortimes.pm25++;
							else
							{
								sensortimes.pm25=0;
								if(sensor.pm25&ALARM_BELOW)
									clear_alarm(ID_CAP_PM_25,ID_ALERT_BELOW);
								if(sensor.pm25&ALARM_UP)
									clear_alarm(ID_CAP_PM_25,ID_ALERT_UP);
								if(sensor.pm25&ALARM_BELOW||sensor.pm25&ALARM_UP)
								{
									sensor.pm25=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.pm25==MAX_COUNT_TIMES&& !sensor.pm25)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_PM25)
								{
									sensor.pm25|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.pm25|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.pm25_send=0;				
							}
						}			
					}
					#endif
					//real time update cap data
					if(strncmp(id,ID_CAP_CO,strlen(ID_CAP_CO))==0)
					{
						write_data(fd_lcd,VAR_DATE_TIME_1,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
					{						
						write_data(fd_lcd,VAR_DATE_TIME_2,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_HCHO,strlen(ID_CAP_HCHO))==0)
					{
						write_data(fd_lcd,VAR_DATE_TIME_3,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(ID_CAP_TEMPERATURE))==0)
					{
						write_data(fd_lcd,VAR_DATE_TIME_4,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_SHI_DU,strlen(ID_CAP_SHI_DU))==0)
					{
						write_data(fd_lcd,VAR_ALARM_TYPE_1,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
					{
						write_data(fd_lcd,VAR_ALARM_TYPE_2,cmd[5]<<8|cmd[6]);
					}
					message=add_item(message,id,data);
					//printf(SUB_PROCESS"id %s data %s\r\n==>\n%s\n",id,data,post_message);
					return message;
				}
			}
			break;
		}
	}
	else
	{
		printf(CAP_PROCESS"CRC error 0x%04X\r\n",CRC_check(cmd,len-2));
		for(i=0;i<len;i++)
			printf("0x%02x ",cmd[i]);
	}
	return message;
}
/*
  *get cmd from lv's cap board
*/
int get_uart(int fd_lcd,int fd)
{
	char ch,state=STATE_IDLE,message_len=0;
	char message[10],i=0,to_check[20];
	int crc,message_type=0;
	while(1)
	{
		if(read(fd,&ch,1)==1)
		{
			switch (state)
			{
				case STATE_IDLE:
				{
					if(ch==0x6c)
						state=STATE_6C;
				}
				break;
				case STATE_6C:
				{
					if(ch==0xaa||ch==0xa1)
					{
						state=STATE_AA;
						i=0;
					}						
				}
				break;
				case STATE_AA:
				{
					message_type=ch<<8;
					i=0;
					state=STATE_MESSAGE_TYPE;
				}
				break;
				case STATE_MESSAGE_TYPE:
				{
					message_type|=ch;
					state=STATE_MESSAGE_LEN;
				}
				break;
				case STATE_MESSAGE_LEN:
				{
					message_len=ch;
					state=STATE_MESSAGE;
					i=0;
				}
				break;
				case STATE_MESSAGE:
				{
					if(i!=message_len)
					{
						message[i++]=ch;
					}
					else
					{
						state=STATE_CRC;
						crc=ch<<8;
					}	
				}
				break;
				case STATE_CRC:
				{
					crc|=ch;
					for(i=0;i<message_len;i++)
					{
						to_check[5+i]=message[i];
					}
					to_check[0]=0x6c;to_check[1]=0xaa;to_check[2]=(message_type>>8)&0xff;to_check[3]=message_type&0xff;
					to_check[4]=message_len;to_check[5+message_len]=(crc>>8)&0xff;
					to_check[5+message_len+1]=crc&0xff;
					char *cmd=(char *)malloc(message_len+7);
					memset(cmd,'\0',message_len+7);
					memcpy(cmd,to_check,message_len+7);
					post_message=build_message(fd,fd_lcd,cmd,message_len+7,post_message);
					free(cmd);
					return 0;						
				}
				default:
				{
					i=0;
					state=STATE_IDLE;
				}	
			}
		}		
	}
	return 0;
}
#endif
void switch_pic(int fd,unsigned char Index)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x04,0x00};
	cmd[5]=Index;
	write(fd,cmd,6);
}
void write_data(int fd,unsigned int Index,int data)
{
	char cmd[]={0x5a,0xa5,0x05,0x82,0x00,0x00,0x00,0x00};
	cmd[4]=(Index&0xff00)>>8;cmd[5]=Index&0x00ff;
	cmd[6]=(data&0xff00)>>8;cmd[7]=data&0x00ff;
	write(fd,cmd,8);
}
void write_string(int fd,unsigned int addr,char *data,int len)
{
	int i=0;
	char *cmd=(char *)malloc(len+6);
	cmd[0]=0x5a;cmd[1]=0xa5;cmd[2]=len+3;cmd[3]=0x82;
	cmd[4]=(addr&0xff00)>>8;cmd[5]=addr&0x00ff;
	for(i=0;i<len;i++)
		cmd[6+i]=data[i];
	for(i=0;i<len+6;i++)
		printf("%02x ",cmd[i]);
	printf("\n<len %d>\n",len);
	write(fd,cmd,len+6);
	free(cmd);
}
#define LCD_PROCESS "[LCD_PROCESS]"
FILE *history_fp=NULL;
void load_history(const char *name)
{
	DIR *d = NULL;
	struct dirent *de = NULL;
	char file_list[512][15];
	int i=0,j=0,m=0;	
	int cnt=0,cnt_co=0;
	char history_time[100000][20];
	char history_data[100000][10];
	char year_j[5]={0},year_m[5]={0},tmp_file[15]={0};
	char mon_j[3]={0},mon_m[3]={0};
	char day_j[3]={0},day_m[3]={0};
	d = opendir(name);
	if(d == 0)
	{
		printf("open failed %s , %s",name,strerror(errno));
		return;
	}
	
	while((de = readdir(d))!=0)
	{
		if(strncmp(de->d_name,".",strlen("."))==0||strncmp(de->d_name,"..",strlen(".."))==0)
			continue;
		memset(file_list[i],'\0',15);
		strcpy(file_list[i],de->d_name);
		i++;
	}	
	closedir(d);
	//compare year
	for(j=0;j<i-1;j++)
	{
		for(m=j+1;m<i;m++)
		{
			memcpy(year_j,file_list[j],4);
			memcpy(year_m,file_list[m],4);
			if(atoi(year_j)>atoi(year_m))
			{
				strcpy(tmp_file,file_list[j]);
				strcpy(file_list[j],file_list[m]);
				strcpy(file_list[m],tmp_file);
			}
		}
	}
	for(j=0;j<i-1;j++)
	{	
		for(m=j+1;m<i;m++)
		{
			memcpy(year_j,file_list[j],4);
			memcpy(mon_j,file_list[j]+5,2);
			memcpy(year_m,file_list[m],4);
			memcpy(mon_m,file_list[m]+5,2);
			if((atoi(mon_j)>atoi(mon_m)) && (atoi(year_m)==atoi(year_j)))
			{
				strcpy(tmp_file,file_list[j]);
				strcpy(file_list[j],file_list[m]);
				strcpy(file_list[m],tmp_file);
			}
		}
	}
	for(j=0;j<i-1;j++)
	{		
		for(m=j+1;m<i;m++)
		{
			memcpy(year_j,file_list[j],4);
			memcpy(mon_j,file_list[j]+5,2);
			memcpy(day_j,file_list[j]+8,2);
			memcpy(year_m,file_list[m],4);
			memcpy(mon_m,file_list[m]+5,2);
			memcpy(day_m,file_list[m]+8,2);
			if((atoi(day_j)>atoi(day_m)) && (atoi(mon_j)==atoi(mon_m)) && (atoi(year_m)==atoi(year_j)))
			{
				//printf(LCD_PROCESS"switch day_j %s,day_m %s,mon_j %s,mon_m %s,year_j %s,year_m %s\n",day_j,day_m,mon_j,mon_m,year_j,year_m);
				strcpy(tmp_file,file_list[j]);
				strcpy(file_list[j],file_list[m]);
				strcpy(file_list[m],tmp_file);
			}
		}
	}
	for(j=0;j<i;j++)
	{
		char *line=NULL;
		char file_path[32]={0};
		int len;
		printf(LCD_PROCESS"==> %s\n",file_list[j]);
		strcpy(file_path,"/home/user/history/");
		strcat(file_path,file_list[j]);
		FILE *fp = fopen(file_path, "r");
		while (getline(&line, &len, fp) != -1) 
		{
			if((cnt%2)!=0)
			{
				//get co,co2,hcho,pm25,shidu,temp
				char *data=doit_data(line,ID_CAP_CO);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(history_co_data[g_history_co_cnt],'\0',10);
					strcpy(history_co_data[g_history_co_cnt],data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					free(data);
					g_history_co_cnt++;
				}
				data=doit_data(line,ID_CAP_CO2);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(history_co2_data[g_history_co2_cnt],'\0',10);
					//strcpy(history_co2_data[g_history_co2_cnt],data);					
					sprintf(history_co2_data[g_history_co2_cnt],"%04d",atoi(data));
					free(data);
					g_history_co2_cnt++;
				}
				data=doit_data(line,ID_CAP_HCHO);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(history_hcho_data[g_history_hcho_cnt],'\0',10);
					strcpy(history_hcho_data[g_history_hcho_cnt],data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					free(data);
					g_history_hcho_cnt++;
				}
				data=doit_data(line,ID_CAP_SHI_DU);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(history_shidu_data[g_history_shidu_cnt],'\0',10);
					strcpy(history_shidu_data[g_history_shidu_cnt],data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					free(data);
					g_history_shidu_cnt++;
				}
				data=doit_data(line,ID_CAP_TEMPERATURE);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(history_temp_data[g_history_temp_cnt],'\0',10);
					strcpy(history_temp_data[g_history_temp_cnt],data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					free(data);
					g_history_temp_cnt++;
				}
				data=doit_data(line,ID_CAP_PM_25);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(history_pm25_data[g_history_pm25_cnt],'\0',10);
					//strcpy(history_pm25_data[g_history_pm25_cnt],data);					
					sprintf(history_pm25_data[g_history_pm25_cnt],"%03d",atoi(data));
					free(data);
					g_history_pm25_cnt++;
				}
			}
			else
			{
				char tmp[11]={0};
				memset(history_co_time[g_history_co_cnt],'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(history_co_time[g_history_co_cnt],tmp);
				strcat(history_co_time[g_history_co_cnt]," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(history_co_time[g_history_co_cnt],tmp);
				
				memset(history_co2_time[g_history_co2_cnt],'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(history_co2_time[g_history_co2_cnt],tmp);
				strcat(history_co2_time[g_history_co2_cnt]," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(history_co2_time[g_history_co2_cnt],tmp);
				
				memset(history_hcho_time[g_history_hcho_cnt],'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(history_hcho_time[g_history_hcho_cnt],tmp);
				strcat(history_hcho_time[g_history_hcho_cnt]," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(history_hcho_time[g_history_hcho_cnt],tmp);
				
				memset(history_shidu_time[g_history_shidu_cnt],'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(history_shidu_time[g_history_shidu_cnt],tmp);
				strcat(history_shidu_time[g_history_shidu_cnt]," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(history_shidu_time[g_history_shidu_cnt],tmp);
				
				memset(history_temp_time[g_history_temp_cnt],'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(history_temp_time[g_history_temp_cnt],tmp);
				strcat(history_temp_time[g_history_temp_cnt]," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(history_temp_time[g_history_temp_cnt],tmp);
				
				memset(history_pm25_time[g_history_pm25_cnt],'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(history_pm25_time[g_history_pm25_cnt],tmp);
				strcat(history_pm25_time[g_history_pm25_cnt]," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(history_pm25_time[g_history_pm25_cnt],tmp);
			}
			cnt++;
		}
		fclose(fp);
	}
	//for(i=0;i<g_history_co_cnt;i++)
		//printf(LCD_PROCESS"[%d]time %s, data %s\n",i,history_co_time[i],history_co_data[i]);
}

void show_history(int fd_lcd,const char *name,char *id,int offset)
{
	#if 0
	DIR *d = NULL;
	struct dirent *de = NULL;
	char file_list[512][15];
	int i=0,j=0,m=0,cnt=0;	
	char *line=NULL;
	char file_path[32]={0};
	int len;
	char year_j[5]={0},year_m[5]={0},tmp_file[15]={0};
	char mon_j[3]={0},mon_m[3]={0};
	char day_j[3]={0},day_m[3]={0};	
	char history_time[8][20];
	char history_data[8][10];
	d = opendir(name);
	if(d == 0)
	{
		printf("open failed %s , %s",name,strerror(errno));
		return;
	}
	
	while((de = readdir(d))!=0)
	{
		if(strncmp(de->d_name,".",strlen("."))==0||strncmp(de->d_name,"..",strlen(".."))==0)
			continue;
		//printf(LCD_PROCESS"==> %s\n",de->d_name);
		memset(file_list[i],'\0',15);
		strcpy(file_list[i],de->d_name);
		i++;
	}	
	closedir(d);
	//compare year
	for(j=0;j<i-1;j++)
	{
		memcpy(year_j,file_list[j],4);
		for(m=j+1;m<i;m++)
		{
			memcpy(year_m,file_list[m],4);
			if(atoi(year_j)<atoi(year_m))
			{
				strcpy(tmp_file,file_list[j]);
				strcpy(file_list[j],file_list[m]);
				strcpy(file_list[m],tmp_file);
			}
		}
	}
	for(j=0;j<i-1;j++)
	{
		memcpy(year_j,file_list[j],4);
		memcpy(mon_j,file_list[j]+5,2);
		for(m=j+1;m<i;m++)
		{
			memcpy(year_m,file_list[m],4);
			memcpy(mon_m,file_list[m]+5,2);
			if((atoi(mon_j)<atoi(mon_m)) && (atoi(year_m)==atoi(year_j)))
			{
				strcpy(tmp_file,file_list[j]);
				strcpy(file_list[j],file_list[m]);
				strcpy(file_list[m],tmp_file);
			}
		}
	}
	for(j=0;j<i-1;j++)
	{
		memcpy(year_j,file_list[j],4);
		memcpy(mon_j,file_list[j]+5,2);
		memcpy(day_j,file_list[j]+8,2);
		for(m=j+1;m<i;m++)
		{
			memcpy(year_m,file_list[m],4);
			memcpy(mon_m,file_list[m]+5,2);
			memcpy(day_m,file_list[m]+8,2);
			//printf(LCD_PROCESS"year_j %s,mon_j %s,day_j %s %d <> year_m %s,mon_m %s,day_m %s %d\n",year_j,mon_j,day_j,atoi(day_j),year_m,mon_m,day_m,atoi(day_m));
			if((atoi(day_j)<atoi(day_m)) && (atoi(mon_j)==atoi(mon_m)) && (atoi(year_m)==atoi(year_j)))
			{
				strcpy(tmp_file,file_list[j]);
				strcpy(file_list[j],file_list[m]);
				strcpy(file_list[m],tmp_file);
			}
		}
	}
	strcpy(file_path,"/home/user/history/");
	strcat(file_path,file_list[0]);	
	printf(LCD_PROCESS"==> %s id %s\n",file_path,id);
	FILE *fp = fopen(file_path, "r");
	i=0;
	m=0;
	if(fp==NULL)
	{
		printf(LCD_PROCESS"can not open %s\n",file_path);
		return ;
	}	
	while(getline(&line, &len, fp) != -1) 
	{
		i++;
		printf(LCD_PROCESS"i is %d\n",i);
	}
	
	fseek(fp,0L,SEEK_SET);
	if(i>(14+offset*2))
		m=i-14-offset*2;
	else
		m=0;
	printf(LCD_PROCESS"m is %d\n",m);
	i=0;
	while (getline(&line, &len, fp) != -1) 
	{
		if(i<m)
		{
			i++;
			continue;
		}
		printf(LCD_PROCESS"<line> %s \n",line);
		if((i%2)!=0)
		{
			//get co,co2,hcho,pm25,shidu,temp
			char *data=doit_data(line,id);
			if(data!=NULL)
			{
				memset(history_data[cnt],'\0',20);
				//strcpy(history_data[cnt],data);					
				sprintf(history_data[cnt],"%04d",atoi(data));
				free(data);
				printf(LCD_PROCESS"<Data %d>%s\n",cnt,history_data[cnt]);
				printf(LCD_PROCESS"<time 00>%s\n",history_time[0]);
				cnt++;
				if(cnt==7)
					break;
			}
		}
		else
		{
			char tmp[11]={0};
			memset(history_time[cnt],'\0',20);
			memcpy(tmp,file_list[0],10);
			strcpy(history_time[cnt],tmp);
			strcat(history_time[cnt]," ");
			memset(tmp,'\0',11);
			memcpy(tmp,line,5);
			strcat(history_time[cnt],tmp);
			printf(LCD_PROCESS"<time %d>%s\n",cnt,history_time[cnt]);
			printf(LCD_PROCESS"<time 0>%s\n",history_time[0]);
		}
		i++;
	}
	free(line);
	fclose(fp);
	#endif
	if(strncmp(id,ID_CAP_CO,strlen(id))==0)
	{
		if((g_history_co_cnt-offset-7)>0)
		{
			write_string(fd_lcd,VAR_CO_TIME1,history_co_time[g_history_co_cnt-offset-1],strlen(history_co_time[g_history_co_cnt-offset-1]));
			write_string(fd_lcd,VAR_CO_DATA1,history_co_data[g_history_co_cnt-offset-1],strlen(history_co_data[g_history_co_cnt-offset-1]));
			write_string(fd_lcd,VAR_CO_TIME2,history_co_time[g_history_co_cnt-offset-2],strlen(history_co_time[g_history_co_cnt-offset-2]));
			write_string(fd_lcd,VAR_CO_DATA2,history_co_data[g_history_co_cnt-offset-2],strlen(history_co_data[g_history_co_cnt-offset-2]));
			write_string(fd_lcd,VAR_CO_TIME3,history_co_time[g_history_co_cnt-offset-3],strlen(history_co_time[g_history_co_cnt-offset-3]));
			write_string(fd_lcd,VAR_CO_DATA3,history_co_data[g_history_co_cnt-offset-3],strlen(history_co_data[g_history_co_cnt-offset-3]));
			write_string(fd_lcd,VAR_CO_TIME4,history_co_time[g_history_co_cnt-offset-4],strlen(history_co_time[g_history_co_cnt-offset-4]));
			write_string(fd_lcd,VAR_CO_DATA4,history_co_data[g_history_co_cnt-offset-4],strlen(history_co_data[g_history_co_cnt-offset-4]));
			write_string(fd_lcd,VAR_CO_TIME5,history_co_time[g_history_co_cnt-offset-5],strlen(history_co_time[g_history_co_cnt-offset-5]));
			write_string(fd_lcd,VAR_CO_DATA5,history_co_data[g_history_co_cnt-offset-5],strlen(history_co_data[g_history_co_cnt-offset-5]));
			write_string(fd_lcd,VAR_CO_TIME6,history_co_time[g_history_co_cnt-offset-6],strlen(history_co_time[g_history_co_cnt-offset-6]));
			write_string(fd_lcd,VAR_CO_DATA6,history_co_data[g_history_co_cnt-offset-6],strlen(history_co_data[g_history_co_cnt-offset-6]));
			write_string(fd_lcd,VAR_CO_TIME7,history_co_time[g_history_co_cnt-offset-7],strlen(history_co_time[g_history_co_cnt-offset-7]));
			write_string(fd_lcd,VAR_CO_DATA7,history_co_data[g_history_co_cnt-offset-7],strlen(history_co_data[g_history_co_cnt-offset-7]));
		}
	}
	if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
		if((g_history_co2_cnt-offset-7)>0)
		{
			write_string(fd_lcd,VAR_CO2_TIME1,history_co2_time[g_history_co2_cnt-offset-1],strlen(history_co2_time[g_history_co2_cnt-offset-1]));
			write_string(fd_lcd,VAR_CO2_DATA1,history_co2_data[g_history_co2_cnt-offset-1],strlen(history_co2_data[g_history_co2_cnt-offset-1]));
			write_string(fd_lcd,VAR_CO2_TIME2,history_co2_time[g_history_co2_cnt-offset-2],strlen(history_co2_time[g_history_co2_cnt-offset-2]));
			write_string(fd_lcd,VAR_CO2_DATA2,history_co2_data[g_history_co2_cnt-offset-2],strlen(history_co2_data[g_history_co2_cnt-offset-2]));
			write_string(fd_lcd,VAR_CO2_TIME3,history_co2_time[g_history_co2_cnt-offset-3],strlen(history_co2_time[g_history_co2_cnt-offset-3]));
			write_string(fd_lcd,VAR_CO2_DATA3,history_co2_data[g_history_co2_cnt-offset-3],strlen(history_co2_data[g_history_co2_cnt-offset-3]));
			write_string(fd_lcd,VAR_CO2_TIME4,history_co2_time[g_history_co2_cnt-offset-4],strlen(history_co2_time[g_history_co2_cnt-offset-4]));
			write_string(fd_lcd,VAR_CO2_DATA4,history_co2_data[g_history_co2_cnt-offset-4],strlen(history_co2_data[g_history_co2_cnt-offset-4]));
			write_string(fd_lcd,VAR_CO2_TIME5,history_co2_time[g_history_co2_cnt-offset-5],strlen(history_co2_time[g_history_co2_cnt-offset-5]));
			write_string(fd_lcd,VAR_CO2_DATA5,history_co2_data[g_history_co2_cnt-offset-5],strlen(history_co2_data[g_history_co2_cnt-offset-5]));
			write_string(fd_lcd,VAR_CO2_TIME6,history_co2_time[g_history_co2_cnt-offset-6],strlen(history_co2_time[g_history_co2_cnt-offset-6]));
			write_string(fd_lcd,VAR_CO2_DATA6,history_co2_data[g_history_co2_cnt-offset-6],strlen(history_co2_data[g_history_co2_cnt-offset-6]));
			write_string(fd_lcd,VAR_CO2_TIME7,history_co2_time[g_history_co2_cnt-offset-7],strlen(history_co2_time[g_history_co2_cnt-offset-7]));
			write_string(fd_lcd,VAR_CO2_DATA7,history_co2_data[g_history_co2_cnt-offset-7],strlen(history_co2_data[g_history_co2_cnt-offset-7]));
		}
	}
	if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
		if((g_history_hcho_cnt-offset-7)>0)
		{
			write_string(fd_lcd,VAR_HCHO_TIME1,history_hcho_time[g_history_hcho_cnt-offset-1],strlen(history_hcho_time[g_history_hcho_cnt-offset-1]));
			write_string(fd_lcd,VAR_HCHO_DATA1,history_hcho_data[g_history_hcho_cnt-offset-1],strlen(history_hcho_data[g_history_hcho_cnt-offset-1]));
			write_string(fd_lcd,VAR_HCHO_TIME2,history_hcho_time[g_history_hcho_cnt-offset-2],strlen(history_hcho_time[g_history_hcho_cnt-offset-2]));
			write_string(fd_lcd,VAR_HCHO_DATA2,history_hcho_data[g_history_hcho_cnt-offset-2],strlen(history_hcho_data[g_history_hcho_cnt-offset-2]));
			write_string(fd_lcd,VAR_HCHO_TIME3,history_hcho_time[g_history_hcho_cnt-offset-3],strlen(history_hcho_time[g_history_hcho_cnt-offset-3]));
			write_string(fd_lcd,VAR_HCHO_DATA3,history_hcho_data[g_history_hcho_cnt-offset-3],strlen(history_hcho_data[g_history_hcho_cnt-offset-3]));
			write_string(fd_lcd,VAR_HCHO_TIME4,history_hcho_time[g_history_hcho_cnt-offset-4],strlen(history_hcho_time[g_history_hcho_cnt-offset-4]));
			write_string(fd_lcd,VAR_HCHO_DATA4,history_hcho_data[g_history_hcho_cnt-offset-4],strlen(history_hcho_data[g_history_hcho_cnt-offset-4]));
			write_string(fd_lcd,VAR_HCHO_TIME5,history_hcho_time[g_history_hcho_cnt-offset-5],strlen(history_hcho_time[g_history_hcho_cnt-offset-5]));
			write_string(fd_lcd,VAR_HCHO_DATA5,history_hcho_data[g_history_hcho_cnt-offset-5],strlen(history_hcho_data[g_history_hcho_cnt-offset-5]));
			write_string(fd_lcd,VAR_HCHO_TIME6,history_hcho_time[g_history_hcho_cnt-offset-6],strlen(history_hcho_time[g_history_hcho_cnt-offset-6]));
			write_string(fd_lcd,VAR_HCHO_DATA6,history_hcho_data[g_history_hcho_cnt-offset-6],strlen(history_hcho_data[g_history_hcho_cnt-offset-6]));
			write_string(fd_lcd,VAR_HCHO_TIME7,history_hcho_time[g_history_hcho_cnt-offset-7],strlen(history_hcho_time[g_history_hcho_cnt-offset-7]));
			write_string(fd_lcd,VAR_HCHO_DATA7,history_hcho_data[g_history_hcho_cnt-offset-7],strlen(history_hcho_data[g_history_hcho_cnt-offset-7]));
		}
	}
	if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
		if((g_history_shidu_cnt-offset-7)>0)
		{
			write_string(fd_lcd,VAR_SHIDU_TIME1,history_shidu_time[g_history_shidu_cnt-offset-1],strlen(history_shidu_time[g_history_shidu_cnt-offset-1]));
			write_string(fd_lcd,VAR_SHIDU_DATA1,history_shidu_data[g_history_shidu_cnt-offset-1],strlen(history_shidu_data[g_history_shidu_cnt-offset-1]));
			write_string(fd_lcd,VAR_SHIDU_TIME2,history_shidu_time[g_history_shidu_cnt-offset-2],strlen(history_shidu_time[g_history_shidu_cnt-offset-2]));
			write_string(fd_lcd,VAR_SHIDU_DATA2,history_shidu_data[g_history_shidu_cnt-offset-2],strlen(history_shidu_data[g_history_shidu_cnt-offset-2]));
			write_string(fd_lcd,VAR_SHIDU_TIME3,history_shidu_time[g_history_shidu_cnt-offset-3],strlen(history_shidu_time[g_history_shidu_cnt-offset-3]));
			write_string(fd_lcd,VAR_SHIDU_DATA3,history_shidu_data[g_history_shidu_cnt-offset-3],strlen(history_shidu_data[g_history_shidu_cnt-offset-3]));
			write_string(fd_lcd,VAR_SHIDU_TIME4,history_shidu_time[g_history_shidu_cnt-offset-4],strlen(history_shidu_time[g_history_shidu_cnt-offset-4]));
			write_string(fd_lcd,VAR_SHIDU_DATA4,history_shidu_data[g_history_shidu_cnt-offset-4],strlen(history_shidu_data[g_history_shidu_cnt-offset-4]));
			write_string(fd_lcd,VAR_SHIDU_TIME5,history_shidu_time[g_history_shidu_cnt-offset-5],strlen(history_shidu_time[g_history_shidu_cnt-offset-5]));
			write_string(fd_lcd,VAR_SHIDU_DATA5,history_shidu_data[g_history_shidu_cnt-offset-5],strlen(history_shidu_data[g_history_shidu_cnt-offset-5]));
			write_string(fd_lcd,VAR_SHIDU_TIME6,history_shidu_time[g_history_shidu_cnt-offset-6],strlen(history_shidu_time[g_history_shidu_cnt-offset-6]));
			write_string(fd_lcd,VAR_SHIDU_DATA6,history_shidu_data[g_history_shidu_cnt-offset-6],strlen(history_shidu_data[g_history_shidu_cnt-offset-6]));
			write_string(fd_lcd,VAR_SHIDU_TIME7,history_shidu_time[g_history_shidu_cnt-offset-7],strlen(history_shidu_time[g_history_shidu_cnt-offset-7]));
			write_string(fd_lcd,VAR_SHIDU_DATA7,history_shidu_data[g_history_shidu_cnt-offset-7],strlen(history_shidu_data[g_history_shidu_cnt-offset-7]));
		}
	}
	if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
		if((g_history_temp_cnt-offset-7)>0)
		{
			write_string(fd_lcd,VAR_TEMP_TIME1,history_temp_time[g_history_temp_cnt-offset-1],strlen(history_temp_time[g_history_temp_cnt-offset-1]));
			write_string(fd_lcd,VAR_TEMP_DATA1,history_temp_data[g_history_temp_cnt-offset-1],strlen(history_temp_data[g_history_temp_cnt-offset-1]));
			write_string(fd_lcd,VAR_TEMP_TIME2,history_temp_time[g_history_temp_cnt-offset-2],strlen(history_temp_time[g_history_temp_cnt-offset-2]));
			write_string(fd_lcd,VAR_TEMP_DATA2,history_temp_data[g_history_temp_cnt-offset-2],strlen(history_temp_data[g_history_temp_cnt-offset-2]));
			write_string(fd_lcd,VAR_TEMP_TIME3,history_temp_time[g_history_temp_cnt-offset-3],strlen(history_temp_time[g_history_temp_cnt-offset-3]));
			write_string(fd_lcd,VAR_TEMP_DATA3,history_temp_data[g_history_temp_cnt-offset-3],strlen(history_temp_data[g_history_temp_cnt-offset-3]));
			write_string(fd_lcd,VAR_TEMP_TIME4,history_temp_time[g_history_temp_cnt-offset-4],strlen(history_temp_time[g_history_temp_cnt-offset-4]));
			write_string(fd_lcd,VAR_TEMP_DATA4,history_temp_data[g_history_temp_cnt-offset-4],strlen(history_temp_data[g_history_temp_cnt-offset-4]));
			write_string(fd_lcd,VAR_TEMP_TIME5,history_temp_time[g_history_temp_cnt-offset-5],strlen(history_temp_time[g_history_temp_cnt-offset-5]));
			write_string(fd_lcd,VAR_TEMP_DATA5,history_temp_data[g_history_temp_cnt-offset-5],strlen(history_temp_data[g_history_temp_cnt-offset-5]));
			write_string(fd_lcd,VAR_TEMP_TIME6,history_temp_time[g_history_temp_cnt-offset-6],strlen(history_temp_time[g_history_temp_cnt-offset-6]));
			write_string(fd_lcd,VAR_TEMP_DATA6,history_temp_data[g_history_temp_cnt-offset-6],strlen(history_temp_data[g_history_temp_cnt-offset-6]));
			write_string(fd_lcd,VAR_TEMP_TIME7,history_temp_time[g_history_temp_cnt-offset-7],strlen(history_temp_time[g_history_temp_cnt-offset-7]));
			write_string(fd_lcd,VAR_TEMP_DATA7,history_temp_data[g_history_temp_cnt-offset-7],strlen(history_temp_data[g_history_temp_cnt-offset-7]));
		}
	}
	if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
		if((g_history_pm25_cnt-offset-7)>0)
		{
			write_string(fd_lcd,VAR_PM25_TIME1,history_pm25_time[g_history_pm25_cnt-offset-1],strlen(history_pm25_time[g_history_pm25_cnt-offset-1]));
			write_string(fd_lcd,VAR_PM25_DATA1,history_pm25_data[g_history_pm25_cnt-offset-1],strlen(history_pm25_data[g_history_pm25_cnt-offset-1]));
			write_string(fd_lcd,VAR_PM25_TIME2,history_pm25_time[g_history_pm25_cnt-offset-2],strlen(history_pm25_time[g_history_pm25_cnt-offset-2]));
			write_string(fd_lcd,VAR_PM25_DATA2,history_pm25_data[g_history_pm25_cnt-offset-2],strlen(history_pm25_data[g_history_pm25_cnt-offset-2]));
			write_string(fd_lcd,VAR_PM25_TIME3,history_pm25_time[g_history_pm25_cnt-offset-3],strlen(history_pm25_time[g_history_pm25_cnt-offset-3]));
			write_string(fd_lcd,VAR_PM25_DATA3,history_pm25_data[g_history_pm25_cnt-offset-3],strlen(history_pm25_data[g_history_pm25_cnt-offset-3]));
			write_string(fd_lcd,VAR_PM25_TIME4,history_pm25_time[g_history_pm25_cnt-offset-4],strlen(history_pm25_time[g_history_pm25_cnt-offset-4]));
			write_string(fd_lcd,VAR_PM25_DATA4,history_pm25_data[g_history_pm25_cnt-offset-4],strlen(history_pm25_data[g_history_pm25_cnt-offset-4]));
			write_string(fd_lcd,VAR_PM25_TIME5,history_pm25_time[g_history_pm25_cnt-offset-5],strlen(history_pm25_time[g_history_pm25_cnt-offset-5]));
			write_string(fd_lcd,VAR_PM25_DATA5,history_pm25_data[g_history_pm25_cnt-offset-5],strlen(history_pm25_data[g_history_pm25_cnt-offset-5]));
			write_string(fd_lcd,VAR_PM25_TIME6,history_pm25_time[g_history_pm25_cnt-offset-6],strlen(history_pm25_time[g_history_pm25_cnt-offset-6]));
			write_string(fd_lcd,VAR_PM25_DATA6,history_pm25_data[g_history_pm25_cnt-offset-6],strlen(history_pm25_data[g_history_pm25_cnt-offset-6]));
			write_string(fd_lcd,VAR_PM25_TIME7,history_pm25_time[g_history_pm25_cnt-offset-7],strlen(history_pm25_time[g_history_pm25_cnt-offset-7]));
			write_string(fd_lcd,VAR_PM25_DATA7,history_pm25_data[g_history_pm25_cnt-offset-7],strlen(history_pm25_data[g_history_pm25_cnt-offset-7]));
		}
	}
	#if 0
	else if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
		write_string(fd_lcd,VAR_CO2_TIME7,history_time[0],strlen(history_time[0]));
		write_string(fd_lcd,VAR_CO2_DATA7,history_data[0],strlen(history_data[0]));
		write_string(fd_lcd,VAR_CO2_TIME6,history_time[1],strlen(history_time[1]));
		write_string(fd_lcd,VAR_CO2_DATA6,history_data[1],strlen(history_data[1]));
		write_string(fd_lcd,VAR_CO2_TIME5,history_time[2],strlen(history_time[2]));
		write_string(fd_lcd,VAR_CO2_DATA5,history_data[2],strlen(history_data[2]));
		write_string(fd_lcd,VAR_CO2_TIME4,history_time[3],strlen(history_time[3]));
		write_string(fd_lcd,VAR_CO2_DATA4,history_data[3],strlen(history_data[3]));
		write_string(fd_lcd,VAR_CO2_TIME3,history_time[4],strlen(history_time[4]));
		write_string(fd_lcd,VAR_CO2_DATA3,history_data[4],strlen(history_data[4]));
		write_string(fd_lcd,VAR_CO2_TIME2,history_time[5],strlen(history_time[5]));
		write_string(fd_lcd,VAR_CO2_DATA2,history_data[5],strlen(history_data[5]));
		write_string(fd_lcd,VAR_CO2_TIME1,history_time[6],strlen(history_time[6]));
		write_string(fd_lcd,VAR_CO2_DATA1,history_data[6],strlen(history_data[6]));
	}
	else if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
		write_string(fd_lcd,VAR_HCHO_TIME1,history_time[0],strlen(history_time[0]));
		write_string(fd_lcd,VAR_HCHO_DATA1,history_data[0],strlen(history_data[0]));
		write_string(fd_lcd,VAR_HCHO_TIME2,history_time[1],strlen(history_time[1]));
		write_string(fd_lcd,VAR_HCHO_DATA2,history_data[1],strlen(history_data[1]));
		write_string(fd_lcd,VAR_HCHO_TIME3,history_time[2],strlen(history_time[2]));
		write_string(fd_lcd,VAR_HCHO_DATA3,history_data[2],strlen(history_data[2]));
		write_string(fd_lcd,VAR_HCHO_TIME4,history_time[3],strlen(history_time[3]));
		write_string(fd_lcd,VAR_HCHO_DATA4,history_data[3],strlen(history_data[3]));
		write_string(fd_lcd,VAR_HCHO_TIME5,history_time[4],strlen(history_time[4]));
		write_string(fd_lcd,VAR_HCHO_DATA5,history_data[4],strlen(history_data[4]));
		write_string(fd_lcd,VAR_HCHO_TIME6,history_time[5],strlen(history_time[5]));
		write_string(fd_lcd,VAR_HCHO_DATA6,history_data[5],strlen(history_data[5]));
		write_string(fd_lcd,VAR_HCHO_TIME7,history_time[6],strlen(history_time[6]));
		write_string(fd_lcd,VAR_HCHO_DATA7,history_data[6],strlen(history_data[6]));
	}
	else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
		write_string(fd_lcd,VAR_TEMP_TIME1,history_time[0],strlen(history_time[0]));
		write_string(fd_lcd,VAR_TEMP_DATA1,history_data[0],strlen(history_data[0]));
		write_string(fd_lcd,VAR_TEMP_TIME2,history_time[1],strlen(history_time[1]));
		write_string(fd_lcd,VAR_TEMP_DATA2,history_data[1],strlen(history_data[1]));
		write_string(fd_lcd,VAR_TEMP_TIME3,history_time[2],strlen(history_time[2]));
		write_string(fd_lcd,VAR_TEMP_DATA3,history_data[2],strlen(history_data[2]));
		write_string(fd_lcd,VAR_TEMP_TIME4,history_time[3],strlen(history_time[3]));
		write_string(fd_lcd,VAR_TEMP_DATA4,history_data[3],strlen(history_data[3]));
		write_string(fd_lcd,VAR_TEMP_TIME5,history_time[4],strlen(history_time[4]));
		write_string(fd_lcd,VAR_TEMP_DATA5,history_data[4],strlen(history_data[4]));
		write_string(fd_lcd,VAR_TEMP_TIME6,history_time[5],strlen(history_time[5]));
		write_string(fd_lcd,VAR_TEMP_DATA6,history_data[5],strlen(history_data[5]));
		write_string(fd_lcd,VAR_TEMP_TIME7,history_time[6],strlen(history_time[6]));
		write_string(fd_lcd,VAR_TEMP_DATA7,history_data[6],strlen(history_data[6]));
	}
	else if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
		write_string(fd_lcd,VAR_SHIDU_TIME1,history_time[0],strlen(history_time[0]));
		write_string(fd_lcd,VAR_SHIDU_DATA1,history_data[0],strlen(history_data[0]));
		write_string(fd_lcd,VAR_SHIDU_TIME2,history_time[1],strlen(history_time[1]));
		write_string(fd_lcd,VAR_SHIDU_DATA2,history_data[1],strlen(history_data[1]));
		write_string(fd_lcd,VAR_SHIDU_TIME3,history_time[2],strlen(history_time[2]));
		write_string(fd_lcd,VAR_SHIDU_DATA3,history_data[2],strlen(history_data[2]));
		write_string(fd_lcd,VAR_SHIDU_TIME4,history_time[3],strlen(history_time[3]));
		write_string(fd_lcd,VAR_SHIDU_DATA4,history_data[3],strlen(history_data[3]));
		write_string(fd_lcd,VAR_SHIDU_TIME5,history_time[4],strlen(history_time[4]));
		write_string(fd_lcd,VAR_SHIDU_DATA5,history_data[4],strlen(history_data[4]));
		write_string(fd_lcd,VAR_SHIDU_TIME6,history_time[5],strlen(history_time[5]));
		write_string(fd_lcd,VAR_SHIDU_DATA6,history_data[5],strlen(history_data[5]));
		write_string(fd_lcd,VAR_SHIDU_TIME7,history_time[6],strlen(history_time[6]));
		write_string(fd_lcd,VAR_SHIDU_DATA7,history_data[6],strlen(history_data[6]));
	}
	else if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
		write_string(fd_lcd,VAR_PM25_TIME1,history_time[0],strlen(history_time[0]));
		write_string(fd_lcd,VAR_PM25_DATA1,history_data[0],strlen(history_data[0]));
		write_string(fd_lcd,VAR_PM25_TIME2,history_time[1],strlen(history_time[1]));
		write_string(fd_lcd,VAR_PM25_DATA2,history_data[1],strlen(history_data[1]));
		write_string(fd_lcd,VAR_PM25_TIME3,history_time[2],strlen(history_time[2]));
		write_string(fd_lcd,VAR_PM25_DATA3,history_data[2],strlen(history_data[2]));
		write_string(fd_lcd,VAR_PM25_TIME4,history_time[3],strlen(history_time[3]));
		write_string(fd_lcd,VAR_PM25_DATA4,history_data[3],strlen(history_data[3]));
		write_string(fd_lcd,VAR_PM25_TIME5,history_time[4],strlen(history_time[4]));
		write_string(fd_lcd,VAR_PM25_DATA5,history_data[4],strlen(history_data[4]));
		write_string(fd_lcd,VAR_PM25_TIME6,history_time[5],strlen(history_time[5]));
		write_string(fd_lcd,VAR_PM25_DATA6,history_data[5],strlen(history_data[5]));
		write_string(fd_lcd,VAR_PM25_TIME7,history_time[6],strlen(history_time[6]));
		write_string(fd_lcd,VAR_PM25_DATA7,history_data[6],strlen(history_data[6]));
	}
	#endif
}
unsigned short input_handle(int fd_lcd,char *input)
{
	int addr=0,data=0;
	static int begin_co=0;
	static int begin_co2=0;
	static int begin_hcho=0;
	static int begin_temp=0;
	static int begin_shidu=0;
	static int begin_pm25=0;
	char * line = NULL;
	char date1[32]={0};
	char date2[32]={0};
	char date3[32]={0};
	char data1[32]={0};
	char data2[32]={0};
	char data3[32]={0};
	char time[]={0x32,0x30,0x31 ,0x35 ,0x2d,0x31 ,0x31 ,0x2d ,0x32 ,0x30 ,0x20,0x32 ,0x37 ,0x3A ,0x32 ,0x30 ,0x3A ,0x30 ,0x30};
	char co[]={0x30,0x2e,0x31,0x32};
	input[0]=2;
	addr=input[1]<<8|input[2];
	data=input[4]<<8|input[5];
	printf(LCD_PROCESS"got press %04x %04x\r\n",addr,data);
	if(addr==TOUCH_DETAIL_CO && (TOUCH_DETAIL_CO-0x100)==data)
	{
		show_history(fd_lcd,"/home/user/history",ID_CAP_CO,0);
		begin_co=0;
	}
	else if(addr==TOUCH_DETAIL_CO2 && (TOUCH_DETAIL_CO2-0x100)==data)
	{
		show_history(fd_lcd,"/home/user/history",ID_CAP_CO2,0);	
		begin_co2=0;
	}	
	else if(addr==TOUCH_DETAIL_HCHO && (TOUCH_DETAIL_HCHO-0x100)==data)
	{
		show_history(fd_lcd,"/home/user/history",ID_CAP_HCHO,0);
		begin_hcho=0;
	}	
	else if(addr==TOUCH_DETAIL_SHIDU && (TOUCH_DETAIL_SHIDU-0x100)==data)
	{
		show_history(fd_lcd,"/home/user/history",ID_CAP_SHI_DU,0);
		begin_shidu=0;
	}	
	else if(addr==TOUCH_DETAIL_TEMP && (TOUCH_DETAIL_TEMP-0x100)==data)
	{
		show_history(fd_lcd,"/home/user/history",ID_CAP_TEMPERATURE,0);
		begin_temp=0;
	}	
	else if(addr==TOUCH_DETAIL_PM25&& (TOUCH_DETAIL_PM25-0x100)==data)
	{
		show_history(fd_lcd,"/home/user/history",ID_CAP_PM_25,0);
		begin_pm25=0;
	}	
	else if(addr==TOUCH_UPDATE_CO && (TOUCH_UPDATE_CO-0x100)==data)
	{
		begin_co+=7;
		show_history(fd_lcd,"/home/user/history",ID_CAP_CO,begin_co);
	}
	else if(addr==TOUCH_UPDATE_CO2 && (TOUCH_UPDATE_CO2-0x100)==data)
	{
		begin_co2+=7;
		show_history(fd_lcd,"/home/user/history",ID_CAP_CO2,begin_co2);
	}
	else if(addr==TOUCH_UPDATE_HCHO && (TOUCH_UPDATE_HCHO-0x100)==data)
	{
		begin_hcho+=7;
		show_history(fd_lcd,"/home/user/history",ID_CAP_HCHO,begin_hcho);
	}
	else if(addr==TOUCH_UPDATE_TEMP && (TOUCH_UPDATE_TEMP-0x100)==data)
	{
		begin_temp+=7;
		show_history(fd_lcd,"/home/user/history",ID_CAP_TEMPERATURE,begin_temp);
	}
	else if(addr==TOUCH_UPDATE_SHIDU&& (TOUCH_UPDATE_SHIDU-0x100)==data)
	{
		begin_shidu+=7;
		show_history(fd_lcd,"/home/user/history",ID_CAP_SHI_DU,begin_shidu);
	}
	else if(addr==TOUCH_UPDATE_PM25 && (TOUCH_UPDATE_PM25-0x100)==data)
	{
		begin_pm25+=7;
		show_history(fd_lcd,"/home/user/history",ID_CAP_PM_25,begin_pm25);
	}
#if 0
	switch(addr)
	{

		case TOUCH_DETAIL_CO:
			{
				if(history_fp!=NULL)
				{

				}
			}
		case TOUCH_UPDATE_CO:
			if((TOUCH_DETAIL_CO-0x100)==data||(TOUCH_UPDATE_CO-0x100)==data)
				return STATE_DETAIL_CO;			
		case TOUCH_DETAIL_CO2:
		case TOUCH_UPDATE_CO2:
			if(TOUCH_DETAIL_CO2-0x100==data||TOUCH_UPDATE_CO2-0x100==data)
				return TOUCH_DETAIL_CO2;
		case TOUCH_DETAIL_HCHO:
		case TOUCH_UPDATE_HCHO:
			if((TOUCH_DETAIL_HCHO-0x100)==data||(TOUCH_UPDATE_HCHO-0x100)==data)
				return STATE_DETAIL_HCHO;
		case TOUCH_DETAIL_TEMP:
		case TOUCH_UPDATE_TEMP:
			if((TOUCH_DETAIL_TEMP-0x100)==data||(TOUCH_UPDATE_TEMP-0x100)==data)
				return STATE_DETAIL_TMP;
		case TOUCH_DETAIL_SHIDU:
		case TOUCH_UPDATE_SHIDU:
			if((TOUCH_DETAIL_SHIDU-0x100)==data||(TOUCH_UPDATE_SHIDU-0x100)==data)
				return STATE_DETAIL_SHIDU;
		case TOUCH_DETAIL_PM25:
		case TOUCH_UPDATE_PM25:
			if((TOUCH_DETAIL_PM25-0x100)==data||(TOUCH_UPDATE_PM25-0x100)==data)
				return STATE_DETAIL_PM25;
		default:
			return STATE_MAIN;

	}
#endif
	return STATE_MAIN;
}
void lcd_loop(int fd)
{	
	char ch;
	int i=1;
	int get=0;
	char ptr[32]={0};	
	switch_pic(fd,0);
	while(1)	
	{	
		if(read(fd,&ch,1)==1)
		{
			printf("<=%x \r\n",ch);
			switch(get)
			{
				case 0:
					if(ch==0x5a)
					{
						printf(LCD_PROCESS"0x5a get ,get =1\r\n");
						get=1;
					}
					break;
				case 1:
					if(ch==0xa5)
					{
						printf(LCD_PROCESS"0xa5 get ,get =2\r\n");
						get=2;

					}
					break;
				case 2:
					if(ch==0x06)
					{
						printf(LCD_PROCESS"0x06 get,get =3\r\n");
						get=3;
						break;
					}
				case 3:
					if(ch==0x83)
					{
						printf(LCD_PROCESS"0x83 get,get =4\r\n");
						get=4;
						i=1;
						break;
					}
				case 4:
					{
						printf(LCD_PROCESS"%02x get ,get =5\r\n",ch);
						ptr[i++]=ch;
						if(i==6)
						{
							get=0;
							ptr[0]=0x01;
							printf(LCD_PROCESS"get %x %x %x %x %x %x\r\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);
							input_handle(fd,ptr);
							printf("enter new loop\n");
						}
					}
					break;			
				default:
					printf(LCD_PROCESS"unknown state\r\n");
					get=0;
					break;						
			}			
		}
	}	
}

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) { 
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E': 
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':  
			newtio.c_cflag &= ~PARENB;
			break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	printf("set done!\n");
	return 0;
}
void dump_curr_time(int fd)
{
	int retval;
	struct rtc_time rtc_tm;

	/* Read the current RTC time/date */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		perror("RTC_RD_TIME ioctl");
		exit(errno);
	}

	printf(MAIN_PROCESS"Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
			rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
			rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
	//if(*out!=NULL)
	//{
	//	sprintf(*out,"%04d-%02d-%02d",rtc_tm.tm_year+1900,rtc_tm.tm_mon+1,rtc_tm.tm_mday);
	//}
}
void set_time(int year,int mon,int day,int hour,int minute,int second)
{	
	int fd, retval;
	struct rtc_time rtc_tm;
	unsigned long data;

	fd = open(RTCDEV, O_RDWR);

	if (fd == -1) {
		perror("RTC open (RTCDEV node missing?)");
		exit(errno);
	}
	rtc_tm.tm_mday = day;
	rtc_tm.tm_mon = mon-1;
	rtc_tm.tm_year = year-1900;
	rtc_tm.tm_hour = hour;
	rtc_tm.tm_min = minute;
	rtc_tm.tm_sec = second;
	/* Read the current RTC time/date */
	retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
	if (retval == -1) {
		perror("RTC_SET_TIME ioctl");
		exit(errno);
	}
	dump_curr_time(fd);
	close(fd);
}
/* Original work from rtc-test example */
int set_alarm(int hour,int mintue,int sec)
{
	int fd, retval;
	struct rtc_time rtc_tm;
	unsigned long data;

	fd = open(RTCDEV, O_RDONLY);

	if (fd == -1) {
		perror("RTC open (RTCDEV node missing?)");
		exit(errno);
	}

	/* Read the current RTC time/date */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		perror("RTC_RD_TIME ioctl");
		exit(errno);
	}

	dump_curr_time(fd);
#if 1
	sprintf(cur_date,"%04d-%02d-%02d.dat",rtc_tm.tm_year+1900,rtc_tm.tm_mon+1,rtc_tm.tm_mday);
	printf(MAIN_PROCESS"cur_date %s\n",cur_date);
	if(rtc_tm.tm_sec!=sec||rtc_tm.tm_min!=mintue||rtc_tm.tm_hour!=hour)
	{
		rtc_tm.tm_sec=sec;
		rtc_tm.tm_min=mintue;
		rtc_tm.tm_hour=hour;
	}
	else
		printf(MAIN_PROCESS"no alarm tm_hour %02d,tm_min %02d,tm_sec %02d\r\n",rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec	);
#else	
	rtc_tm.tm_sec += sec;
	if (rtc_tm.tm_sec >= 60) {
		rtc_tm.tm_sec %= 60;
		rtc_tm.tm_min++;
	}
	if (rtc_tm.tm_min == 60) {
		rtc_tm.tm_min = 0;
		rtc_tm.tm_hour++;
	}
	if (rtc_tm.tm_hour == 24)
		rtc_tm.tm_hour = 0;

	rtc_tm.tm_min +=mintue;
	if (rtc_tm.tm_min == 60) {
		rtc_tm.tm_min = 0;
		rtc_tm.tm_hour++;
	}
	if (rtc_tm.tm_hour == 24)
		rtc_tm.tm_hour = 0;

	rtc_tm.tm_hour +=hour;
	if (rtc_tm.tm_hour == 24)
		rtc_tm.tm_hour = 0;
#endif
	printf(MAIN_PROCESS"tm_hour %02d,tm_min %02d,tm_sec %02d\r\n",rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);		
	retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
	if (retval == -1) {
		perror("RTC_ALM_SET ioctl");
		exit(errno);
	}

	/* Enable alarm interrupts */
	retval = ioctl(fd, RTC_AIE_ON, 0);
	if (retval == -1) {
		perror("RTC_AIE_ON ioctl");
		exit(errno);
	}

	printf(MAIN_PROCESS"Alarm will trigger in %02d:%02d:%02d\n",rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);

	/* This blocks until the alarm ring causes an interrupt */
	retval = read(fd, &data, sizeof(unsigned long));
	if (retval == -1) {
		perror("read");
		exit(errno);
	}

	/* Disable alarm interrupts */
	retval = ioctl(fd, RTC_AIE_OFF, 0);
	if (retval == -1) {
		perror("RTC_AIE_OFF ioctl");
		exit(errno);
	}
	printf(MAIN_PROCESS"Alarm has triggered\n");

	dump_curr_time(fd);
	close(fd);
	return 0;
}
int open_com_port(char *dev)
{
	int fd;
	long  vdisable;
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
		perror("Can't Open Serial ttySAC3");
		return(-1);
	}
	else 
		printf("open tts/0 .....\n");

	if(fcntl(fd, F_SETFL, FNDELAY)<0)
		printf("fcntl failed!\n");
	else
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,FNDELAY));
	if(isatty(STDIN_FILENO)==0)

		printf("standard input is not a terminal device\n");
	else
		printf("isatty success!\n");
	printf("fd-open=%d\n",fd);
	return fd;
}
void set_upload_flag(int a)
{
	g_upload=1;
	alarm(600);
}
void get_sensor_alarm_info()
{
	sensortimes.co=0;
	sensortimes.co2=0;
	sensortimes.hcho=0;
	sensortimes.shidu=0;
	sensortimes.temp=0;
	sensortimes.pm25=0;
	FILE *fp=fopen(CONFIG_FILE,"r");
	if(fp==NULL)
	{
		sensor.co=0;
		sensor.co2=0;
		sensor.hcho=0;
		sensor.shidu=0;
		sensor.temp=0;
		sensor.pm25=0;
		sensor.co_send=0;
		sensor.co2_send=0;
		sensor.hcho_send=0;
		sensor.shidu_send=0;
		sensor.temp_send=0;
		sensor.pm25_send=0;
		fp=fopen(CONFIG_FILE,"w");
		fwrite(&sensor,sizeof(struct _sensor_alarm),1,fp);	
		fclose(fp);
		return;
	}
	fread(&sensor,sizeof(struct _sensor_alarm),1,fp);
	fclose(fp);
	printf(MAIN_PROCESS"GOT Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",sensor.co,sensor.co2,sensor.hcho,sensor.shidu,sensor.temp,sensor.pm25);
}
void save_sensor_alarm_info()
{
	FILE *fp=fopen(CONFIG_FILE,"w");
	fwrite(&sensor,sizeof(struct _sensor_alarm),1,fp);
	fclose(fp);
	printf(MAIN_PROCESS"SAVE Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",sensor.co,sensor.co2,sensor.hcho,sensor.shidu,sensor.temp,sensor.pm25);
}
int main(int argc, char *argv[])
{
	key_t shmid; 
	int fd_com=0,fpid,fd_lcd;
	load_history("/home/user/history");
	get_ip(ip);
	if((shmid = shmget(IPC_PRIVATE, 256, PERM)) == -1 )
	{
		fprintf(stderr, "Create Share Memory Error:%s/n/a", strerror(errno));  
		exit(1);  
	}
	get_sensor_alarm_info();
	server_time = (char *)shmat(shmid, 0, 0);
#ifdef S3C2440
	if((fd_com=open_com_port("/dev/s3c2410_serial1"))<0)
#else
	if((fd_com=open_com_port("/dev/ttySP0"))<0)
#endif
	{
		perror("open_port cap error");
		return -1;
	}
	if(set_opt(fd_com,9600,8,'N',1)<0)
	{
		perror(" set_opt cap error");
		return -1;
	}
	if((fd_lcd=open_com_port("/dev/ttySP1"))<0)
	{
		perror("open_port lcd error");
		close(fd_com);
		return -1;
	}
	if(set_opt(fd_lcd,115200,8,'N',1)<0)
	{
		perror(" set_opt cap error");
		close(fd_com);
		close(fd_lcd);
		return -1;
	}
	if((fd_gprs=open_com_port("/dev/ttySP2"))<0)
	{
		perror("open_port lcd error");
		close(fd_com);
		close(fd_lcd);
		return -1;
	}
	if(set_opt(fd_gprs,115200,8,'N',1)<0)
	{
		perror(" set_opt cap error");
		close(fd_com);
		close(fd_lcd);
		close(fd_gprs);
		return -1;
	}
	pthread_mutex_init(&mutex, NULL);
#if 0
	server_time[0]=0x6c;server_time[1]=ARM_TO_CAP;
	server_time[2]=0x00;server_time[3]=0x01;server_time[4]=0x06;
	server_time[5]=0x0f;server_time[6]=0x0a;
	server_time[7]=0x15;server_time[8]=0x08;
	server_time[9]=0x05;server_time[10]=0x11;
	int crc=CRC_check(server_time,11);
	server_time[11]=(crc&0xff00)>>8;server_time[12]=crc&0x00ff;
	//write(fd,server_time,12);
	write(fd_com,server_time,13);
#endif
	memset(server_time,0,13);
	get_uuid();

	fpid=fork();
	if(fpid==0)
	{
		signal(SIGALRM, set_upload_flag);
		alarm(1);
		while(1)
		{
			get_uart(fd_lcd,fd_com);
		}
	}
	else if(fpid>0)
	{
		fpid=fork();
		if(fpid==0)
		{
			while(1)
			{
				lcd_loop(fd_lcd);
			}	
		}
		else if(fpid>0)
		{
			while(1)
			{
				sync_server(fd_com,0);
				if(server_time[0]!=0 &&server_time[5]!=0)
				{
					set_alarm(00,00,01);
					sync_server(fd_com,1);
				}
				else
					sleep(10);
			}
		}
	}
	return 0;
}
