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
int g_upload=0;
char ip[20]={0};
char cur_date[15]={0};
pthread_mutex_t mutex;
char *post_message=NULL,can_send=0,*warnning_msg=NULL;
char *server_time;
char g_uuid[256]={0};
char send_by_wifi=0;
int fd_gprs=0;
#define ALARM_NONE		0
#define ALARM_BELOW 	1
#define ALARM_UP		2
#define ALARM_UNINSERT 	3

typedef struct _sensor_alarm {
	char co;
	char co2;
	char hcho;
	char shidu;
	char temp;
	char pm25;
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
char *send_web_post(char *url,char *buf,int timeout)
{
	char request[1024]={0};
	int result=0,i,ltimeout=0;
	char *rcv=NULL,ch;
	pthread_mutex_lock(&mutex);
	if(send_by_wifi)
	{		
		sprintf(request,"JSONStr=%s",buf);
		printf(UPLOAD_PROCESS"send web %s\n",request);
		rcv=http_post(url,request,timeout);
	}
	else
	{
		rcv=(char *)malloc(256);
		memset(rcv,'\0',256);
		char *gprs_string=(char *)malloc(strlen(buf)+strlen("POST /saveData/airmessage/messMgr.do HTTP/1.1\r\nHOST: 101.200.182.92:8080\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\n")+30);
		memset(gprs_string,'\0',strlen(buf)+strlen("POST /saveData/airmessage/messMgr.do HTTP/1.1\r\nHOST: 101.200.182.92:8080\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\n")+30);
		strcpy(gprs_string,"POST /saveData/airmessage/messMgr.do HTTP/1.1\r\nHOST: 101.200.182.92:8080\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\n");
		sprintf(length_string,"Content-Length:%d\r\n\r\nJSONStr=",strlen(buf)+8);
		strcat(gprs_string,length_string);
		strcat(gprs_string,buf);
		write(fd_gprs, gprs_string, strlen(gprs_string));	
		i=0;
		while(1)
		{
			if(read(fd_gprs, &ch, 1)==1)
			{
				//rt_kprintf("%c",ch);
				if(ch=='}'||ch=='k')
				{
					rcv[i++]=ch;
					break;
				}
				else if(ch=='{')
					i=0;
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
						printf("gprs timeout\n");						
						free(gprs_string);
						free(rcv);
						return NULL;
					}
				}
			}
		}		
		free(gprs_string);
	}
	if(rcv!=NULL)
		printf(UPLOAD_PROCESS"rcv %s\n\n",rcv);
	else
		printf(UPLOAD_PROCESS"no rcv got\n\n");
	pthread_mutex_unlock(&mutex);
	return rcv;
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
	resend_done=add_item(resend_done,ID_DEVICE_IP_ADDR,"192.168.1.2");
	resend_done=add_item(resend_done,ID_DEVICE_PORT,"9517");
	resend_done=add_item(resend_done,ID_RE_START_TIME,begin);
	resend_done=add_item(resend_done,ID_RE_STOP_TIME,end);
	char *rcv=send_web_post(URL,resend_done,9);
	free(resend_done);
	resend_done=NULL;
	if(rcv!=NULL)
	{	
		int len=strlen(rcv);
		free(rcv);
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
							char *rcv=send_web_post(URL,line,39);
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
							char *rcv=send_web_post(URL,line,9);
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
	rcv=send_web_post(URL,sync_message,9);
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
			//}
			//else if(atoi(type)==6)
			//{
			//}
		}
		free(rcv);
	}
	return ;
}

