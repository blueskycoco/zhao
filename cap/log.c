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

#define PATH_BUF_SIZE 	256
#define LOG_LENGTH		1024
#define LOG_PATH		"/mnt/cdrom/log"
char log_name[256]={0};
pthread_mutex_t mutex_log;
void printfLog(const char *fmt, ...)
{
	FILE *fp;
	char file_path[PATH_BUF_SIZE]={0};
#ifdef NO_LOG
	return;
#endif
	if(strlen(log_name)==0)
		return;
	pthread_mutex_lock(&mutex_log);
	fp = fopen(log_name, "r");
	if (fp == NULL)
	{
		fp=fopen(log_name,"w");
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
		fp=fopen(log_name, "a");
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
void init_log(const char *time)
{
#ifdef NO_LOG
	return ;
#endif
	pthread_mutex_init(&mutex_log, NULL);  
	strcpy(log_name,LOG_PATH);
	strcat(log_name,time);
	strcat(log_name,".log");
	printfLog("\n\n\nBegin LOG %s ====>\n",time);
}
