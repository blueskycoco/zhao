#include "history.h"
#include "netlib.h"
#define HISTORY_TAG "[History Process]"

void set_data(char *line,char *type,struct nano *history,int *cnt)
{
	char *data=doit_data(line,type);
	if(data!=NULL)
	{
		memset(history[*cnt].data,'\0',10);
		if(strncmp(ID_CAP_CO2,type,strlen(ID_CAP_CO2))==0)
			sprintf(history[*cnt].data,"%04d",atoi(data));
		else if(strncmp(ID_CAP_PM_25,type,strlen(ID_CAP_PM_25))==0)
			sprintf(history[*cnt].data,"%03d",atoi(data));
		else
			strcpy(history[*cnt].data,data);					
		free(data);
		(*cnt)++;
	}
}
void set_time(char *file_name,char *line,struct nano *history,int *cnt)
{
	char tmp[11]={0};
	memset(history[*cnt].time,'\0',20);
	memcpy(tmp,file_name,10);
	strcpy(history[*cnt].time,tmp);
	strcat(history[*cnt].time," ");
	memset(tmp,'\0',11);
	memcpy(tmp,line,5);
	strcat(history[*cnt].time,tmp);
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
	
	printf(HISTORY_TAG"begin to shmat\n");
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
	printf(HISTORY_TAG"end to shmat\n");
	*history_done=0;
	printf(HISTORY_TAG"load=>history_done %d\n",*history_done);
	d = opendir(name);
	if(d == 0)
	{
		printf(HISTORY_TAG"open failed %s , %s",name,strerror(errno));
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
		printf(HISTORY_TAG"==> %s\n",file_list[j]);
		strcpy(file_path,"/home/user/history/");
		strcat(file_path,file_list[j]);
		FILE *fp = fopen(file_path, "r");
		while (getline(&line, &len, fp) != -1) 
		{
			if((cnt%2)!=0)
			{
				//get co,co2,hcho,pm25,shidu,temp
				set_data(line,ID_CAP_CO,g_history_co,g_co_cnt);
				set_data(line,ID_CAP_CO2,g_history_co2,g_co2_cnt);
				set_data(line,ID_CAP_HCHO,g_history_hcho,g_hcho_cnt);
				set_data(line,ID_CAP_SHI_DU,g_history_shidu,g_shidu_cnt);
				set_data(line,ID_CAP_TEMPERATURE,g_history_temp,g_temp_cnt);
				set_data(line,ID_CAP_PM_25,g_history_pm25,g_pm25_cnt);
			}
			else
			{
				set_time(file_list[j],line,g_history_co,g_co_cnt);
				set_time(file_list[j],line,g_history_co2,g_co2_cnt);
				set_time(file_list[j],line,g_history_temp,g_temp_cnt);
				set_time(file_list[j],line,g_history_hcho,g_hcho_cnt);
				set_time(file_list[j],line,g_history_shidu,g_shidu_cnt);
				set_time(file_list[j],line,g_history_pm25,g_pm25_cnt);
			}
			cnt++;
		}
		fclose(fp);
	}
	*history_done=1;
	printf(HISTORY_TAG"load=>history_done %d\n",*history_done);
}

