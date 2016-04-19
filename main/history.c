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
#include "history.h"
#include "netlib.h"
#include "log.h"
#define HISTORY "[History Process]"
extern struct history sensor_history;
extern struct share_memory *g_share_memory;
extern key_t  shmid_share_memory;
extern key_t  shmid_history_co;
extern key_t  shmid_history_co2;
extern key_t  shmid_history_hcho;
extern key_t  shmid_history_temp;
extern key_t  shmid_history_shidu;
extern key_t  shmid_history_pm25;

void set_data(char *line,char *type,struct nano *history,long *cnt)
{
	char *data=doit_data(line,type);
	if(data!=NULL)
	{
		memset(history->data,'\0',10);
		if(strncmp(ID_CAP_CO2,type,strlen(ID_CAP_CO2))==0)
			sprintf(history->data,"%04d",atoi(data));
		else if(strncmp(ID_CAP_PM_25,type,strlen(ID_CAP_PM_25))==0)
			sprintf(history->data,"%03d",atoi(data));
		else
			strcpy(history->data,data);					
		free(data);
		(*cnt)++;
	}
}
void set_time(char *file_name,char *line,struct nano *history,long *cnt)
{
	char tmp[11]={0};
	memset(history->time,'\0',20);
	memcpy(tmp,file_name,10);
	strcpy(history->time,tmp);
	strcat(history->time," ");
	memset(tmp,'\0',11);
	memcpy(tmp,line,5);
	strcat(history->time,tmp);
}
void load_history(const char *name)
{
	DIR *d = NULL;
	struct dirent *de = NULL;
	char file_list[512][15];
	int i=0,j=0,m=0;	
	int cnt=0;
	char year_j[5]={0},year_m[5]={0},tmp_file[15]={0};
	char mon_j[3]={0},mon_m[3]={0};
	char day_j[3]={0},day_m[3]={0};
	
	printfLog(HISTORY"begin to shmat\n");	
	sensor_history.co 	= (struct nano *)shmat(shmid_history_co,	 0, 0);
	sensor_history.co2 	= (struct nano *)shmat(shmid_history_co2,	 0, 0);
	sensor_history.hcho = (struct nano *)shmat(shmid_history_hcho,	 0, 0);
	sensor_history.temp = (struct nano *)shmat(shmid_history_temp,	 0, 0);
	sensor_history.shidu= (struct nano *)shmat(shmid_history_shidu,	 0, 0);
	sensor_history.pm25 = (struct nano *)shmat(shmid_history_pm25,	 0, 0);
	g_share_memory 	= (struct share_memory *)shmat(shmid_share_memory,	 0, 0);
	printfLog(HISTORY"end to shmat\n");
	g_share_memory->history_done = 0;
	printfLog(HISTORY"load=>history_done %d\n",g_share_memory->history_done);
	d = opendir(name);
	if(d == 0)
	{
		printfLog(HISTORY"open failed %s , %s",name,strerror(errno));
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
				//printf(HISTORY_TAG"switch day_j %s,day_m %s,mon_j %s,mon_m %s,year_j %s,year_m %s\n",day_j,day_m,mon_j,mon_m,year_j,year_m);
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
		printfLog(HISTORY"==> %s\n",file_list[j]);
		strcpy(file_path,"/home/user/history/");
		strcat(file_path,file_list[j]);
		FILE *fp = fopen(file_path, "r");
		while (getline(&line, &len, fp) != -1) 
		{
			if((cnt%2)!=0)
			{
				//get co,co2,hcho,pm25,shidu,temp
				set_data(line,ID_CAP_CO,		&(sensor_history.co[g_share_memory->cnt[SENSOR_CO]]),	
												&(g_share_memory->cnt[SENSOR_CO]));
				set_data(line,ID_CAP_CO2,		&(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]]),	
												&(g_share_memory->cnt[SENSOR_CO2]));
				set_data(line,ID_CAP_HCHO,		&(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]]),	
												&(g_share_memory->cnt[SENSOR_HCHO]));
				set_data(line,ID_CAP_SHI_DU,	&(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]]),	
												&(g_share_memory->cnt[SENSOR_SHIDU]));
				set_data(line,ID_CAP_TEMPERATURE,&(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]]),	
												&(g_share_memory->cnt[SENSOR_TEMP]));
				set_data(line,ID_CAP_PM_25,		&(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]]),	
												&(g_share_memory->cnt[SENSOR_PM25]));
			}
			else
			{
				set_time(file_list[j],line,&(sensor_history.co[g_share_memory->cnt[SENSOR_CO]]),	
											&(g_share_memory->cnt[SENSOR_CO]));
				set_time(file_list[j],line,&(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]]),	
											&(g_share_memory->cnt[SENSOR_CO2]));
				set_time(file_list[j],line,&(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]]),	
											&(g_share_memory->cnt[SENSOR_HCHO]));
				set_time(file_list[j],line,&(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]]),	
											&(g_share_memory->cnt[SENSOR_SHIDU]));
				set_time(file_list[j],line,&(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]]),	
											&(g_share_memory->cnt[SENSOR_TEMP]));
				set_time(file_list[j],line,&(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]]),	
											&(g_share_memory->cnt[SENSOR_PM25]));
			}
			cnt++;
		}
		fclose(fp);
	}
	g_share_memory->history_done=1;
	printfLog(HISTORY"load=>history_done %d\n",g_share_memory->history_done);
}

