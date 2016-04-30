#include "cap.h"
#include "netlib.h"
#include "xfer.h"
#include "misc.h"
#include "dwin.h"
#define MISC_PROCESS	"[MISC] "
#define RTCDEV 			"/dev/rtc0"
#define ETH_NAME "ra0"
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
void read_curr_time(char *out)
{
	int retval;
	struct rtc_time rtc_tm;
	int fd;
	fd = open(RTCDEV, O_RDWR);
	if(fd==-1)
		return;
	/* Read the current RTC time/date */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		printfLog(MISC_PROCESS"RTC_RD_TIME ioctl error");
		exit(errno);
	}

	printfLog(MISC_PROCESS"Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
			rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
			rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
	if(out!=NULL)
	{
		sprintf(out,"%04d-%02d-%02d",rtc_tm.tm_year+1900,rtc_tm.tm_mon+1,rtc_tm.tm_mday);
	}
	close(fd);
}

void set_time(int year,int mon,int day,int hour,int minute,int second)
{	
	int fd, retval;
	struct rtc_time rtc_tm;
	//unsigned long data;

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
	//long  vdisable;
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
	//int ret;
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
			printfLog(MISC_PROCESS"can not create %s\r\n",file_path);
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

void get_net_interface()
{
	FILE *fp=fopen("/home/user/interface.txt","r");
	if(fp!=NULL)
	{
		if(fread(&(g_share_memory->send_by_wifi),1,1,fp)<0)
			g_share_memory->send_by_wifi=1;
		if(fread(&(g_share_memory->sleep),1,1,fp)<0)
			g_share_memory->sleep=5;
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
	g_share_memory->times[SENSOR_WIND]		=0;
	g_share_memory->times[SENSOR_NOISE]	=0;
	g_share_memory->times[SENSOR_PRESS]	=0;
	g_share_memory->times[SENSOR_TVOC]	=0;
	g_share_memory->times[SENSOR_O3]	=0;
	g_share_memory->times[SENSOR_PM10]	=0;

	FILE *fp=fopen(CONFIG_FILE,"r");
	if(fp==NULL)
	{
		g_share_memory->alarm[SENSOR_CO]	=0;
		g_share_memory->alarm[SENSOR_CO2]	=0;
		g_share_memory->alarm[SENSOR_HCHO]	=0;
		g_share_memory->alarm[SENSOR_SHIDU]	=0;
		g_share_memory->alarm[SENSOR_TEMP]	=0;
		g_share_memory->alarm[SENSOR_PM25]	=0;
		g_share_memory->alarm[SENSOR_WIND]	=0;
		g_share_memory->alarm[SENSOR_NOISE]	=0;
		g_share_memory->alarm[SENSOR_PRESS]	=0;
		g_share_memory->alarm[SENSOR_TVOC]	=0;
		g_share_memory->alarm[SENSOR_O3]	=0;
		g_share_memory->alarm[SENSOR_PM10]	=0;

		g_share_memory->sent[SENSOR_CO]		=0;
		g_share_memory->sent[SENSOR_CO2]	=0;
		g_share_memory->sent[SENSOR_HCHO]	=0;
		g_share_memory->sent[SENSOR_SHIDU]	=0;
		g_share_memory->sent[SENSOR_TEMP]	=0;
		g_share_memory->sent[SENSOR_PM25]	=0;
		g_share_memory->sent[SENSOR_WIND]		=0;
		g_share_memory->sent[SENSOR_NOISE]	=0;
		g_share_memory->sent[SENSOR_PRESS]	=0;
		g_share_memory->sent[SENSOR_TVOC]	=0;
		g_share_memory->sent[SENSOR_O3]	=0;
		g_share_memory->sent[SENSOR_PM10]	=0;

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
	g_share_memory->sensor_state[SENSOR_WIND]	=g_share_memory->alarm[SENSOR_WIND];
	g_share_memory->sensor_state[SENSOR_NOISE]	=g_share_memory->alarm[SENSOR_NOISE];
	g_share_memory->sensor_state[SENSOR_PRESS]	=g_share_memory->alarm[SENSOR_PRESS];
	g_share_memory->sensor_state[SENSOR_TVOC]	=g_share_memory->alarm[SENSOR_TVOC];
	g_share_memory->sensor_state[SENSOR_O3]		=g_share_memory->alarm[SENSOR_O3];
	g_share_memory->sensor_state[SENSOR_PM10]	=g_share_memory->alarm[SENSOR_PM10];

	fclose(fp);
	printfLog(MISC_PROCESS"GOT Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d,wind %d, noise %d, press %d, tvoc %d, o3 %d, pm10 %d\n",
		g_share_memory->alarm[SENSOR_CO],g_share_memory->alarm[SENSOR_CO2],g_share_memory->alarm[SENSOR_HCHO],
		g_share_memory->alarm[SENSOR_SHIDU],g_share_memory->alarm[SENSOR_TEMP],g_share_memory->alarm[SENSOR_PM25],
		g_share_memory->alarm[SENSOR_WIND],g_share_memory->alarm[SENSOR_NOISE],g_share_memory->alarm[SENSOR_PRESS],
		g_share_memory->alarm[SENSOR_TVOC],g_share_memory->alarm[SENSOR_O3],g_share_memory->alarm[SENSOR_PM10]);
}
void save_sensor_alarm_info()
{
	FILE *fp=fopen(CONFIG_FILE,"w");
	g_share_memory->sensor_state[SENSOR_CO]		=g_share_memory->alarm[SENSOR_CO];
	g_share_memory->sensor_state[SENSOR_CO2]	=g_share_memory->alarm[SENSOR_CO2];
	g_share_memory->sensor_state[SENSOR_HCHO]	=g_share_memory->alarm[SENSOR_HCHO];
	g_share_memory->sensor_state[SENSOR_SHIDU]	=g_share_memory->alarm[SENSOR_SHIDU];
	g_share_memory->sensor_state[SENSOR_TEMP]	=g_share_memory->alarm[SENSOR_TEMP];
	g_share_memory->sensor_state[SENSOR_PM25]	=g_share_memory->alarm[SENSOR_PM25];
	g_share_memory->sensor_state[SENSOR_WIND]	=g_share_memory->alarm[SENSOR_WIND];
	g_share_memory->sensor_state[SENSOR_NOISE]	=g_share_memory->alarm[SENSOR_NOISE];
	g_share_memory->sensor_state[SENSOR_PRESS]	=g_share_memory->alarm[SENSOR_PRESS];
	g_share_memory->sensor_state[SENSOR_TVOC]	=g_share_memory->alarm[SENSOR_TVOC];
	g_share_memory->sensor_state[SENSOR_O3]		=g_share_memory->alarm[SENSOR_O3];
	g_share_memory->sensor_state[SENSOR_PM10]	=g_share_memory->alarm[SENSOR_PM10];
	
	fwrite(g_share_memory->sensor_state,sizeof(char)*SENSOR_NO,1,fp);
	fwrite(g_share_memory->sent,sizeof(char)*SENSOR_NO,1,fp);
	fclose(fp);
	printfLog(MISC_PROCESS"SAVE Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d,wind %d, noise %d, press %d, tvoc %d, o3 %d, pm10 %d\n",
		g_share_memory->alarm[SENSOR_CO],g_share_memory->alarm[SENSOR_CO2],g_share_memory->alarm[SENSOR_HCHO],
		g_share_memory->alarm[SENSOR_SHIDU],g_share_memory->alarm[SENSOR_TEMP],g_share_memory->alarm[SENSOR_PM25],
		g_share_memory->alarm[SENSOR_WIND],g_share_memory->alarm[SENSOR_NOISE],g_share_memory->alarm[SENSOR_PRESS],
		g_share_memory->alarm[SENSOR_TVOC],g_share_memory->alarm[SENSOR_O3],g_share_memory->alarm[SENSOR_PM10]);
}
void set_net_interface()
{
	FILE *fp=fopen("/home/user/interface.txt","w");
	fwrite(&(g_share_memory->send_by_wifi),1,1,fp);
	fwrite(&(g_share_memory->sleep),1,1,fp);
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
void resend_history_done(char *begin,char *end)
{
	char *resend_done=NULL;						
	resend_done=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_RE_DATA);
	resend_done=add_item(resend_done,ID_DEVICE_UID,g_share_memory->uuid);
	resend_done=add_item(resend_done,ID_DEVICE_IP_ADDR,g_share_memory->ip);
	resend_done=add_item(resend_done,ID_DEVICE_PORT,(char *)"9517");
	resend_done=add_item(resend_done,ID_RE_START_TIME,begin);
	resend_done=add_item(resend_done,ID_RE_STOP_TIME,end);
	char *rcv=NULL;
	send_web_post(URL,resend_done,9,&rcv);
	free(resend_done);
	resend_done=NULL;
	if(rcv!=NULL)
	{	
		//int len=strlen(rcv);
		free(rcv);
		rcv=NULL;
	}	
}

void resend_history(char *date_begin,char *date_end)
{
	FILE *fp;
	int month_b,year_b,day_b,month_e,year_e,day_e,hour_e,minute_e,max_day;
	char year_begin[5]={0};
	char year_end[5]={0};
	char month_begin[3]={0};
	char month_end[3]={0};
	char day_begin[3]={0};
	char day_end[3]={0};
	char hour_end[3]={0};
	char minute_end[3]={0};
	char file_path[256]={0};
	//char data[512]={0};
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
	printfLog(MISC_PROCESS"year_b %04d,month_b %02d,day_b %02d,year_e %04d,month_e %02d,day_e %02d\r\n",year_b,month_b,day_b,year_e,month_e,day_e);
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
			printfLog(MISC_PROCESS"to open %s\r\n",file_path);
			fp = fopen(file_path, "r");
			if (fp != NULL)
			{
				int read=0,tmp_i=0;
				char * line = NULL;
				size_t len = 0;
				printfLog(MISC_PROCESS"open file %s ok\r\n",file_path);
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
								printfLog(MISC_PROCESS"file_time %02d:%02d,end time %02d:%02d",atoi(local_hour),atoi(local_minute),hour_e,minute_e);
								free(line);
								fclose(fp);
								resend_history_done(date_begin,date_end);
								return;
							}
						}
						else
						{
							line[strlen(line)-1]='\0';							
							printfLog(MISC_PROCESS"[rsend web]\n");
							while(1){
								char *rcv=NULL;
								send_web_post(URL,line,39,&rcv);
								if(rcv!=NULL)
								{	
									//int len1=strlen(rcv);
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
							printfLog(MISC_PROCESS"[rsend web]\n");
							while(1){
								char *rcv=NULL;
								send_web_post(URL,line,9,&rcv);
								if(rcv!=NULL)
								{	
									//int len1=strlen(rcv);
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
				printfLog(MISC_PROCESS"can not open %s\r\n",file_path);
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
			printfLog(MISC_PROCESS"end year_b %04d,month_b %02d,day_b %02d,year_e %04d,month_e %02d,day_e %02d\r\n",year_b,month_b,day_b,year_e,month_e,day_e);
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
//	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) 
	{
		printfLog(MISC_PROCESS"iconv_open failed\n");
		return -1;
	}
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen)==-1)
	{
		printfLog(MISC_PROCESS"iconv failed\n");
		return -1;
	}
	iconv_close(cd);
	return 0;
}
int ping_server_by_gprs()
{
	char *sync_message=NULL,*rcv=NULL,xfer_mode=0;
	sync_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_SYNC);
	sync_message=add_item(sync_message,ID_DEVICE_UID,g_share_memory->uuid);
	sync_message=add_item(sync_message,ID_DEVICE_IP_ADDR,g_share_memory->ip);
	sync_message=add_item(sync_message,ID_DEVICE_PORT,(char *)"9517");
	printfLog(MISC_PROCESS"<ping_server_by_gprs>%s\n",sync_message);
	xfer_mode=g_share_memory->send_by_wifi;
	printfLog(MISC_PROCESS"xfer_mode %d\n",xfer_mode);
	g_share_memory->send_by_wifi=0;
	send_web_post(URL,sync_message,9,&rcv);
	g_share_memory->send_by_wifi=xfer_mode;
	free(sync_message);
	if(rcv!=NULL&&strlen(rcv)!=0)
	{	
		free(rcv);
		return 1;
	}

	return 0;
}
void sync_server(int resend,int set_local)
{
	//int i,j;
	//char text_out[512]={0};
	char *sync_message=NULL,*rcv=NULL;
	if(resend)
		sync_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_ASK_RE_DATA);
	else
		sync_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_SYNC);
	sync_message=add_item(sync_message,ID_DEVICE_UID,g_share_memory->uuid);
	sync_message=add_item(sync_message,ID_DEVICE_IP_ADDR,g_share_memory->ip);
	sync_message=add_item(sync_message,ID_DEVICE_PORT,(char *)"9517");
	printfLog(MISC_PROCESS"<sync GET>%s\n",sync_message);
	send_web_post(URL,sync_message,9,&rcv);
	free(sync_message);
	//free(out1);
	if(rcv!=NULL&&strlen(rcv)!=0)
	{	
		//int len=strlen(rcv);
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
				crc=CRC_check((unsigned char *)g_share_memory->server_time,11);
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
				clear_buf(ADDR_INFO_USER_NAME,40);
				clear_buf(ADDR_INFO_INSTALL_PLACE,60);
				clear_buf(ADDR_INFO_ADDR,40);
				clear_buf(ADDR_INFO_PHONE,40);
				clear_buf(ADDR_INFO_CONTACTER,40);
				if(user_name && strlen(user_name)>0)
				{
					code_convert("utf-8","gbk",user_name,strlen(user_name),cmd,256);
					write_string(ADDR_INFO_USER_NAME,cmd,strlen(cmd));
					printfLog(MISC_PROCESS"user_name:%s\n",user_name);
					free(user_name);
				}
				if(user_place && strlen(user_place)>0)
				{		
				    code_convert("utf-8","gbk",user_place,strlen(user_place),cmd,256);
					write_string(ADDR_INFO_INSTALL_PLACE,cmd,strlen(cmd));
					printfLog(MISC_PROCESS"user_place:%s\n",user_place);
					free(user_place);
				}
				if(user_addr && strlen(user_addr)>0)
				{
				
					code_convert("utf-8","gbk",user_addr,strlen(user_addr),cmd,256);
					write_string(ADDR_INFO_ADDR,cmd,strlen(cmd));					
					printfLog(MISC_PROCESS"user_addr:%s\n",user_addr);
					free(user_addr);
				}
				if(user_phone && strlen(user_phone)>0)
				{
					code_convert("utf-8","gbk",user_phone,strlen(user_phone),cmd,256);
					write_string(ADDR_INFO_PHONE,cmd,strlen(cmd));					
					printfLog(MISC_PROCESS"user_phone:%s\n",user_phone);
					free(user_phone);
				}
				if(user_contraceer && strlen(user_contraceer)>0)
				{
					code_convert("utf-8","gbk",user_contraceer,strlen(user_contraceer),cmd,256);
					write_string(ADDR_INFO_CONTACTER,cmd,strlen(cmd));
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
	int crc=CRC_check((unsigned char *)cmd,5);
	cmd[5]=(crc&0xff00)>>8;cmd[6]=crc&0x00ff; 	
	printfLog(MISC_PROCESS"going to ask_interface begin\n");
	for(i=0;i<7;i++)
		printfLog("%02x ",cmd[i]);
	write(g_share_memory->fd_com,cmd,sizeof(cmd));
	i=0;
	while(1)
	{
		if(i>20)
			break;
		sleep(1);
		printfLog(MISC_PROCESS"sensor_interface_mem[0] %x\n",g_share_memory->sensor_interface_mem[0]);
		if(g_share_memory->sensor_interface_mem[0]==0x0000)
			break;
		else
			write(g_share_memory->fd_com,cmd,sizeof(cmd));
		i++;
			
	}
	printfLog(MISC_PROCESS"\ngoing to ask_interface end\n");
}
void buzzer(int data)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x02,0x00};
	cmd[5]=data;
	write(g_share_memory->fd_lcd,cmd,6);
}
void cut_pic(int on)
{
	char cmd[]={0x5a,0xa5,0x15,0x82,0x06,0xf3,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8a,0x00,0xc5,0x03,0x50,0x01,0x11,0x00,0x78,0x00,0xd0};
	
	char off[]={0x5a,0xa5,0x15,0x82,0x06,0xf3,0x00,0x06,0x00,0x01,0x00,0x01,
				0x00,0x78,0x00,0xd0,0x01,0x3f,0x01,0x1e,0x00,0x78,0x00,0xd0};
	if(on)
		write(g_share_memory->fd_lcd,cmd,sizeof(cmd));
	else
		write(g_share_memory->fd_lcd,off,sizeof(off));
}
void gprs_state(int state,int pic)
{
	char good[]={0x5a,0xa5,0x15,0x82,0x06,0xf3,0x00,0x06,0x00,0x01,0x00,0x22,
				0x01,0xae,0x01,0x76,0x02,0x75,0x01,0xc3,0x01,0xa8,0x01,0x76};
	
	char bad[]={0x5a,0xa5,0x15,0x82,0x06,0xf3,0x00,0x06,0x00,0x01,0x00,0x22,
				0x02,0x87,0x01,0x76,0x03,0x4e,0x01,0xc3,0x02,0x81,0x01,0x76};
	if(pic==STATE_PAGE_O_O)
	{
		good[4]=0x0b;good[5]=0xb3;
		bad[4]=0x0b;bad[5]=0xbc;
	}
	else if(pic==STATE_PAGE_I_O)
	{
		good[4]=0x0b;good[5]=0xc5;
		bad[4]=0x0b;bad[5]=0xce;
	}
	else if(pic==STATE_PAGE_I_I)
	{
		good[4]=0x0b;good[5]=0xd7;
		bad[4]=0x0b;bad[5]=0xe0;
	}
	else if(pic==STATE_PAGE_O_I)
	{
		good[4]=0x0b;good[5]=0xe9;
		bad[4]=0x0b;bad[5]=0xf2;
	}
	if(state)//gprs good
	{
		good[12]=0x01;good[13]=0xa7;
		good[14]=0x01;good[15]=0x05;
		good[16]=0x02;good[17]=0x7c;
		good[18]=0x01;good[19]=0x5e;
		bad[12]=0x02;bad[13]=0x80;
		bad[14]=0x01;bad[15]=0x08;
		bad[16]=0x03;bad[17]=0x56;
		bad[18]=0x01;bad[19]=0x67;
	}
	else//gprs bad
	{
		good[12]=0x01;good[13]=0xab;
		good[14]=0x00;good[15]=0xa4;
		good[16]=0x02;good[17]=0x7d;
		good[18]=0x00;good[19]=0xf5;
		bad[12]=0x02;bad[13]=0x80;
		bad[14]=0x00;bad[15]=0x9e;
		bad[16]=0x03;bad[17]=0x4f;
		bad[18]=0x00;bad[19]=0xef;
	}
	printfLog(MISC_PROCESS"gprs_state %d %d ,fd_lcd %d\n",state,pic,g_share_memory->fd_lcd);
	write(g_share_memory->fd_lcd,good,sizeof(good));
	write(g_share_memory->fd_lcd,bad,sizeof(bad));
}
void co_flash_alarm()
{
	int fpid=0;
	if((fpid=fork())==0)
	{
		switch_pic(MAIN_PAGE);
		buzzer(0x30);
		cut_pic(1);
		sleep(3);
		buzzer(0x30);
		cut_pic(0);
		sleep(3);
		buzzer(0x30);
		cut_pic(1);
		sleep(3);
		buzzer(0x30);
		cut_pic(0);
		return ;
	}
	else
		printfLog(MISC_PROCESS"[PID]%d co flash process\n",fpid);
	return ;
}
