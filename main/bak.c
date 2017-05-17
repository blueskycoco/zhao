#include <stdio.h>
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
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
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

#include "log.h"
#define RTCDEV 			"/dev/rtc0"
#define CAP_IMX280 		"/home/user/cap-imx280"
#define CAP_IMX280_BAK	"/home/user/cap-imx280-bak"
#define WPA_FILE		"/etc/wpa_supplicant.conf"
#define WPA_FILE_BAK	"/home/user/wpa_supplicant.conf-bak"
#if 0
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
		printf("RTC_RD_TIME ioctl error");
		exit(errno);
	}

	printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
			rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
			rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
	if(out!=NULL)
	{
		sprintf(out,"%04d-%02d-%02d_%02d_%02d_%02d",rtc_tm.tm_year+1900,rtc_tm.tm_mon+1,rtc_tm.tm_mday,
				rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
	}
	close(fd);
}
#endif
int file_len(char *path)
{
	FILE *fp;
	int flen = 0;
	if (access(path, F_OK) == 0) {
		fp = fopen(path, "r");
		fseek(fp,0L,SEEK_END);
		flen=ftell(fp);
		fseek(fp,0L,SEEK_SET);
		fclose(fp);
		printfLog("check %s, len %d\n", path, flen);	
	} else {
		printfLog("file %s is not exist\n",path);
	}

	return flen;
}
int main(void)
{
	int flen_ori,flen_bak;
	char file[256] = {0};
	char cmd[256] = {0};
	//read_curr_time(file);
	strcpy(file,"_file");
	init_log(file);
	printfLog("check system files\n");
	
	flen_ori = file_len(CAP_IMX280);
	flen_bak = file_len(CAP_IMX280_BAK);
	if (flen_ori == 0)
	{
		if (flen_bak != 0)
		{
			printfLog("copy from %s to %s\n", CAP_IMX280_BAK,
					CAP_IMX280);
			sprintf(cmd, "cp -f %s %s", CAP_IMX280_BAK, CAP_IMX280);
			system(cmd);
		}
		else
			printfLog("there is no system bak file , hung\n");
	}
	else
	{
		printfLog("copy from %s to %s\n", CAP_IMX280,
					CAP_IMX280_BAK);
		sprintf(cmd, "cp -f %s %s", CAP_IMX280, CAP_IMX280_BAK);
		system(cmd);
	}
	printfLog("check configuration files\n");
	flen_ori = file_len(WPA_FILE);
	flen_bak = file_len(WPA_FILE_BAK);
	if (flen_ori == 0 || flen_ori < flen_bak)
	{
		if (flen_bak != 0)
		{
			printfLog("copy from %s to %s\n", WPA_FILE_BAK,
					WPA_FILE);
			sprintf(cmd, "cp -f %s %s", WPA_FILE_BAK, WPA_FILE);
			system(cmd);
		}
		else
			printfLog("there is no configuration bak file , hung\n");
	}
	else
	{
		printfLog("copy from %s to %s\n", WPA_FILE,
					WPA_FILE_BAK);
		sprintf(cmd, "cp -f %s %s", WPA_FILE, WPA_FILE_BAK);
		system(cmd);
	}

	return 0;
}
