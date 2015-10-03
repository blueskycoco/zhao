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
//#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#define ETH_NAME "ra0"
//#include <linux/sockios.h>
#include <ifaddrs.h>
#include "cJSON.h"
#include "weblib.h"
#include "web_interface.h"
#define START_BYTE 0x6C
#define CAP_TO_ARM 0xAA
#define ARM_TO_CAP 0xBB
#define RESEND_BYTE	0x02
#define TIME_BYTE	0x01
#define ERROR_BYTE	0xFF
#define URL "http://101.200.182.92:8080/saveData/airmessage/messMgr.do"
#define FILE_PATH	"/home/lili/history/"
char server_time[20]={0};
char ip[20]={0};
char *post_message=NULL,can_send=0;
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
char *send_web(char *url,char *buf,int timeout)
{
	char request[1024]={0};
	int result=0;
	sprintf(request,"%s?JSONStr=%s",url,buf);
	printf(LOG_PREFX"send web %s\n",request);
	//char *rcv=http_post(url,buf,timeout);
	char *rcv=http_get(request,timeout);
	if(rcv!=NULL)
		printf(LOG_PREFX"rcv %s\n",rcv);
	else
		printf(LOG_PREFX"no rcv got\n");
#if 0
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
#endif
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
	strcpy(data,date+13);
	strcat(data,"\n");
	fwrite(data,strlen(data),1,fp);
	memset(data,'\0',512);
	strcpy(data,message);
	strcat(data,"\n");
	fwrite(data,strlen(data),1,fp);
	fclose(fp);
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
	memcpy(hour_end,date_end+12,2);
	memcpy(minute_end,date_end+15,2);
	month_b=atoi(month_begin);
	year_b=atoi(year_begin);
	day_b=atoi(day_begin);
	month_e=atoi(month_end);
	year_e=atoi(year_end);
	day_e=atoi(day_end);
	hour_e=atoi(hour_end);
	minute_e=atoi(minute_end);
	printf("year_b %04d,month_b %02d,day_b %02d,year_e %04d,month_e %02d,day_e %02d\r\n",year_b,month_b,day_b,year_e,month_e,day_e);
	while(1)
	{
		if(year_b<=year_e ||month_b<=month_e ||day_b<=day_e)
		{
			sprintf(date,"%04d-%02d-%02d",year_b,month_b,day_b);
			strcpy(file_path,FILE_PATH);
			memcpy(file_path+strlen(FILE_PATH),date_begin,10);
			strcat(file_path,".dat");
			fp = fopen(file_path, "r");
			if (fp != NULL)
			{
				int read=0,tmp_i=0;
				char * line = NULL;
				size_t len = 0;
				while ((read = getline(&line, &len, fp)) != -1) 
				{				
					if(year_b==year_e && month_b==month_e && day_b==day_e)
					{//check time in file
						if((tmp_i%2)==0)
						{							
							char local_hour[3]={0},local_minute[3]={0};
							memcpy(local_hour,line,2);
							memcpy(local_minute,line+3,2);
							if((atoi(local_hour)*60+atoi(local_minute))>(hour_e*60+minute_e))
							{
								printf("file_time %02d:02d,end time %02d:02d",local_hour,local_minute,hour_e,minute_e);
								free(line);
								fclose(fp);
								return;
							}
						}
						else
						{
							line[strlen(line)-1]='\0';
							printf("rsend web %s",line);
							char *rcv=send_web(URL,line,9);
							if(rcv!=NULL)
							{	
								int len1=strlen(rcv);
								printf(LOG_PREFX"<=== %s %d\n",rcv,len1);
								printf(LOG_PREFX"send ok\n");
								free(rcv);
							}
						}
					}
					else
					{
						if((tmp_i%2)!=0)
						{						
							line[strlen(line)-1]='\0';
							printf("rsend web %s",line);
							char *rcv=send_web(URL,line,9);
							if(rcv!=NULL)
							{	
								int len1=strlen(rcv);
								printf(LOG_PREFX"<=== %s %d\n",rcv,len1);
								printf(LOG_PREFX"send ok\n");
								free(rcv);
							}
						}
					}
					tmp_i++;
				}
				free(line);
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
				printf("can not open %s",file_path);
				break;
			}
		}
		else
		{
			printf("end year_b %04d,month_b %02d,day_b %02d,year_e %04d,month_e %02d,day_e %02d\r\n",year_b,month_b,day_b,year_e,month_e,day_e);
			break;
		}
	}
	fclose(fp);
}
void get_ip(char *ip)
{
	GetIP_v4_and_v6_linux(AF_INET,ip,16);
	printf("ip addrss %s\n", ip);
	return ;
}
int read_uart(int fd)
{
	int len,fs_sel,i=0,j=0,get_start=0,get_stop=0,message_len;
	char ch[100]={0};
	char id[32]={0},data[32]={0},date[32]={0},error[32]={0};
	fd_set fs_read;
	char *rcv=NULL;
	struct timeval time1;

	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);
	time1.tv_sec = 10;
	time1.tv_usec = 0;
	if(select(fd+1,&fs_read,NULL,NULL,&time1)>0)
	{
		len=0;
		j=0;
		while(1)
		{
			i=read(fd,ch+len,4-len);
			len+=i;
			if(len==4)
				break;
			else if(i==0)
			{
				j++;
				if(j==3)
					break;
				usleep(10000);
			}	
		}
		printf("read message %02X ,len %d\r\n",ch[2],ch[3]);
		message_len=ch[3];
		len=0;
		j=0;
		while(1)
		{
			i=read(fd,ch+len+4,message_len+2-len);
			len+=i;
			if(len==message_len+2)
				break;
			else if(i==0)
			{
				j++;
				if(j==3)
					break;
				usleep(10000);
			}	
		}
		printf("body %d\r\n",len);
		for(i=0;i<message_len+2+4;i++)
		{
			printf("%02x ",ch[i]);
		}
		printf("\r\n");	
		len=message_len+2;
		if(post_message==NULL)
		{
			post_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
			post_message=add_item(post_message,ID_DEVICE_UID,"230FFEE9981283737D");
			post_message=add_item(post_message,ID_DEVICE_IP_ADDR,ip);
			post_message=add_item(post_message,ID_DEVICE_PORT,"9517");	
		}
		i=0;
		//while(1)
		{
			memset(id,'\0',sizeof(id));
			memset(data,'\0',sizeof(data));
			memset(date,'\0',sizeof(date));
			memset(error,'\0',sizeof(error));
			while(ch[i]!=START_BYTE)
				i++;
			if(i>=len)
				return 0;
			//printf("ch[%d] %02x ,ch[%d+1],%02x\r\n",i,ch[i],i,ch[i+1]);
			if(ch[i]==(char)START_BYTE && ch[i+1]==(char)CAP_TO_ARM)
			{
				if(CRC_check(ch,ch[i+3]+4)==(ch[ch[i+3]+4]<<8|ch[ch[i+3]+5]))
				{
					switch(ch[2])
					{
						case TIME_BYTE:
							{
								sprintf(date,"20%02d-%02d-%02d%%20%02d:%02d",ch[i+4],ch[i+5],ch[i+6],ch[i+7],ch[i+8],ch[i+9]);
								printf("date is %s\r\n",date);
								post_message=add_item(post_message,ID_DEVICE_CAP_TIME,date);
								can_send=1;
							}
							break;
						case ERROR_BYTE:
							{
								sprintf(error,"%dth sensor possible error",ch[i+2]);
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
								if(ch[i+4]==0x45 && ch[i+5]==0x52 && ch[i+6]==0x52 && ch[i+7]==0x4f && ch[i+8]==0x52)
								{
									sprintf(error,"%dth%%20sensor%%20possible%%20error",ch[i+2]);
									post_message=add_item(post_message,ID_ALERT_CAP_FAILED,error);
								}
								else
								{
									sprintf(id,"%d",ch[i+2]);
									sprintf(data,"%d%d",ch[i+4],ch[i+5]);
									//printf("pre data %s %d\r\n",data,strlen(data));
									
									if(ch[i+6]>=strlen(data))
									{									
										sprintf(data,"0.%d%d",ch[i+4],ch[i+5]);
									}
									else if(ch[i+6]==0)
									{									
										sprintf(data,"%d%d.0",ch[i+4],ch[i+5]);
									}
									else
									{
										for(j=strlen(data);j>strlen(data)-ch[i+6]-1;j--)
										{
											data[j]=data[j-1];
											//printf("j %d %c\r\n",j,data[j]);
										}	
										data[j]='.';
									}
									printf("id %s data %s\r\n",id,data);
									post_message=add_item(post_message,id,data);
								}
							}
							break;
					}
				}
				else
					printf("CRC error Get %02x <> Count %02x\r\n",(ch[ch[i+3]+4]<<8|ch[ch[i+3]+5]),CRC_check(ch,ch[i+3]+4));
			}
			else
			{
				printf("wrong info %02x %02x\r\n",ch[0],ch[1]);
				return 0;
			}
			i=ch[i+3]+6;
		}
		if(can_send)
		{
			can_send=0;
			j=0;
			for(i=0;i<strlen(post_message);i++)
			{
				if(post_message[i]=='\n'||post_message[i]=='\r'||post_message[i]=='\t')
					j++;
			}
			printf("send post_message %s",post_message);
			char *out1=malloc(strlen(post_message)-j+1);
			memset(out1,'\0',strlen(post_message)-j+1);
			j=0;
			for(i=0;i<strlen(post_message);i++)
			{
				if(post_message[i]!='\r'&&post_message[i]!='\n'&&post_message[i]!='\t')		
				{
					out1[j++]=post_message[i];
				}
			}
			save_to_file(date,out1);
			printf("send web %s",out1);
			rcv=send_web(URL,out1,9);
			free(post_message);
			post_message=NULL;
			free(out1);
			if(rcv!=NULL)
			{	
				int len=strlen(rcv);
				printf(LOG_PREFX"<=== %s %d\n",rcv,len);
				printf(LOG_PREFX"send ok\n");
				free(rcv);
			}
		}
	}
	else
	{
		char date[128]={0};
		time_t t;
		time(&t);
		struct tm *tmtt;
		tmtt = (struct tm *)localtime(&t);
		strftime(date, 128, "%Y:%m:%d_%H:%M:%S", tmtt);
		printf("no data in %s\r\n",date);		
	}
	return len;
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

int open_com_port()
{
	int fd;
	long  vdisable;

	fd = open( "/dev/s3c2410_serial1", O_RDWR|O_NOCTTY|O_NDELAY);
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

int main(int argc, char *argv[])
{

	int fd_com=0;
	get_ip(ip);
	if((fd_com=open_com_port())<0)
	{
		perror("open_port error");
		return -1;
	}
	if(set_opt(fd_com,9600,8,'N',1)<0)
	{
		perror(" set_opt error");
		return -1;
	}
	while(1)
	{
		read_uart(fd_com);
		//sleep(3);
	}
	return 0;
}

