#include <fnmatch.h> 
#include <signal.h>
#include <sys/ipc.h>  
#include <sys/shm.h>  
#include <linux/rtc.h>
#include <sys/time.h>
#include "misc.h"
#include "cap.h"
#define MISC_PROCESS	"MISC"
#define RTCDEV 			"/dev/rtc0"

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0)
	{
		printfLog(MISC_PROCESS"SetupSerial 1");
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
		printfLog(MISC_PROCESS"com set error");
		return -1;
	}
	printfLog(MISC_PROCESS"set done!\n");
	return 0;
}
void dump_curr_time(int fd)
{
	int retval;
	struct rtc_time rtc_tm;

	/* Read the current RTC time/date */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		printfLog(MISC_PROCESS"RTC_RD_TIME ioctl error");
		exit(errno);
	}

	printfLog(MISC_PROCESS"Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
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
		printfLog(MISC_PROCESS"RTC open (RTCDEV node missing?)");
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
		printfLog(MISC_PROCESS"RTC_SET_TIME ioctl error");
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
	char cur_date[15]={0};

	fd = open(RTCDEV, O_RDONLY);

	if (fd == -1) {
		printfLog(MISC_PROCESS"RTC open (RTCDEV node missing?)");
		exit(errno);
	}

	/* Read the current RTC time/date */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		printfLog(MISC_PROCESS"RTC_RD_TIME ioctl error");
		exit(errno);
	}

	dump_curr_time(fd);
