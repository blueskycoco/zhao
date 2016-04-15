#include <fnmatch.h> 
#include <signal.h>
#include <sys/ipc.h>  
#include <sys/shm.h>  
#include <linux/rtc.h>
#include <sys/time.h>
#include "misc.h"
#define MISC_PROCESS	"MISC"
#define RTCDEV 			"/dev/rtc0"
extern sensor_alarm sensor;
extern struct state *g_state;
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
			g_state->network_state=1;
			return 1;				
		}
	}
	g_state->network_state=0;
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
void get_ip(char *ip)
{
	GetIP_v4_and_v6_linux(AF_INET,ip,16);
	printfLog(MISC_PROCESS"ip addrss %s\n", ip);
	return ;
}

void get_sensor_alarm_info()
{
	sensor.times[SENSOR_CO]		=0;
	sensor.times[SENSOR_CO2]	=0;
	sensor.times[SENSOR_HCHO]	=0;
	sensor.times[SENSOR_SHIDU]	=0;
	sensor.times[SENSOR_TEMP]	=0;
	sensor.times[SENSOR_PM25]	=0;
	FILE *fp=fopen(CONFIG_FILE,"r");
	if(fp==NULL)
	{
		sensor.alarm[SENSOR_CO]		=0;
		sensor.alarm[SENSOR_CO2]	=0;
		sensor.alarm[SENSOR_HCHO]	=0;
		sensor.alarm[SENSOR_SHIDU]	=0;
		sensor.alarm[SENSOR_TEMP]	=0;
		sensor.alarm[SENSOR_PM25]	=0;
		sensor.sent[SENSOR_CO]		=0;
		sensor.sent[SENSOR_CO2]		=0;
		sensor.sent[SENSOR_HCHO]	=0;
		sensor.sent[SENSOR_SHIDU]	=0;
		sensor.sent[SENSOR_TEMP]	=0;
		sensor.sent[SENSOR_PM25]	=0;
		fp=fopen(CONFIG_FILE,"w");
		fwrite(&sensor,sizeof(struct _sensor_alarm),1,fp);	
		fclose(fp);
		return;
	}
	fread(&sensor,sizeof(struct _sensor_alarm),1,fp);
	g_state->sensor_state[SENSOR_CO]	=sensor.alarm[SENSOR_CO];
	g_state->sensor_state[SENSOR_CO2]	=sensor.alarm[SENSOR_CO2];
	g_state->sensor_state[SENSOR_HCHO]	=sensor.alarm[SENSOR_HCHO];
	g_state->sensor_state[SENSOR_SHIDU]	=sensor.alarm[SENSOR_SHIDU];
	g_state->sensor_state[SENSOR_TEMP]	=sensor.alarm[SENSOR_TEMP];
	g_state->sensor_state[SENSOR_PM25]	=sensor.alarm[SENSOR_PM25];
	fclose(fp);
	printfLog(MISC_PROCESS"GOT Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",
		sensor.alarm[SENSOR_CO],sensor.alarm[SENSOR_CO2],sensor.alarm[SENSOR_HCHO],
		sensor.alarm[SENSOR_SHIDU],sensor.alarm[SENSOR_TEMP],sensor.alarm[SENSOR_PM25]);
}
void save_sensor_alarm_info()
{
	FILE *fp=fopen(CONFIG_FILE,"w");
	g_state->sensor_state[SENSOR_CO]	=sensor.alarm[SENSOR_CO];
	g_state->sensor_state[SENSOR_CO2]	=sensor.alarm[SENSOR_CO2];
	g_state->sensor_state[SENSOR_HCHO]	=sensor.alarm[SENSOR_HCHO];
	g_state->sensor_state[SENSOR_SHIDU]	=sensor.alarm[SENSOR_SHIDU];
	g_state->sensor_state[SENSOR_TEMP]	=sensor.alarm[SENSOR_TEMP];
	g_state->sensor_state[SENSOR_PM25]	=sensor.alarm[SENSOR_PM25];
	fwrite(&sensor,sizeof(struct _sensor_alarm),1,fp);
	fclose(fp);
	printfLog(MISC_PROCESS"SAVE Alarm_Config co %d, co2 %d, hcho %d,shidu %d, temp %d, pm25 %d\n",
		sensor.alarm[SENSOR_CO],sensor.alarm[SENSOR_CO2],sensor.alarm[SENSOR_HCHO],
		sensor.alarm[SENSOR_SHIDU],sensor.alarm[SENSOR_TEMP],sensor.alarm[SENSOR_PM25]);
}

