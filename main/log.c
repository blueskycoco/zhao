#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include "cap.h"

#define PATH_BUF_SIZE 	256
#define LOG_LENGTH		1024
#define LOG_PATH		"/mnt/cdrom/log"
pthread_mutex_t mutex_log;
void printfLog(const char *fmt, ...)
{
	FILE *fp;
	char file_path[PATH_BUF_SIZE]={0};
	pthread_mutex_lock(&mutex_log);
	strcpy(file_path,LOG_PATH);
	memcpy(file_path+strlen(LOG_PATH),(char *)(g_share_memory->server_time),10);
	strcat(file_path,".log");
	fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		fp=fopen(file_path,"w");
		if(fp==NULL)
		{
			printf("can not create %s\r\n",file_path);
			pthread_mutex_unlock(&mutex_log);
			return;
		}	
	}
	else
	{
		fclose(fp);
		fp=fopen(file_path, "a");
	}
    va_list ap;
    char buf[LOG_LENGTH] = { 0 };
    va_start(ap, fmt);
    vsnprintf(buf, LOG_LENGTH, fmt, ap);
    va_end(ap);

	fwrite(buf,strlen(buf),1,fp);
	fclose(fp);
	pthread_mutex_unlock(&mutex_log);
}
void init_log()
{
	pthread_mutex_init(&mutex_log, NULL);  
}
#if 0
#define PROCESS1 "[PROCESS1]"
#define PROCESS2 "[PROCESS2]"
#define PROCESS_MAIN "[PROCESS_MAIN]"

int main(int argc, char *argv[])
{	
	int fpid;
	char *log_str1="process 1 log";
	char *log_str2="process 2 log";
	char *log_str_main="process main log";
	
	init_log();

	if((fpid=fork())==0)
	{
		while(1)
			printfLog("2016-04-14",PROCESS1"%s\r\n",log_str1);
		return 0;
	}
	else
		printf("[PID]%d \n",fpid);
	fpid=fork();
	if(fpid==0)
	{
		while(1)
			printfLog("2016-04-14",PROCESS2"%s\r\n",log_str2);
	}
	else if(fpid>0)
		printf("[PID]%d \n",fpid);
	
	while(1)
		printfLog("2016-04-14",PROCESS_MAIN"%s\r\n",log_str_main);
	return 0;
}
#endif