void get_ip(char *ip)
{
	GetIP_v4_and_v6_linux(AF_INET,ip,16);
	printf("ip addrss %s\n", ip);
	return ;
}
#define CAP_PROCESS "[CAP_PROCESS]"
int get_uart(int fd_lcd,int fd)
{
	#define STATE_IDLE 	0
	#define STATE_6C 	1
	#define STATE_AA 	2
	#define STATE_MESSAGE_TYPE 3
	#define STATE_MESSAGE_LEN 4
	#define STATE_MESSAGE 5
	#define STATE_CRC 6
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
											rcv=send_web_post(URL,warnning_msg,9);
											free(warnning_msg);
											warnning_msg=NULL;
											if(rcv!=NULL)
											{	
												int len=strlen(rcv);
												free(rcv);
											}	
										}
										can_send=1;
										if(g_upload)
										{
											save_to_file(date,post_message);
											//compare 6 sensor max & min 
											
										}
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
											if(to_check[i+3]==ID_CAP_CO2 && !sensor.co2)
												sensor.co2=ALARM_UNINSERT;
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
											//inform server
											sprintf(error,"%dth sensor possible error",to_check[i+3]);
											//char *warnning_msg=NULL;				
											sprintf(id,to_check[i+3]);
											warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
											warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
											warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,"192.168.1.2");
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
												if(g_upload){
												int temp=1;
												for(i=0;i<to_check[i+7];i++)
													temp*=10;
												value=(float((to_check[i+5]<<8|to_check[i+6])))/((float)temp);
												}
											}	
											else
											{
												if(g_upload)
													value=(float)(to_check[i+5]<<8|to_check[i+6]);
											}
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
							printf(CAP_PROCESS"CRC error \r\n");
							for(i=0;i<message_len+7;i++)
								printf("0x%02x ",to_check[i]);
						}					
						if(can_send)
						{
							can_send=0;
							if(g_upload)
							{
								g_upload=0;
								//printf(SUB_PROCESS"send web %s",post_message);
								rcv=send_web_post(URL,post_message,9);
								if(rcv!=NULL)
								{	
									int len=strlen(rcv);
									//printf(SUB_PROCESS"<=== %s %d\n",rcv,len);
									//printf(SUB_PROCESS"send ok\n");
									free(rcv);
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
	printf("\n");
	write(fd,cmd,len+6);
	free(cmd);
}
#define LCD_PROCESS "[LCD_PROCESS]"
FILE *history_fp=NULL;
unsigned short input_handle(int fd_lcd,char *input)
{
	int addr=0,data=0;
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
	if(	addr==TOUCH_DETAIL_CO||
		addr==TOUCH_DETAIL_CO2||
		addr==TOUCH_DETAIL_HCHO||
		addr==TOUCH_DETAIL_TEMP||
		addr==TOUCH_DETAIL_SHIDU||
		addr==TOUCH_DETAIL_PM25)
		{
			if((TOUCH_DETAIL_CO-0x100)==data||
				(TOUCH_DETAIL_CO2-0x100)==data||
				(TOUCH_DETAIL_HCHO-0x100)==data||
				(TOUCH_DETAIL_TEMP-0x100)==data||
				(TOUCH_DETAIL_SHIDU-0x100)==data||
				(TOUCH_DETAIL_PM25-0x100)==data)
			{
				if(history_fp!=NULL)
				{
					fclose(history_fp);
					printf("close the last opened\n");
				}
				char *file_path=(char *)malloc(256);
				memset(file_path,'\0',256);
				int year=2015,mon=11,day=23,hour=9,min=36,sec=12,cod=34;
				sprintf(time,"%04d-%02d-%02d %02d:%02d:%02d",year,mon,day,hour,min,sec);
				sprintf(co,"0.%02d",cod);
				write_string(fd_lcd,0x0006,time,sizeof(time));
				write_string(fd_lcd,0x0018,co,sizeof(co));				
				printf(LCD_PROCESS"cur is %s\n",cur_date);
				strcpy(file_path,FILE_PATH);
				//strcat(file_path,date_buf);
				printf(LCD_PROCESS"to open %s\r\n",file_path);
				history_fp = fopen(file_path, "r");
				if(history_fp!=NULL)
				{
					if(fgets(line,512,history_fp))
					{
						char tmp[17]={'\0'};
						memcpy(tmp,cur_date,10);
						strcat(tmp,line);
						printf(LCD_PROCESS"date %s\n",tmp);
						write_string(fd_lcd,VAR_CO_YEAR1,tmp,sizeof(tmp));
					}
					else
						printf(LCD_PROCESS"fgets failed\n");
				}
				else
					printf(LCD_PROCESS"open file %s failed\n",file_path);
				
			}
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
		fp=fopen(CONFIG_FILE,"w");
		fwrite(&sensor,sizeof(struct _sensor_alarm),1,fp);	
		fclose(fp);
		return;
	}
	fread(&sensor,sizeof(struct _sensor_alarm),1,fp);
	fclose(fp);
	printf(MAIN_PROCESS"GOT Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",sensor.co,sensor.co2,sensor.hcho,sensor.shidu,sensor.temp,sensor.pm25);
}
int main(int argc, char *argv[])
{
	key_t shmid; 
	int fd_com=0,fpid,fd_lcd;
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

