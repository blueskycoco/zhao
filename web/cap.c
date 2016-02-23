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
#include <iconv.h>
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
#include "dwin.h"
//#include <iostream> 
#define RTCDEV "/dev/rtc0"
#define START_BYTE 0x6C
#define CAP_TO_ARM 0xAA
#define ARM_TO_CAP 0xBB
#define VERIFY_BYTE	0x0003
#define RESEND_BYTE	0x0002
#define TIME_BYTE	0x0001
#define ERROR_BYTE	0xFF
#define URL "http://123.57.26.24:8080/saveData/airmessage/messMgr.do"
//#define URL "http://16.168.0.3:8080/saveData/airmessage/messMgr.do"
#define FILE_PATH	"/home/user/history/"
#define MAIN_PROCESS 						"[MainSystem]:"
#define SUB_PROCESS 						"[ChildSystem]:"
//char server_time[20]={0};
void set_time(int year,int mon,int day,int hour,int minute,int second);
void write_data(int fd,unsigned int Index,int data);
void switch_pic(int fd,unsigned char Index);
void write_string(int fd,unsigned int addr,char *data,int len);
void dump_curr_time(int fd);
void save_sensor_alarm_info();
void clear_buf(int fd,int addr,int len);
void lcd_on(int page);

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
key_t shmid_co,shmid_co2,shmid_hcho,shmid_temp,shmid_pm25,shmid_shidu;
key_t shmid_co_cnt,shmid_co2_cnt,shmid_hcho_cnt,shmid_temp_cnt,shmid_pm25_cnt,shmid_shidu_cnt;
struct nano{
	char time[20];
	char data[10];
};
struct nano *g_history_co,*g_history_temp,*g_history_shidu,*g_history_co2,*g_history_hcho,*g_history_pm25;
long *g_co_cnt,*g_co2_cnt,*g_hcho_cnt,*g_temp_cnt,*g_pm25_cnt,*g_shidu_cnt;
char logged=0,g_index=0;
key_t state_shmid;
struct state{
	char network_state;
	char sensor[6];
};
key_t history_load_done_shmid;
int *history_done;
int fd_com=0,fd_lcd=0;
int lcd_state=1;
struct state *g_state;
#define NORMAL_MODE	0
#define TUN_ZERO_MODE	1
#define SENSOR_VERIFY_MODE	2
char factory_mode=NORMAL_MODE;//0 is normall mode , 1 is tun zero mode , 2 is sensor verify mode
int sensor_interface[10]={0};
struct cur_zero_info
{
	int cur_ch2o;
	int cur_co;
	//char cur_ch2o_point=0;
	//char cur_co_point=0;
};
struct cur_zero_info *g_zero_info;
key_t shmid_zero_info;
void buzzer(int fd,int data);
void cut_pic(int fd,int on);
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
		{
			printf(UPLOAD_PROCESS"<==%s\n",*out);
			g_state->network_state=1;
		}
		else
		{
			printf(UPLOAD_PROCESS"<==NULL\n");
			g_state->network_state=0;
		}
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
						g_state->network_state=1;
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
								g_state->network_state=1;
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
							g_state->network_state=0;
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

	sprintf(request,"%s?JSONStr=%s",url,buf);
	printf(UPLOAD_PROCESS"send web %s\n",request);
	rcv=http_get(request,timeout);
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
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) 
	{
		printf("iconv_open failed\n");
		return -1;
	}
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen)==-1)
	{
		printf("iconv failed\n");
		return -1;
	}
	iconv_close(cd);
	return 0;
}
#define SYNC_PREFX "[SYNC_PROCESS]"
void sync_server(int fd,int resend,int set_local)
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
				//printf(SYNC_PREFX"211 %s\r\n",doit_data(rcv,"211"));
				//printf(SYNC_PREFX"212 %s\r\n",doit_data(rcv,"212"));
				if(set_local)
				set_time(server_time[5]+2000,server_time[6],server_time[7],server_time[8],server_time[9],server_time[10]);
				free(starttime);
				char *user_name=doit_data(rcv,"203");
				char *user_place=doit_data(rcv,"211");
				char *user_addr=doit_data(rcv,"200");
				char *user_phone=doit_data(rcv,"202");
				char *user_contraceer=doit_data(rcv,"201");				
				char cmd[256]={0};
					clear_buf(fd_lcd,ADDR_USER_NAME,40);
					clear_buf(fd_lcd,ADDR_INSTALL_PLACE,60);
					clear_buf(fd_lcd,ADDR_USER_ADDR,40);
					clear_buf(fd_lcd,ADDR_USER_PHONE,40);
					clear_buf(fd_lcd,ADDR_USER_CONTACTER,40);
				if(user_name && strlen(user_name)>0)
				{
					code_convert("utf-8","gbk",user_name,strlen(user_name),cmd,256);
					write_string(fd_lcd,ADDR_USER_NAME,cmd,strlen(cmd));
					printf("user_name:%s\n",user_name);
					free(user_name);
				}
				if(user_place && strlen(user_place)>0)
				{		
				    code_convert("utf-8","gbk",user_place,strlen(user_place),cmd,256);
					write_string(fd_lcd,ADDR_INSTALL_PLACE,cmd,strlen(cmd));
					printf("user_place:%s\n",user_place);
					free(user_place);
				}
				if(user_addr && strlen(user_addr)>0)
				{
				
					code_convert("utf-8","gbk",user_addr,strlen(user_addr),cmd,256);
					write_string(fd_lcd,ADDR_USER_ADDR,cmd,strlen(cmd));					
					printf("user_addr:%s\n",user_addr);
					free(user_addr);
				}
				if(user_phone && strlen(user_phone)>0)
				{
					code_convert("utf-8","gbk",user_phone,strlen(user_phone),cmd,256);
					write_string(fd_lcd,ADDR_USER_PHONE,cmd,strlen(cmd));					
					printf("user_phone:%s\n",user_phone);
					free(user_phone);
				}
				if(user_contraceer && strlen(user_contraceer)>0)
				{
					code_convert("utf-8","gbk",user_contraceer,strlen(user_contraceer),cmd,256);
					write_string(fd_lcd,ADDR_USER_CONTACTER,cmd,strlen(cmd));
					printf("user_contraceer:%s\n",user_contraceer);
					free(user_contraceer);
				}
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
int ping_server()
{
	char cmd[256]={0};
	char ret[256]={0};
	FILE *fp;
	sprintf(cmd,"ping -W 1 -c 1 123.57.26.24");
	if((fp=popen(cmd,"r"))!=NULL)
	{
		memset(ret,0,256);
		fread(ret,sizeof(char),sizeof(ret),fp);
		pclose(fp);
		printf("ping return %s %d\n",ret,strlen(ret));
		if(strstr(ret,"from")!=NULL)
		{
			g_state->network_state=1;
			return 1;				
		}
	}
	g_state->network_state=0;
	return 0;
}
void get_ip(char *ip)
{
	GetIP_v4_and_v6_linux(AF_INET,ip,16);
	printf("ip addrss %s\n", ip);
	return ;
}
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
			memset(g_history_co[*g_co_cnt].data,'\0',10);
			strcpy(g_history_co[*g_co_cnt].data,data);					
			memset(g_history_co[*g_co_cnt].time,'\0',20);
			strcpy(g_history_co[*g_co_cnt].time,date);							
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			(*g_co_cnt)++;
		}
		data=doit_data(message,ID_CAP_CO2);
		if(data!=NULL)
		{
			memset(g_history_co2[*g_co2_cnt].data,'\0',10);				
			memset(g_history_co2[*g_co2_cnt].time,'\0',20);
			strcpy(g_history_co2[*g_co2_cnt].time,date);	
			//sprintf(g_history_co2[*g_co2_cnt].data,"%04d",atoi(data));
			strcpy(g_history_co2[*g_co2_cnt].data,data);	
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			(*g_co2_cnt)++;
		}
		data=doit_data(message,ID_CAP_SHI_DU);
		if(data!=NULL)
		{
			memset(g_history_shidu[*g_shidu_cnt].data,'\0',10);
			strcpy(g_history_shidu[*g_shidu_cnt].data,data);					
			memset(g_history_shidu[*g_shidu_cnt].time,'\0',20);
			strcpy(g_history_shidu[*g_shidu_cnt].time,date);							
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			(*g_shidu_cnt)++;
		}
		data=doit_data(message,ID_CAP_HCHO);
		if(data!=NULL)
		{
			memset(g_history_hcho[*g_hcho_cnt].data,'\0',10);
			strcpy(g_history_hcho[*g_hcho_cnt].data,data);					
			memset(g_history_hcho[*g_hcho_cnt].time,'\0',20);
			strcpy(g_history_hcho[*g_hcho_cnt].time,date);							
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			(*g_hcho_cnt)++;
		}
		data=doit_data(message,ID_CAP_TEMPERATURE);
		if(data!=NULL)
		{
			memset(g_history_temp[*g_temp_cnt].data,'\0',10);
			strcpy(g_history_temp[*g_temp_cnt].data,data);					
			memset(g_history_temp[*g_temp_cnt].time,'\0',20);
			strcpy(g_history_temp[*g_temp_cnt].time,date);							
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			(*g_temp_cnt)++;
		}
		data=doit_data(message,ID_CAP_PM_25);
		if(data!=NULL)
		{
			memset(g_history_pm25[*g_pm25_cnt].data,'\0',10);			
			memset(g_history_pm25[*g_pm25_cnt].time,'\0',20);
			strcpy(g_history_pm25[*g_pm25_cnt].time,date);		
			sprintf(g_history_pm25[*g_pm25_cnt].data,"%4d",atoi(data));
			//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
			free(data);
			(*g_pm25_cnt)++;
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
void co_flash_alarm()
{
	if(fork()==0)
	{
		switch_pic(fd_lcd,MAIN_PAGE);
		buzzer(fd_lcd,0x30);
		cut_pic(fd_lcd,1);
		sleep(3);
		buzzer(fd_lcd,0x30);
		cut_pic(fd_lcd,0);
		sleep(3);
		buzzer(fd_lcd,0x30);
		cut_pic(fd_lcd,1);
		sleep(3);
		buzzer(fd_lcd,0x30);
		cut_pic(fd_lcd,0);
		return ;
	}
	return ;
}
void show_factory(int fd,int zero,char *cmd,int len)
{	
	char id[32]={0},data[32]={0},date[32]={0},error[32]={0};
	unsigned int crc=(cmd[len-2]<<8)|cmd[len-1];	
	if(crc==CRC_check(cmd,len-2))
	{
		if(zero)
		{
			if(cmd[3]==atoi(ID_CAP_CO))
			{
				write_data(fd_lcd,ADDR_TUN_ZERO_CO,cmd[5]<<8|cmd[6]);
				g_zero_info->cur_co=(cmd[5]<<8)|cmd[6];
				//g_zero_info->cur_co_point=cmd[7];
			}
			if(cmd[3]==atoi(ID_CAP_HCHO))
			{
				write_data(fd_lcd,ADDR_TUN_ZERO_HCHO,cmd[5]<<8|cmd[6]);
				g_zero_info->cur_ch2o=(cmd[5]<<8)|cmd[6];
				//g_zero_info->cur_ch2o_point=cmd[7];
			}
		}
		else
		{			
			if(cmd[3]==atoi(ID_CAP_PM_25))
				write_data(fd_lcd,ADDR_REAL_VALUE,cmd[5]<<8|cmd[6]);
		}
	}
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
					float value=0;
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
							value=((float)(cmd[5]<<8|cmd[6]))/(float)temp;
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
							//printf("CO2 %f\n",value*10000);
							if(value*10000<MIN_CO2)
								sensortimes.co2++;
							else if(value*10000>MAX_CO2)
								sensortimes.co2++;
							else
							{
								//printf("remove co2 alarm\n");
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
								if(value*10000<MIN_CO2)
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
								co_flash_alarm();
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
						write_data(fd_lcd,ADDR_RUN_TIME_CO,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
					{						
						write_data(fd_lcd,ADDR_RUN_TIME_CO2,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_HCHO,strlen(ID_CAP_HCHO))==0)
					{
						write_data(fd_lcd,ADDR_RUN_TIME_HCHO,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(ID_CAP_TEMPERATURE))==0)
					{
						write_data(fd_lcd,ADDR_RUN_TIME_TEMP,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_SHI_DU,strlen(ID_CAP_SHI_DU))==0)
					{
						write_data(fd_lcd,ADDR_RUN_TIME_SHIDU,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
					{
						write_data(fd_lcd,ADDR_RUN_TIME_PM25,cmd[5]<<8|cmd[6]);
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
					if(factory_mode==NORMAL_MODE)
						post_message=build_message(fd,fd_lcd,cmd,message_len+7,post_message);
					else if(factory_mode==TUN_ZERO_MODE)
						show_factory(fd_lcd,1,cmd,message_len+7);
					else
						show_factory(fd_lcd,0,cmd,message_len+7);
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

void switch_pic(int fd,unsigned char Index)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x04,0x00};
	cmd[5]=Index;
	write(fd,cmd,6);
}
void buzzer(int fd,int data)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x02,0x00};
	cmd[5]=data;
	write(fd,cmd,6);
}
void cut_pic(int fd,int on)
{
	char cmd[]={0x5a,0xa5,0x15,0x82,0x06,0xf3,0x00,0x06,0x00,0x01,0x00,0x1c,
				0x01,0x2f,0x00,0x58,0x01,0x8e,0x00,0x7b,0x00,0x35,0x00,0x5c};
	
	char off[]={0x5a,0xa5,0x15,0x82,0x06,0xf3,0x00,0x06,0x00,0x01,0x00,0x01,
				0x00,0x35,0x00,0x5c,0x00,0x94,0x00,0x83,0x00,0x35,0x00,0x5c};
	if(on)
		write(fd,cmd,sizeof(cmd));
	else
		write(fd,off,sizeof(off));
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
	long ii;
	printf(LCD_PROCESS"begin to shmat1\n");
	g_history_co = (struct nano *)shmat(shmid_co, 0, 0);
	g_history_co2 = (struct nano *)shmat(shmid_co2, 0, 0);
	g_history_hcho = (struct nano *)shmat(shmid_hcho, 0, 0);
	g_history_shidu = (struct nano *)shmat(shmid_shidu, 0, 0);
	g_history_temp = (struct nano *)shmat(shmid_temp, 0, 0);
	g_history_pm25 = (struct nano *)shmat(shmid_pm25, 0, 0);
	g_co_cnt = (long *)shmat(shmid_co_cnt, 0, 0);
	g_co2_cnt = (long *)shmat(shmid_co2_cnt, 0, 0);
	g_hcho_cnt = (long *)shmat(shmid_hcho_cnt, 0, 0);
	g_temp_cnt = (long *)shmat(shmid_temp_cnt, 0, 0);
	g_shidu_cnt = (long *)shmat(shmid_shidu_cnt, 0, 0);
	g_pm25_cnt = (long *)shmat(shmid_pm25_cnt, 0, 0);
	printf(LCD_PROCESS"end to shmat\n");
	*history_done=0;
	printf("load=>history_done %d\n",*history_done);
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
					memset(g_history_co[*g_co_cnt].data,'\0',10);
					strcpy(g_history_co[*g_co_cnt].data,data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					//printf(LCD_PROCESS"g_co_cnt in load %d %s data %s\n",*g_co_cnt,g_history_co[*g_co_cnt].data,data);
					free(data);
					(*g_co_cnt)++;
				}
				data=doit_data(line,ID_CAP_CO2);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(g_history_co2[*g_co2_cnt].data,'\0',10);
					strcpy(g_history_co2[*g_co2_cnt].data,data);					
					//sprintf(g_history_co2[*g_co2_cnt].data,"%04d",atoi(data));
					free(data);
					(*g_co2_cnt)++;
				}
				data=doit_data(line,ID_CAP_HCHO);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(g_history_hcho[*g_hcho_cnt].data,'\0',10);
					strcpy(g_history_hcho[*g_hcho_cnt].data,data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					free(data);
					(*g_hcho_cnt)++;
				}
				data=doit_data(line,ID_CAP_SHI_DU);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(g_history_shidu[*g_shidu_cnt].data,'\0',10);
					strcpy(g_history_shidu[*g_shidu_cnt].data,data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					free(data);
					(*g_shidu_cnt)++;
				}
				data=doit_data(line,ID_CAP_TEMPERATURE);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(g_history_temp[*g_temp_cnt].data,'\0',10);
					strcpy(g_history_temp[*g_temp_cnt].data,data);					
					//sprintf(history_co_data[g_history_co_cnt],"%04d",atoi(co));
					free(data);
					(*g_temp_cnt)++;
				}
				data=doit_data(line,ID_CAP_PM_25);
				if(data!=NULL)
				{
					//printf(LCD_PROCESS"<co>%s\n",co);
					memset(g_history_pm25[*g_pm25_cnt].data,'\0',10);
					//strcpy(history_pm25_data[g_history_pm25_cnt],data);					
					sprintf(g_history_pm25[*g_pm25_cnt].data,"%4d",atoi(data));
					free(data);
					(*g_pm25_cnt)++;
				}
			}
			else
			{
				char tmp[11]={0};
				memset(g_history_co[*g_co_cnt].time,'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(g_history_co[*g_co_cnt].time,tmp);
				strcat(g_history_co[*g_co_cnt].time," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(g_history_co[*g_co_cnt].time,tmp);
				
				memset(g_history_co2[*g_co2_cnt].time,'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(g_history_co2[*g_co2_cnt].time,tmp);
				strcat(g_history_co2[*g_co2_cnt].time," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(g_history_co2[*g_co2_cnt].time,tmp);
				
				memset(g_history_temp[*g_temp_cnt].time,'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(g_history_temp[*g_temp_cnt].time,tmp);
				strcat(g_history_temp[*g_temp_cnt].time," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(g_history_temp[*g_temp_cnt].time,tmp);
				
				memset(g_history_hcho[*g_hcho_cnt].time,'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(g_history_hcho[*g_hcho_cnt].time,tmp);
				strcat(g_history_hcho[*g_hcho_cnt].time," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(g_history_hcho[*g_hcho_cnt].time,tmp);
				
				memset(g_history_shidu[*g_shidu_cnt].time,'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(g_history_shidu[*g_shidu_cnt].time,tmp);
				strcat(g_history_shidu[*g_shidu_cnt].time," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(g_history_shidu[*g_shidu_cnt].time,tmp);
				
				memset(g_history_pm25[*g_pm25_cnt].time,'\0',20);
				memcpy(tmp,file_list[j],10);
				strcpy(g_history_pm25[*g_pm25_cnt].time,tmp);
				strcat(g_history_pm25[*g_pm25_cnt].time," ");
				memset(tmp,'\0',11);
				memcpy(tmp,line,5);
				strcat(g_history_pm25[*g_pm25_cnt].time,tmp);
			}
			cnt++;
		}
		fclose(fp);
	}
	//for(i=0;i<g_history_co_cnt;i++)
		//printf(LCD_PROCESS"[%d]time %s, data %s\n",i,history_co_time[i],history_co_data[i]);
	*history_done=1;
	printf("load=>history_done %d\n",*history_done);
}
void show_sensor_network(int fd)
{
	int pic=0;
	ping_server();
	printf("sensor %d %d %d %d %d %d\nnet_work %d\n",g_state->sensor[0],g_state->sensor[1],g_state->sensor[2],g_state->sensor[3],g_state->sensor[4],g_state->sensor[5],
		g_state->network_state);
	if((g_state->sensor[0] || g_state->sensor[1] || g_state->sensor[2] || g_state->sensor[3] || g_state->sensor[4] || g_state->sensor[5])&&(!g_state->network_state))
		pic=STATE_E_E_PAGE;
	else if((g_state->sensor[0] || g_state->sensor[1] || g_state->sensor[2] || g_state->sensor[3] || g_state->sensor[4] || g_state->sensor[5])&&(g_state->network_state))
		pic=STATE_E_O_PAGE;
	else if(!(g_state->sensor[0] || g_state->sensor[1] || g_state->sensor[2] || g_state->sensor[3] || g_state->sensor[4] || g_state->sensor[5])&&(g_state->network_state))
		pic=STATE_O_O_PAGE;
	else if(!(g_state->sensor[0] || g_state->sensor[1] || g_state->sensor[2] || g_state->sensor[3] || g_state->sensor[4] || g_state->sensor[5])&&(!g_state->network_state))
		pic=STATE_O_E_PAGE;
	switch_pic(fd,pic);
}
void show_history(int fd_lcd,char *id,int offset)
{	
	char tmp[4]={0};
	if(strncmp(id,ID_CAP_CO,strlen(id))==0)
	{
//		printf("g_co_cnt %d\n",*g_co_cnt);
		if((*g_co_cnt-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7);
			write_string(fd_lcd,ADDR_CO_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",*g_co_cnt/7);
			write_string(fd_lcd,ADDR_CO_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(fd_lcd,ADDR_CO_LIST_TIME_0,g_history_co[*g_co_cnt-offset-1].time,strlen(g_history_co[*g_co_cnt-offset-1].time));
			write_string(fd_lcd,ADDR_CO_LIST_DATA_0,g_history_co[*g_co_cnt-offset-1].data,strlen(g_history_co[*g_co_cnt-offset-1].data));
			write_string(fd_lcd,ADDR_CO_LIST_TIME_1,g_history_co[*g_co_cnt-offset-2].time,strlen(g_history_co[*g_co_cnt-offset-2].time));
			write_string(fd_lcd,ADDR_CO_LIST_DATA_1,g_history_co[*g_co_cnt-offset-2].data,strlen(g_history_co[*g_co_cnt-offset-2].data));
			write_string(fd_lcd,ADDR_CO_LIST_TIME_2,g_history_co[*g_co_cnt-offset-3].time,strlen(g_history_co[*g_co_cnt-offset-3].time));
			write_string(fd_lcd,ADDR_CO_LIST_DATA_2,g_history_co[*g_co_cnt-offset-3].data,strlen(g_history_co[*g_co_cnt-offset-3].data));
			write_string(fd_lcd,ADDR_CO_LIST_TIME_3,g_history_co[*g_co_cnt-offset-4].time,strlen(g_history_co[*g_co_cnt-offset-4].time));
			write_string(fd_lcd,ADDR_CO_LIST_DATA_3,g_history_co[*g_co_cnt-offset-4].data,strlen(g_history_co[*g_co_cnt-offset-4].data));
			write_string(fd_lcd,ADDR_CO_LIST_TIME_4,g_history_co[*g_co_cnt-offset-5].time,strlen(g_history_co[*g_co_cnt-offset-5].time));
			write_string(fd_lcd,ADDR_CO_LIST_DATA_4,g_history_co[*g_co_cnt-offset-5].data,strlen(g_history_co[*g_co_cnt-offset-5].data));
			write_string(fd_lcd,ADDR_CO_LIST_TIME_5,g_history_co[*g_co_cnt-offset-6].time,strlen(g_history_co[*g_co_cnt-offset-6].time));
			write_string(fd_lcd,ADDR_CO_LIST_DATA_5,g_history_co[*g_co_cnt-offset-6].data,strlen(g_history_co[*g_co_cnt-offset-6].data));
			write_string(fd_lcd,ADDR_CO_LIST_TIME_6,g_history_co[*g_co_cnt-offset-7].time,strlen(g_history_co[*g_co_cnt-offset-7].time));
			write_string(fd_lcd,ADDR_CO_LIST_DATA_6,g_history_co[*g_co_cnt-offset-7].data,strlen(g_history_co[*g_co_cnt-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
//		printf("g_co2_cnt %d\n",*g_co2_cnt);
		if((*g_co2_cnt-offset-7)>0)
		{			
			sprintf(tmp,"%4d",offset/7);
			write_string(fd_lcd,ADDR_CO2_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",*g_co2_cnt/7);			
			write_string(fd_lcd,ADDR_CO2_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(fd_lcd,ADDR_CO2_LIST_TIME_0,g_history_co2[*g_co2_cnt-offset-1].time,strlen(g_history_co2[*g_co2_cnt-offset-1].time));
			write_string(fd_lcd,ADDR_CO2_LIST_DATA_0,g_history_co2[*g_co2_cnt-offset-1].data,strlen(g_history_co2[*g_co2_cnt-offset-1].data));
			write_string(fd_lcd,ADDR_CO2_LIST_TIME_1,g_history_co2[*g_co2_cnt-offset-2].time,strlen(g_history_co2[*g_co2_cnt-offset-2].time));
			write_string(fd_lcd,ADDR_CO2_LIST_DATA_1,g_history_co2[*g_co2_cnt-offset-2].data,strlen(g_history_co2[*g_co2_cnt-offset-2].data));
			write_string(fd_lcd,ADDR_CO2_LIST_TIME_2,g_history_co2[*g_co2_cnt-offset-3].time,strlen(g_history_co2[*g_co2_cnt-offset-3].time));
			write_string(fd_lcd,ADDR_CO2_LIST_DATA_2,g_history_co2[*g_co2_cnt-offset-3].data,strlen(g_history_co2[*g_co2_cnt-offset-3].data));
			write_string(fd_lcd,ADDR_CO2_LIST_TIME_3,g_history_co2[*g_co2_cnt-offset-4].time,strlen(g_history_co2[*g_co2_cnt-offset-4].time));
			write_string(fd_lcd,ADDR_CO2_LIST_DATA_3,g_history_co2[*g_co2_cnt-offset-4].data,strlen(g_history_co2[*g_co2_cnt-offset-4].data));
			write_string(fd_lcd,ADDR_CO2_LIST_TIME_4,g_history_co2[*g_co2_cnt-offset-5].time,strlen(g_history_co2[*g_co2_cnt-offset-5].time));
			write_string(fd_lcd,ADDR_CO2_LIST_DATA_4,g_history_co2[*g_co2_cnt-offset-5].data,strlen(g_history_co2[*g_co2_cnt-offset-5].data));
			write_string(fd_lcd,ADDR_CO2_LIST_TIME_5,g_history_co2[*g_co2_cnt-offset-6].time,strlen(g_history_co2[*g_co2_cnt-offset-6].time));
			write_string(fd_lcd,ADDR_CO2_LIST_DATA_5,g_history_co2[*g_co2_cnt-offset-6].data,strlen(g_history_co2[*g_co2_cnt-offset-6].data));
			write_string(fd_lcd,ADDR_CO2_LIST_TIME_6,g_history_co2[*g_co2_cnt-offset-7].time,strlen(g_history_co2[*g_co2_cnt-offset-7].time));
			write_string(fd_lcd,ADDR_CO2_LIST_DATA_6,g_history_co2[*g_co2_cnt-offset-7].data,strlen(g_history_co2[*g_co2_cnt-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
//		printf("g_co_cnt %d\n",*g_hcho_cnt);
		if((*g_hcho_cnt-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7);
			write_string(fd_lcd,ADDR_HCHO_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",*g_hcho_cnt/7);			
			write_string(fd_lcd,ADDR_HCHO_PAGE_TOTAL,tmp,strlen(tmp));		
			write_string(fd_lcd,ADDR_HCHO_LIST_TIME_0,g_history_hcho[*g_hcho_cnt-offset-1].time,strlen(g_history_hcho[*g_hcho_cnt-offset-1].time));
			write_string(fd_lcd,ADDR_HCHO_LIST_DATA_0,g_history_hcho[*g_hcho_cnt-offset-1].data,strlen(g_history_hcho[*g_hcho_cnt-offset-1].data));
			write_string(fd_lcd,ADDR_HCHO_LIST_TIME_1,g_history_hcho[*g_hcho_cnt-offset-2].time,strlen(g_history_hcho[*g_hcho_cnt-offset-2].time));
			write_string(fd_lcd,ADDR_HCHO_LIST_DATA_1,g_history_hcho[*g_hcho_cnt-offset-2].data,strlen(g_history_hcho[*g_hcho_cnt-offset-2].data));
			write_string(fd_lcd,ADDR_HCHO_LIST_TIME_2,g_history_hcho[*g_hcho_cnt-offset-3].time,strlen(g_history_hcho[*g_hcho_cnt-offset-3].time));
			write_string(fd_lcd,ADDR_HCHO_LIST_DATA_2,g_history_hcho[*g_hcho_cnt-offset-3].data,strlen(g_history_hcho[*g_hcho_cnt-offset-3].data));
			write_string(fd_lcd,ADDR_HCHO_LIST_TIME_3,g_history_hcho[*g_hcho_cnt-offset-4].time,strlen(g_history_hcho[*g_hcho_cnt-offset-4].time));
			write_string(fd_lcd,ADDR_HCHO_LIST_DATA_3,g_history_hcho[*g_hcho_cnt-offset-4].data,strlen(g_history_hcho[*g_hcho_cnt-offset-4].data));
			write_string(fd_lcd,ADDR_HCHO_LIST_TIME_4,g_history_hcho[*g_hcho_cnt-offset-5].time,strlen(g_history_hcho[*g_hcho_cnt-offset-5].time));
			write_string(fd_lcd,ADDR_HCHO_LIST_DATA_4,g_history_hcho[*g_hcho_cnt-offset-5].data,strlen(g_history_hcho[*g_hcho_cnt-offset-5].data));
			write_string(fd_lcd,ADDR_HCHO_LIST_TIME_5,g_history_hcho[*g_hcho_cnt-offset-6].time,strlen(g_history_hcho[*g_hcho_cnt-offset-6].time));
			write_string(fd_lcd,ADDR_HCHO_LIST_DATA_5,g_history_hcho[*g_hcho_cnt-offset-6].data,strlen(g_history_hcho[*g_hcho_cnt-offset-6].data));
			write_string(fd_lcd,ADDR_HCHO_LIST_TIME_6,g_history_hcho[*g_hcho_cnt-offset-7].time,strlen(g_history_hcho[*g_hcho_cnt-offset-7].time));
			write_string(fd_lcd,ADDR_HCHO_LIST_DATA_6,g_history_hcho[*g_hcho_cnt-offset-7].data,strlen(g_history_hcho[*g_hcho_cnt-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
//		printf("g_shidu_cnt %d\n",*g_shidu_cnt);
		if((*g_shidu_cnt-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7);
			write_string(fd_lcd,ADDR_SHIDU_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",*g_shidu_cnt/7);			
			write_string(fd_lcd,ADDR_SHIDU_PAGE_TOTAL,tmp,strlen(tmp));		
			write_string(fd_lcd,ADDR_SHIDU_LIST_TIME_0,g_history_shidu[*g_shidu_cnt-offset-1].time,strlen(g_history_shidu[*g_shidu_cnt-offset-1].time));
			write_string(fd_lcd,ADDR_SHIDU_LIST_DATA_0,g_history_shidu[*g_shidu_cnt-offset-1].data,strlen(g_history_shidu[*g_shidu_cnt-offset-1].data));
			write_string(fd_lcd,ADDR_SHIDU_LIST_TIME_1,g_history_shidu[*g_shidu_cnt-offset-2].time,strlen(g_history_shidu[*g_shidu_cnt-offset-2].time));
			write_string(fd_lcd,ADDR_SHIDU_LIST_DATA_1,g_history_shidu[*g_shidu_cnt-offset-2].data,strlen(g_history_shidu[*g_shidu_cnt-offset-2].data));
			write_string(fd_lcd,ADDR_SHIDU_LIST_TIME_2,g_history_shidu[*g_shidu_cnt-offset-3].time,strlen(g_history_shidu[*g_shidu_cnt-offset-3].time));
			write_string(fd_lcd,ADDR_SHIDU_LIST_DATA_2,g_history_shidu[*g_shidu_cnt-offset-3].data,strlen(g_history_shidu[*g_shidu_cnt-offset-3].data));
			write_string(fd_lcd,ADDR_SHIDU_LIST_TIME_3,g_history_shidu[*g_shidu_cnt-offset-4].time,strlen(g_history_shidu[*g_shidu_cnt-offset-4].time));
			write_string(fd_lcd,ADDR_SHIDU_LIST_DATA_3,g_history_shidu[*g_shidu_cnt-offset-4].data,strlen(g_history_shidu[*g_shidu_cnt-offset-4].data));
			write_string(fd_lcd,ADDR_SHIDU_LIST_TIME_4,g_history_shidu[*g_shidu_cnt-offset-5].time,strlen(g_history_shidu[*g_shidu_cnt-offset-5].time));
			write_string(fd_lcd,ADDR_SHIDU_LIST_DATA_4,g_history_shidu[*g_shidu_cnt-offset-5].data,strlen(g_history_shidu[*g_shidu_cnt-offset-5].data));
			write_string(fd_lcd,ADDR_SHIDU_LIST_TIME_5,g_history_shidu[*g_shidu_cnt-offset-6].time,strlen(g_history_shidu[*g_shidu_cnt-offset-6].time));
			write_string(fd_lcd,ADDR_SHIDU_LIST_DATA_5,g_history_shidu[*g_shidu_cnt-offset-6].data,strlen(g_history_shidu[*g_shidu_cnt-offset-6].data));
			write_string(fd_lcd,ADDR_SHIDU_LIST_TIME_6,g_history_shidu[*g_shidu_cnt-offset-7].time,strlen(g_history_shidu[*g_shidu_cnt-offset-7].time));
			write_string(fd_lcd,ADDR_SHIDU_LIST_DATA_6,g_history_shidu[*g_shidu_cnt-offset-7].data,strlen(g_history_shidu[*g_shidu_cnt-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
//		printf("g_temp_cnt %d\n",*g_temp_cnt);
		if((*g_temp_cnt-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7);
			write_string(fd_lcd,ADDR_TEMP_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",*g_temp_cnt/7);			
			write_string(fd_lcd,ADDR_TEMP_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(fd_lcd,ADDR_TEMP_LIST_TIME_0,g_history_temp[*g_temp_cnt-offset-1].time,strlen(g_history_temp[*g_temp_cnt-offset-1].time));
			write_string(fd_lcd,ADDR_TEMP_LIST_DATA_0,g_history_temp[*g_temp_cnt-offset-1].data,strlen(g_history_temp[*g_temp_cnt-offset-1].data));
			write_string(fd_lcd,ADDR_TEMP_LIST_TIME_1,g_history_temp[*g_temp_cnt-offset-2].time,strlen(g_history_temp[*g_temp_cnt-offset-2].time));
			write_string(fd_lcd,ADDR_TEMP_LIST_DATA_1,g_history_temp[*g_temp_cnt-offset-2].data,strlen(g_history_temp[*g_temp_cnt-offset-2].data));
			write_string(fd_lcd,ADDR_TEMP_LIST_TIME_2,g_history_temp[*g_temp_cnt-offset-3].time,strlen(g_history_temp[*g_temp_cnt-offset-3].time));
			write_string(fd_lcd,ADDR_TEMP_LIST_DATA_2,g_history_temp[*g_temp_cnt-offset-3].data,strlen(g_history_temp[*g_temp_cnt-offset-3].data));
			write_string(fd_lcd,ADDR_TEMP_LIST_TIME_3,g_history_temp[*g_temp_cnt-offset-4].time,strlen(g_history_temp[*g_temp_cnt-offset-4].time));
			write_string(fd_lcd,ADDR_TEMP_LIST_DATA_3,g_history_temp[*g_temp_cnt-offset-4].data,strlen(g_history_temp[*g_temp_cnt-offset-4].data));
			write_string(fd_lcd,ADDR_TEMP_LIST_TIME_4,g_history_temp[*g_temp_cnt-offset-5].time,strlen(g_history_temp[*g_temp_cnt-offset-5].time));
			write_string(fd_lcd,ADDR_TEMP_LIST_DATA_4,g_history_temp[*g_temp_cnt-offset-5].data,strlen(g_history_temp[*g_temp_cnt-offset-5].data));
			write_string(fd_lcd,ADDR_TEMP_LIST_TIME_5,g_history_temp[*g_temp_cnt-offset-6].time,strlen(g_history_temp[*g_temp_cnt-offset-6].time));
			write_string(fd_lcd,ADDR_TEMP_LIST_DATA_5,g_history_temp[*g_temp_cnt-offset-6].data,strlen(g_history_temp[*g_temp_cnt-offset-6].data));
			write_string(fd_lcd,ADDR_TEMP_LIST_TIME_6,g_history_temp[*g_temp_cnt-offset-7].time,strlen(g_history_temp[*g_temp_cnt-offset-7].time));
			write_string(fd_lcd,ADDR_TEMP_LIST_DATA_6,g_history_temp[*g_temp_cnt-offset-7].data,strlen(g_history_temp[*g_temp_cnt-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
//		printf("g_pm25_cnt %d\n",*g_pm25_cnt);
		if((*g_pm25_cnt-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7);
			write_string(fd_lcd,ADDR_PM25_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",*g_pm25_cnt/7);			
			write_string(fd_lcd,ADDR_PM25_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(fd_lcd,ADDR_PM25_LIST_TIME_0,g_history_pm25[*g_pm25_cnt-offset-1].time,strlen(g_history_pm25[*g_pm25_cnt-offset-1].time));
			write_string(fd_lcd,ADDR_PM25_LIST_DATA_0,g_history_pm25[*g_pm25_cnt-offset-1].data,strlen(g_history_pm25[*g_pm25_cnt-offset-1].data));
			write_string(fd_lcd,ADDR_PM25_LIST_TIME_1,g_history_pm25[*g_pm25_cnt-offset-2].time,strlen(g_history_pm25[*g_pm25_cnt-offset-2].time));
			write_string(fd_lcd,ADDR_PM25_LIST_DATA_1,g_history_pm25[*g_pm25_cnt-offset-2].data,strlen(g_history_pm25[*g_pm25_cnt-offset-2].data));
			write_string(fd_lcd,ADDR_PM25_LIST_TIME_2,g_history_pm25[*g_pm25_cnt-offset-3].time,strlen(g_history_pm25[*g_pm25_cnt-offset-3].time));
			write_string(fd_lcd,ADDR_PM25_LIST_DATA_2,g_history_pm25[*g_pm25_cnt-offset-3].data,strlen(g_history_pm25[*g_pm25_cnt-offset-3].data));
			write_string(fd_lcd,ADDR_PM25_LIST_TIME_3,g_history_pm25[*g_pm25_cnt-offset-4].time,strlen(g_history_pm25[*g_pm25_cnt-offset-4].time));
			write_string(fd_lcd,ADDR_PM25_LIST_DATA_3,g_history_pm25[*g_pm25_cnt-offset-4].data,strlen(g_history_pm25[*g_pm25_cnt-offset-4].data));
			write_string(fd_lcd,ADDR_PM25_LIST_TIME_4,g_history_pm25[*g_pm25_cnt-offset-5].time,strlen(g_history_pm25[*g_pm25_cnt-offset-5].time));
			write_string(fd_lcd,ADDR_PM25_LIST_DATA_4,g_history_pm25[*g_pm25_cnt-offset-5].data,strlen(g_history_pm25[*g_pm25_cnt-offset-5].data));
			write_string(fd_lcd,ADDR_PM25_LIST_TIME_5,g_history_pm25[*g_pm25_cnt-offset-6].time,strlen(g_history_pm25[*g_pm25_cnt-offset-6].time));
			write_string(fd_lcd,ADDR_PM25_LIST_DATA_5,g_history_pm25[*g_pm25_cnt-offset-6].data,strlen(g_history_pm25[*g_pm25_cnt-offset-6].data));
			write_string(fd_lcd,ADDR_PM25_LIST_TIME_6,g_history_pm25[*g_pm25_cnt-offset-7].time,strlen(g_history_pm25[*g_pm25_cnt-offset-7].time));
			write_string(fd_lcd,ADDR_PM25_LIST_DATA_6,g_history_pm25[*g_pm25_cnt-offset-7].data,strlen(g_history_pm25[*g_pm25_cnt-offset-7].data));
		}
	}
}
void draw_curve(int fd,int *data,int len)
{
	//char cmd[]={0x5a,0xa5,0x00,0x84,0x01};
	int i;
	char *cmd=(char *)malloc(len*2+5);
	cmd[0]=0x5a;cmd[1]=0xa5;cmd[2]=len*2+2;cmd[3]=0x84;cmd[4]=0x01;
	for(i=0;i<len;i++)
	{
		cmd[5+2*i]=(data[i]&0xff00)>>8;
		cmd[6+2*i]=data[i]&0x00ff;
	}
	for(i=0;i<len*2+5;i++)
		printf("%02x ",cmd[i]);
	printf("\n<len %d>\n",len);
	write(fd,cmd,len*2+5);
	free(cmd);
}
void clear_curve(int fd)
{
	char cmd[]={0x5a,0xa5,0x02,0xeb,0x5a};
	write(fd,cmd,5);
}
void show_curve(int fd,char *id,int* offset)
{
	int buf[24]={0};
	char info[20]={0};
	char temp[10]={0},temp2[10]={0};
	char hour1[3]={0},hour2[3]={0};
	char index[5][17]={0};
	char index_time[5][3]={0};
	int i=0,j=0,m=0;
	clear_curve(fd);
	if(strncmp(id,ID_CAP_CO,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_co_cnt);
		if((*g_co_cnt-*offset)>0)
		{
			strcpy(index[0],"200");
			strcpy(index[1],"150");
			strcpy(index[2],"100");
			strcpy(index[3],"050");
			strcpy(index[4],"000");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"CO");
			write_string(fd, ADDR_CURVE_DATE, g_history_co[*g_co_cnt-*offset-1].time,10);
			memcpy(temp,g_history_co[*g_co_cnt-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,g_history_co[*g_co_cnt-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,g_history_co[*g_co_cnt-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printf("hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(g_history_co[*g_co_cnt-*offset-i-1].data)*5;
								printf("j %d %s==>%d\n",j,g_history_co[*g_co_cnt-*offset-i-1].time,buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,g_history_co[*g_co_cnt-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printf("j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			*offset+=i;
			if(*offset>=*g_co_cnt)
				*offset=0;
		}
		else
			*offset=0;
	}
	if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
		//printf("g_co2_cnt %d\n",*g_co2_cnt);
		if((*g_co2_cnt-*offset)>0)
		{
			strcpy(index[0],"0.2000");
			strcpy(index[1],"0.1500");
			strcpy(index[2],"0.1000");
			strcpy(index[3],"0.0500");
			strcpy(index[4],"0.0000");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"CO2");
			write_string(fd, ADDR_CURVE_DATE, g_history_co2[*g_co2_cnt-*offset-1].time,10);
			memcpy(temp,g_history_co2[*g_co2_cnt-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,g_history_co2[*g_co2_cnt-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,g_history_co2[*g_co2_cnt-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printf("hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(g_history_co2[*g_co2_cnt-*offset-i-1].data+2);
								printf("j %d %s==>%d\n",j,g_history_co2[*g_co2_cnt-*offset-i-1].time,buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,g_history_co2[*g_co2_cnt-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printf("j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			*offset+=i;
			if(*offset>=*g_co2_cnt)
				*offset=0;
		}
		else
			*offset=0;
	}
	if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		if((*g_hcho_cnt-*offset)>0)
		{
			strcpy(index[0],"003");
			strcpy(index[1],"2.5");
			strcpy(index[2],"1.5");
			strcpy(index[3],"001");
			strcpy(index[4],"000");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"HCHO");
			write_string(fd, ADDR_CURVE_DATE, g_history_hcho[*g_hcho_cnt-*offset-1].time,10);
			memcpy(temp,g_history_hcho[*g_hcho_cnt-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,g_history_hcho[*g_hcho_cnt-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,g_history_hcho[*g_hcho_cnt-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printf("hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=(atoi(g_history_hcho[*g_hcho_cnt-*offset-i-1].data+2)*667)/1000;
								printf("j %d %s==>%d\n",j,g_history_hcho[*g_hcho_cnt-*offset-i-1].time,buf[j-1]);							
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,g_history_hcho[*g_hcho_cnt-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printf("j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			*offset+=i;
			if(*offset>=*g_hcho_cnt)
				*offset=0;

		}
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
		//printf("g_shidu_cnt %d\n",*g_shidu_cnt);
		if((*g_shidu_cnt-*offset-7)>0)
		{
			char info_l[]={"湿度"};
			strcpy(index[0],"90");
			strcpy(index[1],"68");
			strcpy(index[2],"45");
			strcpy(index[3],"23");
			strcpy(index[4],"00");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			//strcpy(info,"Humidity");
			sprintf(info,"%s",info_l);
			write_string(fd, ADDR_CURVE_DATE, g_history_shidu[*g_shidu_cnt-*offset-1].time,10);
			memcpy(temp,g_history_shidu[*g_shidu_cnt-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,g_history_shidu[*g_shidu_cnt-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,g_history_shidu[*g_shidu_cnt-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printf("hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(g_history_shidu[*g_shidu_cnt-*offset-i-1].data)*22;
								printf("j %d %s==>%d\n",j,g_history_shidu[*g_shidu_cnt-*offset-i-1].time,buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,g_history_shidu[*g_shidu_cnt-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printf("j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			*offset+=i;
			if(*offset>=*g_shidu_cnt)
				*offset=0;

		}
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
		//printf("g_temp_cnt %d\n",*g_temp_cnt);
		if((*g_temp_cnt-*offset-7)>0)
		{
			char info_l[]={"温度"};
			strcpy(index[0],"60");
			strcpy(index[1],"40");
			strcpy(index[2],"20");
			strcpy(index[3],"00");
			strcpy(index[4],"-20");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			//strcpy(info,"Temperature");
			sprintf(info,"%s",info_l);
			write_string(fd, ADDR_CURVE_DATE, g_history_temp[*g_temp_cnt-*offset-1].time,10);
			memcpy(temp,g_history_temp[*g_temp_cnt-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,g_history_temp[*g_temp_cnt-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,g_history_temp[*g_temp_cnt-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printf("hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=(atoi(g_history_temp[*g_temp_cnt-*offset-i-1].data)+20)*25;
								printf("j %d %s==>%d\n",j,g_history_temp[*g_temp_cnt-*offset-i-1].time,buf[j-1]);
							}
							
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,g_history_temp[*g_temp_cnt-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printf("j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			*offset+=i;
			if(*offset>=*g_temp_cnt)
				*offset=0;
		}
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
		//printf("g_pm25_cnt %d\n",*g_pm25_cnt);
		if((*g_pm25_cnt-*offset)>0)
		{
			strcpy(index[0],"1000");
			strcpy(index[1],"750");
			strcpy(index[2],"500");
			strcpy(index[3],"250");
			strcpy(index[4],"000");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"PM25");
			write_string(fd, ADDR_CURVE_DATE, g_history_pm25[*g_pm25_cnt-*offset-1].time,10);
			memcpy(temp,g_history_pm25[*g_pm25_cnt-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,g_history_pm25[*g_pm25_cnt-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,g_history_pm25[*g_pm25_cnt-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printf("hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(g_history_pm25[*g_pm25_cnt-*offset-i-1].data)*2;
								printf("j %d %s==>%d %d\n",j,g_history_pm25[*g_pm25_cnt-*offset-i-1].time,atoi(g_history_pm25[*g_pm25_cnt-*offset-i-1].data),buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,g_history_pm25[*g_pm25_cnt-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printf("j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			*offset+=i;
			if(*offset>=*g_pm25_cnt)
				*offset=0;
		}
		else
			*offset=0;
	}
	//write index
	clear_buf(fd,ADDR_CURVE_TYPE,10);
	clear_buf(fd, ADDR_CURVE_DATA_5,16);
	clear_buf(fd, ADDR_CURVE_DATA_4,16);
	clear_buf(fd, ADDR_CURVE_DATA_3,16);
	clear_buf(fd, ADDR_CURVE_DATA_2,16);
	clear_buf(fd, ADDR_CURVE_DATA_1,16);
	write_string(fd, ADDR_CURVE_TYPE, info,strlen(info));
	write_string(fd, ADDR_CURVE_TIME_5, index_time[0],2);
	write_string(fd, ADDR_CURVE_TIME_4, index_time[1],2);
	write_string(fd, ADDR_CURVE_TIME_3, index_time[2],2);
	write_string(fd, ADDR_CURVE_TIME_2, index_time[3],2);
	write_string(fd, ADDR_CURVE_TIME_1, index_time[4],2);
	write_string(fd, ADDR_CURVE_DATA_5, index[0], strlen(index[0]));
	write_string(fd, ADDR_CURVE_DATA_4, index[1], strlen(index[1]));
	write_string(fd, ADDR_CURVE_DATA_3, index[2], strlen(index[2]));
	write_string(fd, ADDR_CURVE_DATA_2, index[3], strlen(index[3]));
	write_string(fd, ADDR_CURVE_DATA_1, index[4], strlen(index[4]));
	draw_curve(fd,buf,j);
}
int read_dgus(int fd,int addr,char len,char *out)
{
	char ch,i;
	int timeout=0;
	char cmd[]={0x5a,0xa5,0x04,0x83,0x00,0x00,0x00};
	cmd[4]=(addr & 0xff00)>>8;
	cmd[5]=addr & 0x00ff;
	cmd[6]=len;
	write(fd,cmd,7);
	i=0;
	while(1)	
	{	
		if(read(fd,&ch,1)==1)
		{
			if(i>=7)
				out[i-7]=ch;
			i++;
			printf("==> %x\n",ch);
			if(i==(2*len+7))
				return 1;
		}
		else
		{
			timeout++;
			if(timeout>100)
				return 0;
			usleep(1000);
		}
	}
	return 0;
}
int verify_pwd(char *username,char *passwd)
{
	int result=0;
	if((strlen(username)==strlen("root") && strlen(passwd)==strlen("84801058")) && strncmp(username,"root",strlen("root"))==0 && strncmp(passwd,"84801058",strlen("84801058"))==0)
	{
		logged=1;
		return 1;
	}
	char *verify_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_VERIFY_USER);
	verify_msg=add_item(verify_msg,ID_DEVICE_UID,g_uuid);
	verify_msg=add_item(verify_msg,ID_DEVICE_IP_ADDR,ip);
	verify_msg=add_item(verify_msg,ID_DEVICE_PORT,"9517");
	verify_msg=add_item(verify_msg,ID_USER_NAME_TYPE,username);
	verify_msg=add_item(verify_msg,ID_USER_PWD_TYPE,passwd);
	char *rcv=NULL;
	send_web_post(URL,verify_msg,9,&rcv);
	free(verify_msg);
	verify_msg=NULL;
	if(rcv!=NULL)
	{	
		if(strstr(rcv,"ok"))
		{
			result=1;
			logged=1;
		}
		free(rcv);
		rcv=NULL;
	}	
	return result;
}
void clear_buf(int fd,int addr,int len)
{
	char *tmp=(char *)malloc(len);
	memset(tmp,0,len);
	write_string(fd,addr,tmp,len);
	free(tmp);
}
void display_time(int fd,int year,int mon,int day,int hour,int min,int seconds)
{
	char buf[5]={0};
	sprintf(buf,"%d",year);
	write_string(fd,ADDR_TIME_YEAR,buf,4);
	memset(buf,0,5);
	sprintf(buf,"%d",day);
	write_string(fd,ADDR_TIME_DAY,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",mon);
	write_string(fd,ADDR_TIME_MONTH,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",hour);
	write_string(fd,ADDR_TIME_HOUR,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",min);
	write_string(fd,ADDR_TIME_MIN,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",seconds);
	write_string(fd,ADDR_TIME_SECOND,buf,2);
}
void manul_set_time(int fd)
{
	char year[5]={0};
	char mon[3]={0};
	char day[3]={0};
	char hour[3]={0};
	char min[3]={0};
	char second[3]={0};
	if(read_dgus(fd,ADDR_TIME_YEAR,2,year) && read_dgus(fd,ADDR_TIME_DAY,1,day)
		&& read_dgus(fd,ADDR_TIME_MONTH,1,mon) && read_dgus(fd,ADDR_TIME_HOUR,1,hour)
		&& read_dgus(fd,ADDR_TIME_MIN,1,min) && read_dgus(fd,ADDR_TIME_SECOND,1,second))
	{
		if(atoi(year)>0 && atoi(mon)>0 && atoi(mon)<=12 && atoi(day)>0 && atoi(day)<=31
			&& atoi(hour)>=0 && atoi(hour)<=23 && atoi(min)>=0 && atoi(min)<=59
			&&atoi(second)>=0 && atoi(second)<=59)
		{
			printf("year %s \nmon %s\nday %s\nhour %s\nmin %s\nseconds %s\n",year,mon,day,hour,min,second);
			server_time[0]=0x6c;server_time[1]=ARM_TO_CAP;
			server_time[2]=0x00;server_time[3]=0x01;server_time[4]=0x06;
			server_time[5]=atoi(year)-2000;server_time[6]=atoi(mon);
			server_time[7]=atoi(day);server_time[8]=atoi(hour);
			server_time[9]=atoi(min);server_time[10]=atoi(second);
			int crc=CRC_check(server_time,11);
			server_time[11]=(crc&0xff00)>>8;server_time[12]=crc&0x00ff;
			write(fd_com,server_time,13);
		}
		//set_time(server_time[5]+2000,server_time[6],server_time[7],server_time[8],server_time[9],server_time[10]);
	}
}
void log_in(int fd)
{
	char user_name[256]={0};
	char passwd[256]={0};
	if(read_dgus(fd,ADDR_USER_NAME_VERIFY,5,user_name) && read_dgus(fd,ADDR_USER_PWD_VERIFY,5,passwd))
	{
		printf("User Name %s \nUser Pwd %s\n",user_name,passwd);
		if(verify_pwd(user_name,passwd))
		{
			if(*history_done)
				switch_pic(fd,CURVE_PAGE);
			else
				switch_pic(fd,MAIN_PAGE);	
			return;
		}
	}
	switch_pic(fd,MAIN_PAGE);
}
void wifi_handle(int fd)
{
	char ap_name[256]={0};
	char ap_passwd[256]={0};
	char ret[256]={0};
	char cmd[256]={0};
	int i;
	FILE *fp;
	//clear_buf(fd,WIFI_AP_NAME_ADDR,20);
	//clear_buf(fd,WIFI_AP_PWD_ADDR,20);
	if(read_dgus(fd,ADDR_AP_NAME,10,ap_name) && read_dgus(fd,ADDR_AP_PASSWD,10,ap_passwd))
	{
		printf("AP Name %s \nAP Pwd %s\n",ap_name,ap_passwd);
		if(strlen(ap_passwd)<8||strlen(ap_name)==0)
			return ;
		for(i=0;i<100;i++)
		{
			sprintf(cmd,"wpa_cli -ira0 get_network %d ssid",i);
			if((fp=popen(cmd,"r"))!=NULL)
			{
				memset(ret,0,256);
				fread(ret,sizeof(char),sizeof(ret),fp);
				pclose(fp);
				printf("wpa_cli -ira0 get_network return %s %d\n",ret,strlen(ret));
				if(strstr(ret,"FAIL")!=NULL)
					break;
				if(strstr(ret,ap_name)!=NULL)
					break;					
			}
		}
		if(strstr(ret,"FAIL")!=NULL)
		{
			memset(cmd,0,256);
			sprintf(cmd,"wpa_cli -ira0 add_network");
			if((fp=popen(cmd,"r"))!=NULL)
			{
				memset(ret,0,256);
				fread(ret,sizeof(char),sizeof(ret),fp);
				pclose(fp);
				ret[strlen(ret)-1]='\0';
				printf("add_network return %s\n",ret);
				i=atoi(ret);
			}
		}
		printf("i is %d\n",i);
		sprintf(cmd,"wpa_cli -ira0 set_network %d key_mgmt WPA-PSK",i);
		printf("exec %s\n",cmd);
		system(cmd);	
		sprintf(cmd,"wpa_cli -ira0 set_network %d psk \\\"%s\\\"",i,ap_passwd);
		printf("exec %s\n",cmd);
		system(cmd);		
		sprintf(cmd,"wpa_cli -ira0 set_network %d ssid \\\"%s\\\"",i,ap_name);
		printf("exec %s\n",cmd);
		system(cmd);
		sprintf(cmd,"wpa_cli -ira0 enable_network %d",i);
		printf("exec %s\n",cmd);
		system(cmd);
		//sprintf(cmd,"wpa_cli -ira0 select_network %d",i);
		//printf("exec %s\n",cmd);
		//system(cmd);
		sprintf(cmd,"wpa_cli -ira0 save_config");
		printf("exec %s\n",cmd);		
		system(cmd);
		sprintf(cmd,"udhcpc -i ra0");
		printf("exec %s\n",cmd);
		//system(cmd);
	}
}
void tun_zero(int fd,int on)
{
	char cmd_request_verify[]=	{0x6c,ARM_TO_CAP,0x00,0x04,0x01,0x00,0x00,0x00};
	char cmd_return_point[]=	{0x6c,ARM_TO_CAP,0x00,0x05,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
	int crc = 0;
	int i =0;
	//send cap board start co & hcho
	if(on)
	{		
		printf("Start Co,ch2o tun zero\n");
		for(i=0;i<10;i++)
			if(sensor_interface[i] == TYPE_SENSOR_CO_WEISHEN ||
				sensor_interface[i] == TYPE_SENSOR_CO_DD)
				break;
		printf("CO interface %d %4x\n",i,sensor_interface[i]);
		cmd_request_verify[5]=i;
		crc=CRC_check(cmd_request_verify,6);
		cmd_request_verify[6]=(crc&0xff00)>>8;cmd_request_verify[7]=crc&0x00ff;		
		write(fd_com,cmd_request_verify,sizeof(cmd_request_verify));
		sleep(1);
		cmd_return_point[5]=i;
		crc=CRC_check(cmd_return_point,10);
		cmd_return_point[10]=(crc&0xff00)>>8;cmd_return_point[11]=crc&0x00ff;		
		write(fd_com,cmd_return_point,sizeof(cmd_return_point));
		for(i=0;i<10;i++)
			if(sensor_interface[i] == TYPE_SENSOR_CH2O_WEISHEN ||
				sensor_interface[i] == TYPE_SENSOR_CH2O_AERSHEN)
				break;
		printf("CH2O interface %d %4x\n",i,sensor_interface[i]);
		cmd_request_verify[5]=i;
		crc=CRC_check(cmd_request_verify,6);
		cmd_request_verify[6]=(crc&0xff00)>>8;cmd_request_verify[7]=crc&0x00ff;		
		write(fd_com,cmd_request_verify,sizeof(cmd_request_verify));
		sleep(1);
		cmd_return_point[5]=i;
		crc=CRC_check(cmd_return_point,10);
		cmd_return_point[10]=(crc&0xff00)>>8;cmd_return_point[11]=crc&0x00ff;		
		write(fd_com,cmd_return_point,sizeof(cmd_return_point));
	}
	else
	{
		printf("Stop Co,ch2o tun zero\n");
		for(i=0;i<10;i++)
			if(sensor_interface[i] == TYPE_SENSOR_CH2O_WEISHEN ||
				sensor_interface[i] == TYPE_SENSOR_CH2O_AERSHEN)
				break;
		printf("CH2O interface %d %4x\n",i,sensor_interface[i]);
		cmd_return_point[5]=i;
		cmd_return_point[7]=(g_zero_info->cur_ch2o>>8) & 0xff;
		cmd_return_point[8]=(g_zero_info->cur_ch2o & 0xff);
		//cmd_return_point[9]=g_zero_info->cur_ch2o_point;
		crc=CRC_check(cmd_return_point,9);
		cmd_return_point[9]=(crc&0xff00)>>8;cmd_return_point[10]=crc&0x00ff;
		write(fd_com,cmd_return_point,sizeof(cmd_return_point));
		for(i=0;i<10;i++)
			if(sensor_interface[i] == TYPE_SENSOR_CO_WEISHEN ||
				sensor_interface[i] == TYPE_SENSOR_CO_DD)
				break;
		printf("CH2O interface %d %4x\n",i,sensor_interface[i]);
		cmd_return_point[5]=i;
		cmd_return_point[7]=(g_zero_info->cur_co>>8) & 0xff;
		cmd_return_point[8]=(g_zero_info->cur_co & 0xff);
		//cmd_return_point[9]=g_zero_info->cur_co_point;
		crc=CRC_check(cmd_return_point,9);
		cmd_return_point[9]=(crc&0xff00)>>8;cmd_return_point[10]=crc&0x00ff;
		write(fd_com,cmd_return_point,sizeof(cmd_return_point));
	}
}
/*
  *get cmd from lv's cap board
*/
int get_interface()
{
	char ch,state=STATE_IDLE,message_len=0;
	char message[10],i=0,to_check[20];
	int crc,message_type=0;
	while(1)
	{
		if(read(fd_com,&ch,1)==1)
		{
			printf("get_interface %02x\n",ch);
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
					if(crc==CRC_check(cmd,message_len+5))
					{
						for(i=0;i<message_len;i=i+2)
						{
							sensor_interface[i/2]=(message[i]<<8)|message[i+1];
							printf("sensor_interface[%d] = %4x\n",i/2,sensor_interface[i/2]);
						}
					}
					else
					{
						printf(CAP_PROCESS"Get Interface CRC error 0x%04X\r\n",CRC_check(cmd,message_len+5));
						for(i=0;i<message_len+5;i++)
							printf("0x%02x ",cmd[i]);
					}

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

void ask_interface()
{
	char cmd[] = {0x6c,ARM_TO_CAP,0x00,0x06,0x00,0x00,0x00};	
	int crc=CRC_check(cmd,5);
	cmd[5]=(crc&0xff00)>>8;cmd[6]=crc&0x00ff; 	
	write(fd_com,cmd,sizeof(cmd));
	get_interface();
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
	static int curve_co=0;
	static int curve_co2=0;
	static int curve_hcho=0;
	static int curve_temp=0;
	static int curve_shidu=0;
	static int curve_pm25=0;
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
	if(lcd_state==0)
	{
		lcd_on(MAIN_PAGE);
		alarm(300);
	}
	else
	{
		alarm(0);
		alarm(300);
	}
	if(addr==TOUCH_RUN_TIME_CO && (TOUCH_RUN_TIME_CO+0x100)==data)
	{//show history CO the first page
		if(logged)
		{
			if(*history_done)
			{
				switch_pic(fd_lcd,CURVE_PAGE);
				show_curve(fd_lcd,ID_CAP_CO,&curve_co);
			}
			else
			{
				switch_pic(fd_lcd,LIST_CO_PAGE);
				show_history(fd_lcd,ID_CAP_CO,begin_co);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(fd_lcd,ADDR_USER_NAME_VERIFY,16);
			clear_buf(fd_lcd,ADDR_USER_PWD_VERIFY,16);
			switch_pic(fd_lcd,LOG_IN_PAGE);
		}
		g_index=LIST_CO_PAGE;
	}
	else if(addr==TOUCH_RUN_TIME_CO2 && (TOUCH_RUN_TIME_CO2+0x100)==data)
	{//show history CO2 the first page
		if(logged)
		{
			if(*history_done)
			{
				switch_pic(fd_lcd,CURVE_PAGE);
				show_curve(fd_lcd,ID_CAP_CO2,&curve_co2);
			}
			else
			{
				switch_pic(fd_lcd,LIST_CO2_PAGE);
				show_history(fd_lcd,ID_CAP_CO2,begin_co2);
				//begin_co2=0;
			}
		}
		else
		{
			clear_buf(fd_lcd,ADDR_USER_NAME_VERIFY,16);
			clear_buf(fd_lcd,ADDR_USER_PWD_VERIFY,16);
			switch_pic(fd_lcd,LOG_IN_PAGE);	
		}
		g_index=LIST_CO2_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_HCHO && (TOUCH_RUN_TIME_HCHO+0x100)==data)
	{//show history HCHO the first page	
		if(logged)
		{
			if(*history_done)
			{
				switch_pic(fd_lcd,CURVE_PAGE);
				show_curve(fd_lcd,ID_CAP_HCHO,&curve_hcho);
			}
			else
			{
				switch_pic(fd_lcd,LIST_HCHO_PAGE);
				show_history(fd_lcd,ID_CAP_HCHO,begin_hcho);
				//begin_co=0;
			}
		}
		else
		{		
			clear_buf(fd_lcd,ADDR_USER_NAME_VERIFY,16);
			clear_buf(fd_lcd,ADDR_USER_PWD_VERIFY,16);
			switch_pic(fd_lcd,LOG_IN_PAGE);			
		}
		g_index=LIST_HCHO_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_SHIDU && (TOUCH_RUN_TIME_SHIDU+0x100)==data)
	{//show history SHIDU the first page
		if(logged)
		{
			if(*history_done)
			{
				switch_pic(fd_lcd,CURVE_PAGE);
				show_curve(fd_lcd,ID_CAP_SHI_DU,&curve_shidu);
			}
			else
			{
				switch_pic(fd_lcd,LIST_SHIDU_PAGE);
				show_history(fd_lcd,ID_CAP_SHI_DU,begin_shidu);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(fd_lcd,ADDR_USER_NAME_VERIFY,16);
			clear_buf(fd_lcd,ADDR_USER_PWD_VERIFY,16);
			switch_pic(fd_lcd,LOG_IN_PAGE);
		}		
		g_index=LIST_SHIDU_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_TEMP && (TOUCH_RUN_TIME_TEMP+0x100)==data)
	{//show history TEMPERATURE the first page
		if(logged)
		{
			if(*history_done)
			{
				switch_pic(fd_lcd,CURVE_PAGE);
				show_curve(fd_lcd,ID_CAP_TEMPERATURE,&curve_temp);
			}
			else
			{
				switch_pic(fd_lcd,LIST_TEMP_PAGE);
				show_history(fd_lcd,ID_CAP_TEMPERATURE,begin_temp);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(fd_lcd,ADDR_USER_NAME_VERIFY,16);
			clear_buf(fd_lcd,ADDR_USER_PWD_VERIFY,16);
			switch_pic(fd_lcd,LOG_IN_PAGE);
		}		
		g_index=LIST_TEMP_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_PM25&& (TOUCH_RUN_TIME_PM25+0x100)==data)
	{//show history PM25 the first page
		if(logged)
		{
			if(*history_done)
			{
				switch_pic(fd_lcd,CURVE_PAGE);
				show_curve(fd_lcd,ID_CAP_PM_25,&curve_pm25);
			}
			else
			{
				switch_pic(fd_lcd,LIST_PM25_PAGE);
				show_history(fd_lcd,ID_CAP_PM_25,begin_pm25);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(fd_lcd,ADDR_USER_NAME_VERIFY,16);
			clear_buf(fd_lcd,ADDR_USER_PWD_VERIFY,16);
			switch_pic(fd_lcd,LOG_IN_PAGE);
		}
		g_index=LIST_PM25_PAGE;
	}	
	else if(addr==TOUCH_CO_REFRESH_PAGE && (TOUCH_CO_REFRESH_PAGE+0x100)==data)
	{//show history CO the next page
		begin_co=0;
		show_history(fd_lcd,ID_CAP_CO,begin_co);
	}
	else if(addr==TOUCH_CO_LAST_PAGE && (TOUCH_CO_LAST_PAGE+0x100)==data)
	{//show history CO the next page
		if(begin_co>=7)
		{
			begin_co-=7;
			show_history(fd_lcd,ID_CAP_CO,begin_co);
		}
	}
	else if(addr==TOUCH_CO_NEXT_PAGE && (TOUCH_CO_NEXT_PAGE+0x100)==data)
	{//show history CO the next page
		if(begin_co+7<*g_co_cnt)
		{
			begin_co+=7;
			show_history(fd_lcd,ID_CAP_CO,begin_co);
		}
	}
	else if(addr==TOUCH_CO2_REFRESH_PAGE && (TOUCH_CO2_REFRESH_PAGE+0x100)==data)
	{//show history CO2 the next page
		begin_co2=0;
		show_history(fd_lcd,ID_CAP_CO2,begin_co2);
	}
	else if(addr==TOUCH_CO2_LAST_PAGE && (TOUCH_CO2_LAST_PAGE+0x100)==data)
	{//show history CO2 the next page
		if(begin_co2>=7)
		{
			begin_co2-=7;
			show_history(fd_lcd,ID_CAP_CO2,begin_co2);
		}
	}
	else if(addr==TOUCH_CO2_NEXT_PAGE && (TOUCH_CO2_NEXT_PAGE+0x100)==data)
	{//show history CO2 the next page
		if(begin_co2+7<*g_co2_cnt)
		{
			begin_co2+=7;
			show_history(fd_lcd,ID_CAP_CO2,begin_co2);
		}
	}
	else if(addr==TOUCH_HCHO_REFRESH_PAGE && (TOUCH_HCHO_REFRESH_PAGE+0x100)==data)
	{//show history HCHO the next page
		begin_hcho=0;
		show_history(fd_lcd,ID_CAP_HCHO,begin_hcho);
	}
	else if(addr==TOUCH_HCHO_LAST_PAGE && (TOUCH_HCHO_LAST_PAGE+0x100)==data)
	{//show history HCHO the next page
		if(begin_hcho>=7)
		{
			begin_hcho-=7;
			show_history(fd_lcd,ID_CAP_HCHO,begin_hcho);
		}
	}
	else if(addr==TOUCH_HCHO_NEXT_PAGE && (TOUCH_HCHO_NEXT_PAGE+0x100)==data)
	{//show history HCHO the next page
		if(begin_hcho+7<*g_hcho_cnt)
		{
			begin_hcho+=7;
			show_history(fd_lcd,ID_CAP_HCHO,begin_hcho);
		}
	}
	else if(addr==TOUCH_TEMP_REFRESH_PAGE && (TOUCH_TEMP_REFRESH_PAGE+0x100)==data)
	{//show history TEMPERATURE the next page
		begin_temp=0;
		show_history(fd_lcd,ID_CAP_TEMPERATURE,begin_temp);
	}
	else if(addr==TOUCH_TEMP_LAST_PAGE && (TOUCH_TEMP_LAST_PAGE+0x100)==data)
	{//show history TEMP the next page
		if(begin_temp>=7)
		{
			begin_temp-=7;
			show_history(fd_lcd,ID_CAP_TEMPERATURE,begin_temp);
		}
	}
	else if(addr==TOUCH_TEMP_NEXT_PAGE && (TOUCH_TEMP_NEXT_PAGE+0x100)==data)
	{//show history TEMP the next page
		if(begin_temp+7<*g_temp_cnt)
		{
			begin_temp+=7;
			show_history(fd_lcd,ID_CAP_TEMPERATURE,begin_temp);
		}
	}
	else if(addr==TOUCH_SHIDU_REFRESH_PAGE&& (TOUCH_SHIDU_REFRESH_PAGE+0x100)==data)
	{//show history SHIDU the next page
		begin_shidu=0;
		show_history(fd_lcd,ID_CAP_SHI_DU,begin_shidu);
	}
	else if(addr==TOUCH_SHIDU_LAST_PAGE && (TOUCH_SHIDU_LAST_PAGE+0x100)==data)
	{//show history SHIDU the next page
		if(begin_shidu>=7)
		{
			begin_shidu-=7;
			show_history(fd_lcd,ID_CAP_SHI_DU,begin_shidu);
		}
	}
	else if(addr==TOUCH_SHIDU_NEXT_PAGE && (TOUCH_SHIDU_NEXT_PAGE+0x100)==data)
	{//show history SHIDU the next page
		if(begin_shidu+7<*g_shidu_cnt)
		{
			begin_shidu+=7;
			show_history(fd_lcd,ID_CAP_SHI_DU,begin_shidu);
		}
	}
	else if(addr==TOUCH_PM25_REFRESH_PAGE && (TOUCH_PM25_REFRESH_PAGE+0x100)==data)
	{//show history PM25 the next page
		begin_pm25=0;
		show_history(fd_lcd,ID_CAP_PM_25,begin_pm25);
	}
	else if(addr==TOUCH_PM25_LAST_PAGE && (TOUCH_PM25_LAST_PAGE+0x100)==data)
	{//show history PM25 the next page
		if(begin_pm25>=7)
		{
			begin_pm25-=7;
			show_history(fd_lcd,ID_CAP_PM_25,begin_pm25);
		}
	}
	else if(addr==TOUCH_PM25_NEXT_PAGE && (TOUCH_PM25_NEXT_PAGE+0x100)==data)
	{//show history PM25 the next page
		if(begin_pm25+7<*g_pm25_cnt)
		{
			begin_pm25+=7;
			show_history(fd_lcd,ID_CAP_PM_25,begin_pm25);
		}
	}	
	else if(addr==TOUCH_DEVICE_STATE&& (TOUCH_DEVICE_STATE+0x100)==data)
	{//show sensor and network state
		show_sensor_network(fd_lcd);
	}
	else if(addr==TOUCH_TIME_SET&& (TOUCH_TIME_SET+0x100)==data)
	{//set time
		clear_buf(fd_lcd,ADDR_TIME_YEAR,4);
		clear_buf(fd_lcd,ADDR_TIME_MONTH,2);
		clear_buf(fd_lcd,ADDR_TIME_DAY,2);
		clear_buf(fd_lcd,ADDR_TIME_HOUR,2);
		clear_buf(fd_lcd,ADDR_TIME_MIN,2);
		clear_buf(fd_lcd,ADDR_TIME_SECOND,2);
		//switch_pic(fd_lcd, TIME_SETTING_PAGE);
	}
	else if(addr==TOUCH_SELECT_WIFI&& (TOUCH_SELECT_WIFI+0x100)==data)
	{//WiFi Passwd changed
		wifi_handle(fd_lcd);
		send_by_wifi=1;
		write_string(fd_lcd,ADDR_XFER_MODE,"WIFI",strlen("WIFI"));
	}
	else if(addr==TOUCH_SELECT_GPRS&& (TOUCH_SELECT_GPRS+0x100)==data)
	{//use gprs to xfer
		send_by_wifi=0;
		write_string(fd_lcd,ADDR_XFER_MODE,"GPRS",strlen("GPRS"));
	}
	else if(addr==TOUCH_XFER_SET&&(TOUCH_XFER_SET+0x100)==data)
	{//enter wifi passwd setting
		clear_buf(fd_lcd,ADDR_AP_NAME,20);
		clear_buf(fd_lcd,ADDR_AP_PASSWD,20);
		if(send_by_wifi)
		{
			write_string(fd_lcd,ADDR_XFER_MODE,"WIFI",strlen("WIFI"));
		}
		else
		{
			write_string(fd_lcd,ADDR_XFER_MODE,"GPRS",strlen("GPRS"));
		}
	}
	else if(addr==TOUCH_TIME_CHANGE_MANUL && (TOUCH_TIME_CHANGE_MANUL+0x100)==data)
	{//manul set time
		manul_set_time(fd_lcd);
	}
	else if(addr==TOUCH_TIME_CHANGE_SERVER && (TOUCH_TIME_CHANGE_SERVER+0x100)==data)
	{//set time from server
		sync_server(fd_com,0,0);
		display_time(fd_lcd,server_time[5]+2000,server_time[6],server_time[7],server_time[8],server_time[9],server_time[10]);
		//sleep(3);
		//switch_pic(fd_lcd,18);
	}
	else if(addr==TOUCH_VERIFY && (TOUCH_VERIFY+0x100)==data)
	{//verify sensor display
	
	}
	else if(addr==TOUCH_TUN_ZERO_BEGIN && (TOUCH_TUN_ZERO_BEGIN+0x100)==data)
	{//tun zero point start
		if(factory_mode!=TUN_ZERO_MODE)
		{
			factory_mode=TUN_ZERO_MODE;
			tun_zero(fd_lcd,1);
		}
	}
	else if((addr==TOUCH_TUN_ZERO_RETURN && (TOUCH_TUN_ZERO_RETURN+0x100)==data) || 
		(addr==TOUCH_TUN_ZERO_END && (TOUCH_TUN_ZERO_END+0x100)==data))
	{//tun zero point stop
		if(factory_mode == TUN_ZERO_MODE)
		{
			tun_zero(fd_lcd,0);
			factory_mode=NORMAL_MODE;
		}
	}
	else if(addr==TOUCH_INTERFACE_SET && (TOUCH_INTERFACE_SET+0x100)==data)
	{//set sensor interface
	
	}
	else if(addr==TOUCH_PRODUCT_INFO && (TOUCH_PRODUCT_INFO+0x100)==data)
	{//product info
		clear_buf(fd_lcd,ADDR_PRODUCT_NAME,40);
		clear_buf(fd_lcd,ADDR_PRODUCT_MODE,40);
		clear_buf(fd_lcd,ADDR_PRODUCT_ID,40);
		write_string(fd_lcd,ADDR_PRODUCT_ID,g_uuid,strlen(g_uuid));
	}		
	else if(addr==TOUCH_CURVE_LIST&& (TOUCH_CURVE_LIST+0x100)==data)
	{//show detail in list
		switch (g_index)
		{
			case LIST_CO_PAGE:
			{//co
				switch_pic(fd_lcd,LIST_CO_PAGE);
				show_history(fd_lcd,ID_CAP_CO,begin_co);
				//begin_co=0;
			}
			break;
			case LIST_CO2_PAGE:
			{//co2
				switch_pic(fd_lcd,LIST_CO2_PAGE);
				show_history(fd_lcd,ID_CAP_CO2,begin_co2);
				//begin_co2=0;
			}
			break;
			case LIST_HCHO_PAGE:
			{//hcho
				switch_pic(fd_lcd,LIST_HCHO_PAGE);
				show_history(fd_lcd,ID_CAP_HCHO,begin_hcho);
				//begin_hcho=0;
			}
			break;
			case LIST_SHIDU_PAGE:
			{//shidu
				switch_pic(fd_lcd,LIST_SHIDU_PAGE);
				show_history(fd_lcd,ID_CAP_SHI_DU,begin_shidu);
				//begin_shidu=0;
			}
			break;
			case LIST_TEMP_PAGE:
			{//temp
				switch_pic(fd_lcd,LIST_TEMP_PAGE);
				show_history(fd_lcd,ID_CAP_TEMPERATURE,begin_temp);
				//begin_temp=0;
			}
			break;
			case LIST_PM25_PAGE:
			{//pm25
				switch_pic(fd_lcd,LIST_PM25_PAGE);
				show_history(fd_lcd,ID_CAP_PM_25,begin_pm25);
				//begin_pm25=0;
			}
			break;
			default:
				break;
		}
	}
	else if(addr==TOUCH_USER_OK_VERIFY&& (TOUCH_USER_OK_VERIFY+0x100)==data)
	{//Login if didn't 
		log_in(fd_lcd);
		printf("lcd=>history_done %d\n",*history_done);
		if(logged)
		{
			if(*history_done==0)
			switch (g_index)
			{
			case LIST_CO_PAGE:
			{//co
				switch_pic(fd_lcd,LIST_CO_PAGE);
				show_history(fd_lcd,ID_CAP_CO,begin_co);
				//begin_co=0;
			}
			break;
			case LIST_CO2_PAGE:
			{//co2
				switch_pic(fd_lcd,LIST_CO2_PAGE);
				show_history(fd_lcd,ID_CAP_CO2,begin_co2);
				//begin_co2=0;
			}
			break;
			case LIST_HCHO_PAGE:
			{//hcho
				switch_pic(fd_lcd,LIST_HCHO_PAGE);
				show_history(fd_lcd,ID_CAP_HCHO,begin_hcho);
				//begin_hcho=0;
			}
			break;
			case LIST_SHIDU_PAGE:
			{//shidu
				switch_pic(fd_lcd,LIST_SHIDU_PAGE);
				show_history(fd_lcd,ID_CAP_SHI_DU,begin_shidu);
				//begin_shidu=0;
			}
			break;
			case LIST_TEMP_PAGE:
			{//temp
				switch_pic(fd_lcd,LIST_TEMP_PAGE);
				show_history(fd_lcd,ID_CAP_TEMPERATURE,begin_temp);
				//begin_temp=0;
			}
			break;
			case LIST_PM25_PAGE:
			{//pm25
				switch_pic(fd_lcd,LIST_PM25_PAGE);
				show_history(fd_lcd,ID_CAP_PM_25,begin_pm25);
				//begin_pm25=0;
			}
			break;
			default:
				break;
			}
			else
			switch (g_index)
			{
				case LIST_CO_PAGE:
				{//co
					show_curve(fd_lcd,ID_CAP_CO,&curve_co);
				}
				break;
				case LIST_CO2_PAGE:
				{//co2
					show_curve(fd_lcd,ID_CAP_CO2,&curve_co2);
				}
				break;
				case LIST_HCHO_PAGE:
				{//hcho
					show_curve(fd_lcd,ID_CAP_HCHO,&curve_hcho);
				}
				break;
				case LIST_SHIDU_PAGE:
				{//shidu
					show_curve(fd_lcd,ID_CAP_SHI_DU,&curve_shidu);
				}
				break;
				case LIST_TEMP_PAGE:
				{//temp
					show_curve(fd_lcd,ID_CAP_TEMPERATURE,&curve_temp);
				}
				break;
				case LIST_PM25_PAGE:
				{//pm25
					show_curve(fd_lcd,ID_CAP_PM_25,&curve_pm25);
				}
				break;
				default:
					break;
			}
		}
	}
	return STATE_MAIN;
}
void lcd_loop(int fd)
{	
	char ch;
	int i=1;
	int get=0;
	char ptr[32]={0};	
	switch_pic(fd,MAIN_PAGE);
	while(1)	
	{	
		if(read(fd,&ch,1)==1)
		{
			//printf("<=%x \r\n",ch);
			switch(get)
			{
				case 0:
					if(ch==0x5a)
					{
						//printf(LCD_PROCESS"0x5a get ,get =1\r\n");
						get=1;
					}
					break;
				case 1:
					if(ch==0xa5)
					{
						//printf(LCD_PROCESS"0xa5 get ,get =2\r\n");
						get=2;

					}
					break;
				case 2:
					if(ch==0x06)
					{
						//printf(LCD_PROCESS"0x06 get,get =3\r\n");
						get=3;
						break;
					}
				case 3:
					if(ch==0x83)
					{
						//printf(LCD_PROCESS"0x83 get,get =4\r\n");
						get=4;
						i=1;
						break;
					}
				case 4:
					{
						//printf(LCD_PROCESS"%02x get ,get =5\r\n",ch);
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
	printf("set upload flag\n");
	g_upload=1;
	alarm(600);
}
void lcd_off(int a)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x01,0x00};
	write(fd_lcd,cmd,6);
	switch_pic(fd_lcd,OFF_PAGE);
	lcd_state=0;
	printf("lcd off\n");
}
void lcd_on(int page)
{
	switch_pic(fd_lcd,page);
	char cmd[]={0x5a,0xa5,0x03,0x80,0x01,0x40};
	write(fd_lcd,cmd,6);
	lcd_state=1;
	printf("lcd on\n");
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
	g_state->sensor[0]=sensor.co;
	g_state->sensor[1]=sensor.co2;
	g_state->sensor[2]=sensor.hcho;
	g_state->sensor[3]=sensor.shidu;
	g_state->sensor[4]=sensor.temp;
	g_state->sensor[5]=sensor.pm25;

	fclose(fp);
	printf(MAIN_PROCESS"GOT Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",sensor.co,sensor.co2,sensor.hcho,sensor.shidu,sensor.temp,sensor.pm25);
}
void save_sensor_alarm_info()
{
	FILE *fp=fopen(CONFIG_FILE,"w");
	g_state->sensor[0]=sensor.co;
	g_state->sensor[1]=sensor.co2;
	g_state->sensor[2]=sensor.hcho;
	g_state->sensor[3]=sensor.shidu;
	g_state->sensor[4]=sensor.temp;
	g_state->sensor[5]=sensor.pm25;
	fwrite(&sensor,sizeof(struct _sensor_alarm),1,fp);
	fclose(fp);
	printf(MAIN_PROCESS"SAVE Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",sensor.co,sensor.co2,sensor.hcho,sensor.shidu,sensor.temp,sensor.pm25);
}
int main(int argc, char *argv[])
{
	int fpid;	
	long i;
	key_t shmid;
	if((history_load_done_shmid= shmget(IPC_PRIVATE, sizeof(int), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_co = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_co2 = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_hcho = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_shidu = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_temp = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_pm25 = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_co_cnt = shmget(IPC_PRIVATE, sizeof(long), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_co2_cnt = shmget(IPC_PRIVATE, sizeof(long), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_hcho_cnt = shmget(IPC_PRIVATE, sizeof(long), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_temp_cnt = shmget(IPC_PRIVATE, sizeof(long), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_shidu_cnt = shmget(IPC_PRIVATE, sizeof(long), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_pm25_cnt = shmget(IPC_PRIVATE, sizeof(long), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((state_shmid= shmget(IPC_PRIVATE, sizeof(struct state), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }
	if((shmid_zero_info = shmget(IPC_PRIVATE, sizeof(struct cur_zero_info), PERM)) == -1 )
	{
        fprintf(stderr, LCD_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);  
    }

	g_state = (struct state *)shmat(state_shmid, 0, 0);
	if(fork()==0)
	{
		history_done = (int *)shmat(history_load_done_shmid,0,0);
		load_history("/home/user/history");
		return 0;
	}
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
	memset(server_time,0,13);
	get_uuid();
	#if 0
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,1);
	sleep(3);
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,0);
	sleep(3);
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,1);
	sleep(3);
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,0);
	#endif
	fpid=fork();
	if(fpid==0)
	{
		printf(LCD_PROCESS"begin to shmat1\n");
		server_time = (char *)shmat(shmid, 0, 0);
		g_state = (struct state *)shmat(state_shmid, 0, 0);
		g_history_co = (struct nano *)shmat(shmid_co, 0, 0);
		g_history_co2 = (struct nano *)shmat(shmid_co2, 0, 0);
		g_history_hcho = (struct nano *)shmat(shmid_hcho, 0, 0);
		g_history_shidu = (struct nano *)shmat(shmid_shidu, 0, 0);
		g_history_temp = (struct nano *)shmat(shmid_temp, 0, 0);
		g_history_pm25 = (struct nano *)shmat(shmid_pm25, 0, 0);
		g_co_cnt = (long *)shmat(shmid_co_cnt, 0, 0);
		g_co2_cnt = (long *)shmat(shmid_co2_cnt, 0, 0);
		g_hcho_cnt = (long *)shmat(shmid_hcho_cnt, 0, 0);
		g_temp_cnt = (long *)shmat(shmid_temp_cnt, 0, 0);
		g_shidu_cnt = (long *)shmat(shmid_shidu_cnt, 0, 0);
		g_pm25_cnt = (long *)shmat(shmid_pm25_cnt, 0, 0);
		g_zero_info = (struct cur_zero_info *)shmat(shmid_zero_info, 0, 0);
		printf(LCD_PROCESS"end to shmat\n");
		signal(SIGALRM, set_upload_flag);
		alarm(600);
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
			printf(LCD_PROCESS"begin to shmat1\n");
			server_time = (char *)shmat(shmid, 0, 0);
			g_state = (struct state *)shmat(state_shmid, 0, 0);
			g_history_co = (struct nano *)shmat(shmid_co, 0, 0);
			g_history_co2 = (struct nano *)shmat(shmid_co2, 0, 0);
			g_history_hcho = (struct nano *)shmat(shmid_hcho, 0, 0);
			g_history_shidu = (struct nano *)shmat(shmid_shidu, 0, 0);
			g_history_temp = (struct nano *)shmat(shmid_temp, 0, 0);
			g_history_pm25 = (struct nano *)shmat(shmid_pm25, 0, 0);
			g_co_cnt = (long *)shmat(shmid_co_cnt, 0, 0);
			g_co2_cnt = (long *)shmat(shmid_co2_cnt, 0, 0);
			g_hcho_cnt = (long *)shmat(shmid_hcho_cnt, 0, 0);
			g_temp_cnt = (long *)shmat(shmid_temp_cnt, 0, 0);
			g_shidu_cnt = (long *)shmat(shmid_shidu_cnt, 0, 0);
			g_pm25_cnt = (long *)shmat(shmid_pm25_cnt, 0, 0);
			history_done = (int *)shmat(history_load_done_shmid,0,0);
			g_zero_info = (struct cur_zero_info *)shmat(shmid_zero_info, 0, 0);
			printf(LCD_PROCESS"end to shmat\n");
			signal(SIGALRM, lcd_off);
			alarm(300);
			printf("to get interface\n");
			//ask_interface();
			printf("end get interface\n");
			while(1)
			{
				lcd_loop(fd_lcd);
			}	
		}
		else if(fpid>0)
		{
			while(1)
			{
				sync_server(fd_com,0,1);
				if(server_time[0]!=0 &&server_time[5]!=0)
				{
					set_alarm(00,00,01);
					sync_server(fd_com,1,1);
				}
				else
					sleep(10);
			}
		}
	}
	return 0;
}