#if 1
	sprintf(cur_date,"%04d-%02d-%02d.dat",rtc_tm.tm_year+1900,rtc_tm.tm_mon+1,rtc_tm.tm_mday);
	printfLog(MISC_PROCESS"cur_date %s\n",cur_date);
	if(rtc_tm.tm_sec!=sec||rtc_tm.tm_min!=mintue||rtc_tm.tm_hour!=hour)
	{
		rtc_tm.tm_sec=sec;
		rtc_tm.tm_min=mintue;
		rtc_tm.tm_hour=hour;
	}
	else
		printfLog(MISC_PROCESS"no alarm tm_hour %02d,tm_min %02d,tm_sec %02d\r\n",rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec	);
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
	printfLog(MISC_PROCESS"tm_hour %02d,tm_min %02d,tm_sec %02d\r\n",rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);		
	retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
	if (retval == -1) {
		printfLog(MISC_PROCESS"RTC_ALM_SET ioctl error");
		exit(errno);
	}

	/* Enable alarm interrupts */
	retval = ioctl(fd, RTC_AIE_ON, 0);
	if (retval == -1) {
		printfLog(MISC_PROCESS"RTC_AIE_ON ioctl error");
		exit(errno);
	}

	printfLog(MISC_PROCESS"Alarm will trigger in %02d:%02d:%02d\n",rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);

	/* This blocks until the alarm ring causes an interrupt */
	retval = read(fd, &data, sizeof(unsigned long));
	if (retval == -1) {
		perror("read error");
		exit(errno);
	}

	/* Disable alarm interrupts */
	retval = ioctl(fd, RTC_AIE_OFF, 0);
	if (retval == -1) {
		perror("RTC_AIE_OFF ioctl error");
		exit(errno);
	}
	printfLog(MISC_PROCESS"Alarm has triggered\n");

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
		printfLog(MISC_PROCESS"Can't Open Serial ttySAC3");
		return(-1);
	}
	else 
		printfLog(MISC_PROCESS"open tts/0 .....\n");

	if(fcntl(fd, F_SETFL, FNDELAY)<0)
		printfLog(MISC_PROCESS"fcntl failed!\n");
	else
		printfLog(MISC_PROCESS"fcntl=%d\n",fcntl(fd, F_SETFL,FNDELAY));
	if(isatty(STDIN_FILENO)==0)

		printfLog(MISC_PROCESS"standard input is not a terminal device\n");
	else
		printfLog(MISC_PROCESS"isatty success!\n");
	printfLog(MISC_PROCESS"fd-open=%d\n",fd);
	return fd;
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
		printfLog(MISC_PROCESS"ping return %s %d\n",ret,strlen(ret));
		if(strstr(ret,"from")!=NULL)
		{
			g_share_memory->network_state=1;
			return 1;				
		}
	}
	g_share_memory->network_state=0;
	return 0;
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
		printfLog(MISC_PROCESS"in address");  
		return -1;
	}
	if(getifaddrs(&ifap0))
	{
		printfLog(MISC_PROCESS"getifaddrs error");  
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
				printfLog(MISC_PROCESS"address %s\n",address);
				freeifaddrs(ifap0);
				return 0;
			}
			else 
			{
				printfLog(MISC_PROCESS"inet_ntop error\r\n");
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
void get_ip()
{
	GetIP_v4_and_v6_linux(AF_INET,g_share_memory->ip,16);
	printfLog(MISC_PROCESS"ip addrss %s\n", g_share_memory->ip);
	return ;
}
void get_uuid()
{
	memset(g_share_memory->uuid,0,256);
	FILE *fp=fopen("/home/user/uuid.txt","r");
	if(fp!=NULL)
	{
		if(fread(g_share_memory->uuid,256,1,fp)<0)
			strcpy(g_share_memory->uuid,"1234abcd");
		fclose(fp);
	}
	else
		strcpy(g_share_memory->uuid,"1234abcd");
	g_share_memory->uuid[strlen(g_share_memory->uuid)-1]='\0';
	printfLog(MISC_PROCESS"uuid is %s\n",g_share_memory->uuid);
}

void get_net_interface()
{
	FILE *fp=fopen("/home/user/interface.txt","r");
	if(fp!=NULL)
	{
		if(fread(&(g_share_memory->send_by_wifi),1,1,fp)<0)
			g_share_memory->send_by_wifi=1;
		fclose(fp);
	}
	else
		g_share_memory->send_by_wifi=1;
	printfLog(MISC_PROCESS"get interface is %d\n",g_share_memory->send_by_wifi);
}

void get_sensor_alarm_info()
{
	g_share_memory->times[SENSOR_CO]		=0;
	g_share_memory->times[SENSOR_CO2]	=0;
	g_share_memory->times[SENSOR_HCHO]	=0;
	g_share_memory->times[SENSOR_SHIDU]	=0;
	g_share_memory->times[SENSOR_TEMP]	=0;
	g_share_memory->times[SENSOR_PM25]	=0;
	FILE *fp=fopen(CONFIG_FILE,"r");
	if(fp==NULL)
	{
		g_share_memory->alarm[SENSOR_CO]		=0;
		g_share_memory->alarm[SENSOR_CO2]	=0;
		g_share_memory->alarm[SENSOR_HCHO]	=0;
		g_share_memory->alarm[SENSOR_SHIDU]	=0;
		g_share_memory->alarm[SENSOR_TEMP]	=0;
		g_share_memory->alarm[SENSOR_PM25]	=0;
		g_share_memory->sent[SENSOR_CO]		=0;
		g_share_memory->sent[SENSOR_CO2]		=0;
		g_share_memory->sent[SENSOR_HCHO]	=0;
		g_share_memory->sent[SENSOR_SHIDU]	=0;
		g_share_memory->sent[SENSOR_TEMP]	=0;
		g_share_memory->sent[SENSOR_PM25]	=0;
		fp=fopen(CONFIG_FILE,"w");
		fwrite(g_share_memory->alarm,sizeof(char)*SENSOR_NO,1,fp);
		fwrite(g_share_memory->sent,sizeof(char)*SENSOR_NO,1,fp);
		fclose(fp);
		return;
	}
	fread(g_share_memory->alarm,sizeof(char)*SENSOR_NO,1,fp);
	fread(g_share_memory->sent,sizeof(char)*SENSOR_NO,1,fp);
	g_share_memory->sensor_state[SENSOR_CO]		=g_share_memory->alarm[SENSOR_CO];
	g_share_memory->sensor_state[SENSOR_CO2]	=g_share_memory->alarm[SENSOR_CO2];
	g_share_memory->sensor_state[SENSOR_HCHO]	=g_share_memory->alarm[SENSOR_HCHO];
	g_share_memory->sensor_state[SENSOR_SHIDU]	=g_share_memory->alarm[SENSOR_SHIDU];
	g_share_memory->sensor_state[SENSOR_TEMP]	=g_share_memory->alarm[SENSOR_TEMP];
	g_share_memory->sensor_state[SENSOR_PM25]	=g_share_memory->alarm[SENSOR_PM25];
	fclose(fp);
	printfLog(MISC_PROCESS"GOT Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",
		g_share_memory->alarm[SENSOR_CO],g_share_memory->alarm[SENSOR_CO2],g_share_memory->alarm[SENSOR_HCHO],
		g_share_memory->alarm[SENSOR_SHIDU],g_share_memory->alarm[SENSOR_TEMP],g_share_memory->alarm[SENSOR_PM25]);
}
void save_sensor_alarm_info()
{
	FILE *fp=fopen(CONFIG_FILE,"w");
	g_share_memory->sensor_state[SENSOR_CO]		=g_share_memory.alarm[SENSOR_CO];
	g_share_memory->sensor_state[SENSOR_CO2]	=g_share_memory.alarm[SENSOR_CO2];
	g_share_memory->sensor_state[SENSOR_HCHO]	=g_share_memory.alarm[SENSOR_HCHO];
	g_share_memory->sensor_state[SENSOR_SHIDU]	=g_share_memory.alarm[SENSOR_SHIDU];
	g_share_memory->sensor_state[SENSOR_TEMP]	=g_share_memory.alarm[SENSOR_TEMP];
	g_share_memory->sensor_state[SENSOR_PM25]	=g_share_memory.alarm[SENSOR_PM25];
	fwrite(g_share_memory->sensor_state,sizeof(char)*SENSOR_NO,1,fp);
	fwrite(g_share_memory->sent,sizeof(char)*SENSOR_NO,1,fp);
	fclose(fp);
	printfLog(MISC_PROCESS"SAVE Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",
		g_share_memory->alarm[SENSOR_CO],g_share_memory->alarm[SENSOR_CO2],g_share_memory->alarm[SENSOR_HCHO],
		g_share_memory->alarm[SENSOR_SHIDU],g_share_memory->alarm[SENSOR_TEMP],g_share_memory->alarm[SENSOR_PM25]);
}
void set_net_interface()
{
	FILE *fp=fopen("/home/user/interface.txt","w");
	fwrite(g_share_memory->send_by_wifi,1,1,fp);
	fclose(fp);
	printfLog(MISC_PROCESS"set interface is %d\n",g_share_memory->send_by_wifi);
}
int CaculateWeekDay(int y,int m, int d)
{
	if(m==1||m==2) 
	{
		m+=12;
		y--;
	}
	int iWeek=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;
	switch(iWeek)
	{
		case 0: printfLog(MISC_PROCESS"m1\n"); break;
		case 1: printfLog(MISC_PROCESS"m2\n"); break;
		case 2: printfLog(MISC_PROCESS"m3\n"); break;
		case 3: printfLog(MISC_PROCESS"m4\n"); break;
		case 4: printfLog(MISC_PROCESS"m5\n"); break;
		case 5: printfLog(MISC_PROCESS"m6\n"); break;
		case 6: printfLog(MISC_PROCESS"m7\n"); break;
	}
	return iWeek;
} 
//check crc result with cap board sent.
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
void sync_server(int resend,int set_local)
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
	printfLog(MISC_PROCESS"<sync GET>%s\n",sync_message);
	send_web_post(URL,sync_message,9,&rcv);
	free(sync_message);
	//free(out1);
	if(rcv!=NULL&&strlen(rcv)!=0)
	{	
		int len=strlen(rcv);
		printfLog(MISC_PROCESS"<=== %s\n",rcv);
		printfLog(MISC_PROCESS"send ok\n");
		char *starttime=NULL;
		char *tmp=NULL;
		if(resend)
		{

			starttime=doit_data(rcv,(char *)"101");
			tmp=doit_data(rcv,(char *)"102");
			if(starttime!=NULL && tmp!=NULL)
			{
				printfLog(MISC_PROCESS"%s\r\n",tmp);
				printfLog(MISC_PROCESS"%s\r\n",starttime);
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
				g_share_memory->server_time[0]=0x6c;g_share_memory->server_time[1]=ARM_TO_CAP;
				g_share_memory->server_time[2]=0x00;g_share_memory->server_time[3]=0x01;g_share_memory->server_time[4]=0x06;
				memcpy(year,starttime+2,2);
				memcpy(month,starttime+5,2);
				memcpy(day,starttime+8,2);
				memcpy(hour,starttime+11,2);
				memcpy(minute,starttime+14,2);
				memcpy(second,starttime+17,2);
				g_share_memory->server_time[5]=atoi(year);g_share_memory->server_time[6]=atoi(month);
				g_share_memory->server_time[7]=atoi(day);g_share_memory->server_time[8]=atoi(hour);
				g_share_memory->server_time[9]=atoi(minute);g_share_memory->server_time[10]=atoi(second);
				crc=CRC_check(g_share_memory->server_time,11);
				g_share_memory->server_time[11]=(crc&0xff00)>>8;g_share_memory->server_time[12]=crc&0x00ff;
				write(g_share_memory->fd_com,g_share_memory->server_time,13);
				printfLog(MISC_PROCESS"SERVER TIME %s\r\n",starttime);
				//tmp=doit_data(rcv+4,(char *)"211");
				//printf(SYNC_PREFX"211 %s\r\n",doit_data(rcv,"211"));
				//printf(SYNC_PREFX"212 %s\r\n",doit_data(rcv,"212"));
				if(set_local)
				set_time(g_share_memory->server_time[5]+2000,g_share_memory->server_time[6],g_share_memory->server_time[7],g_share_memory->server_time[8],g_share_memory->server_time[9],g_share_memory->server_time[10]);
				int week=CaculateWeekDay(g_share_memory->server_time[5],g_share_memory->server_time[6],g_share_memory->server_time[7]);
				char rtc_time[7];
				rtc_time[0]=(g_share_memory->server_time[5]/10)*16+(g_share_memory->server_time[5]%10);
				rtc_time[1]=(g_share_memory->server_time[6]/10)*16+(g_share_memory->server_time[6]%10);
				rtc_time[2]=(g_share_memory->server_time[7]/10)*16+(g_share_memory->server_time[7]%10);
				rtc_time[3]=week+1;
				rtc_time[4]=(g_share_memory->server_time[8]/10)*16+(g_share_memory->server_time[8]%10);
				rtc_time[5]=(g_share_memory->server_time[9]/10)*16+(g_share_memory->server_time[9]%10);
				rtc_time[6]=(g_share_memory->server_time[10]/10)*16+(g_share_memory->server_time[10]%10);
				set_lcd_time(rtc_time);
				free(starttime);
				char *user_name=doit_data(rcv,"203");
				char *user_place=doit_data(rcv,"211");
				char *user_addr=doit_data(rcv,"200");
				char *user_phone=doit_data(rcv,"202");
				char *user_contraceer=doit_data(rcv,"201");				
				char cmd[256]={0};
				clear_buf(ADDR_USER_NAME,40);
				clear_buf(ADDR_INSTALL_PLACE,60);
				clear_buf(ADDR_USER_ADDR,40);
				clear_buf(ADDR_USER_PHONE,40);
				clear_buf(ADDR_USER_CONTACTER,40);
				if(user_name && strlen(user_name)>0)
				{
					code_convert("utf-8","gbk",user_name,strlen(user_name),cmd,256);
					write_string(ADDR_USER_NAME,cmd,strlen(cmd));
					printfLog(MISC_PROCESS"user_name:%s\n",user_name);
					free(user_name);
				}
				if(user_place && strlen(user_place)>0)
				{		
				    code_convert("utf-8","gbk",user_place,strlen(user_place),cmd,256);
					write_string(ADDR_INSTALL_PLACE,cmd,strlen(cmd));
					printfLog(MISC_PROCESS"user_place:%s\n",user_place);
					free(user_place);
				}
				if(user_addr && strlen(user_addr)>0)
				{
				
					code_convert("utf-8","gbk",user_addr,strlen(user_addr),cmd,256);
					write_string(ADDR_USER_ADDR,cmd,strlen(cmd));					
					printfLog(MISC_PROCESS"user_addr:%s\n",user_addr);
					free(user_addr);
				}
				if(user_phone && strlen(user_phone)>0)
				{
					code_convert("utf-8","gbk",user_phone,strlen(user_phone),cmd,256);
					write_string(ADDR_USER_PHONE,cmd,strlen(cmd));					
					printfLog(MISC_PROCESS"user_phone:%s\n",user_phone);
					free(user_phone);
				}
				if(user_contraceer && strlen(user_contraceer)>0)
				{
					code_convert("utf-8","gbk",user_contraceer,strlen(user_contraceer),cmd,256);
					write_string(ADDR_USER_CONTACTER,cmd,strlen(cmd));
					printfLog(MISC_PROCESS"user_contraceer:%s\n",user_contraceer);
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

void ask_interface()
{
	int i = 0;
	char cmd[] = {0x6c,ARM_TO_CAP,0x00,0x06,0x00,0x00,0x00};	
	int crc=CRC_check(cmd,5);
	cmd[5]=(crc&0xff00)>>8;cmd[6]=crc&0x00ff; 	
	printfLog(MISC_PROCESS"going to ask_interface begin\n");
	for(i=0;i<7;i++)
		printfLog(MISC_PROCESS"%02x ",cmd[i]);
	printfLog(MISC_PROCESS"\ngoing to ask_interface end\n");
	write(g_share_memory->fd_com,cmd,sizeof(cmd));
	i=0;
	while(1)
	{
		if(i>20)
			break;
		sleep(1);
		if(g_share_memory->sensor_interface_mem[0]==0x0000)
			break;
		else
			write(g_share_memory->fd_com,cmd,sizeof(cmd));
		i++;
			
	}
}

