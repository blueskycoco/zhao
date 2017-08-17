#include "cap.h"
#include "misc.h"
#include "dwin.h"
#include "netlib.h"
#include "xfer.h"
#include <sys/msg.h>

#define CAP_PROCESS "[CAP_PROCESS] "
int g_upload=0;

char *post_message=NULL,*warnning_msg=NULL;
struct msg_st  
{  
	long int msg_type;
	int len;
	char text[512];  
}; 

//set g_upload flag to make 10 mins upload once.
void set_upload_flag(int a)
{
	printfLog(CAP_PROCESS"set upload flag\n");
	g_upload=1;
	alarm(20);
}
//format sensor history data
void set_upload_data(char *id,struct nano *history,long *cnt,char *data,char *date)
{
	//printfLog(CAP_PROCESS"set_upload_data id %s,cnt %d,data %s,date %s\n",id,*cnt,data,date);
	memset(history->data,'\0',10);
	if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
		sprintf(history->data,"%04d",atoi(data+2));
	else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
		sprintf(history->data,"%03d",atoi(data));
	else				
		strcpy(history->data,data);	
	memset(history->time,'\0',20);
	strcpy(history->time,date);							
	free(data);
	(*cnt)++;
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
		char *data=doit_data(message,ID_CAP_CO_EXT);
		if(data!=NULL)
			set_upload_data(ID_CAP_CO_EXT,&(sensor_history.co[g_share_memory->cnt[SENSOR_CO]]),
			&(g_share_memory->cnt[SENSOR_CO]),data,date);
		
		data=doit_data(message,ID_CAP_CO2);
		if(data!=NULL)
			set_upload_data(ID_CAP_CO2,&(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]]),
			&(g_share_memory->cnt[SENSOR_CO2]),data,date);
		
		data=doit_data(message,ID_CAP_SHI_DU);
		if(data!=NULL)
			set_upload_data(ID_CAP_SHI_DU,&(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]]),
			&(g_share_memory->cnt[SENSOR_SHIDU]),data,date);
		
		data=doit_data(message,ID_CAP_HCHO_EXT);
		if(data!=NULL)
			set_upload_data(ID_CAP_HCHO_EXT,&(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]]),
			&(g_share_memory->cnt[SENSOR_HCHO]),data,date);
		
		data=doit_data(message,ID_CAP_TEMPERATURE);
		if(data!=NULL)
			set_upload_data(ID_CAP_TEMPERATURE,&(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]]),
			&(g_share_memory->cnt[SENSOR_TEMP]),data,date);
		
		data=doit_data(message,ID_CAP_PM_25);
		if(data!=NULL)
			set_upload_data(ID_CAP_PM_25,&(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]]),
			&(g_share_memory->cnt[SENSOR_PM25]),data,date);
		
		data=doit_data(message,ID_CAP_BUZZY);
		if(data!=NULL)
			set_upload_data(ID_CAP_BUZZY,&(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]]),
			&(g_share_memory->cnt[SENSOR_NOISE]),data,date);
		
		data=doit_data(message,ID_CAP_FENG_SU);
		if(data!=NULL)
			set_upload_data(ID_CAP_FENG_SU,&(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]]),
			&(g_share_memory->cnt[SENSOR_WIND]),data,date);
		
		data=doit_data(message,ID_CAP_CHOU_YANG_EXT);
		if(data!=NULL)
			set_upload_data(ID_CAP_CHOU_YANG_EXT,&(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]]),
			&(g_share_memory->cnt[SENSOR_O3]),data,date);
		
		data=doit_data(message,ID_CAP_QI_YA);
		if(data!=NULL)
			set_upload_data(ID_CAP_QI_YA,&(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]]),
			&(g_share_memory->cnt[SENSOR_PRESS]),data,date);
		
		data=doit_data(message,ID_CAP_TVOC_EXT);
		if(data!=NULL)
			set_upload_data(ID_CAP_TVOC_EXT,&(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]]),
			&(g_share_memory->cnt[SENSOR_TVOC]),data,date);
		
		data=doit_data(message,ID_CAP_PM_10);
		if(data!=NULL)
			set_upload_data(ID_CAP_PM_10,&(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]]),
			&(g_share_memory->cnt[SENSOR_PM10]),data,date);
	}
	send_web_post(URL,message,9,&rcv);
	if(rcv != NULL)
	{	
		free(rcv);
		rcv=NULL;
	}			
}
void clear_alarm(char *id,char *alarm_type)
{
	char *clear_msg=NULL;
	clear_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_CLEAR_WARNING);
	clear_msg=add_item(clear_msg,ID_DEVICE_UID,g_share_memory->uuid);
	clear_msg=add_item(clear_msg,ID_DEVICE_IP_ADDR,g_share_memory->ip);
	clear_msg=add_item(clear_msg,ID_DEVICE_PORT,(char *)"9517");						
	clear_msg=add_item(clear_msg,ID_ALARM_SENSOR,id);
	clear_msg=add_item(clear_msg,ID_ALARM_TYPE,alarm_type);
	send_server_save_local(NULL,clear_msg,0);
	free(clear_msg);
}
char *build_alarm_json(char *json,char *data)
{	
	char error[32]={0};
	sprintf(error,"%sth sensor possible error",data);
	json=rm_item(json,ID_ALARM_SENSOR);
	json=rm_item(json,ID_ALERT_CAP_FAILED);
	json=add_item(json,ID_ALARM_SENSOR,data);
	json=add_item(json,ID_ALERT_CAP_FAILED,error);
	return json;
}
char *update_alarm(char *json,char *data,char *alarm,char *sent)
{
	if(*alarm&&(!(*sent)))
	{
		json=build_alarm_json(json,data);
		printfLog(CAP_PROCESS"Upload alarm msg :\n%s\n",json);
		send_server_save_local(NULL,json,0);
		*sent=1;
	}
	return json;
}
char *get_pj(int id,float *buf,int len)
{	
	int i, j;
	float temp;
	char *rt=NULL;
	//printfLog("Enter get_pj %d %d\n",id,len);
	for (i = 0; i < len - 1; i++)
		for (j = 0; j < len - 1 - i; j++)
			if (buf[j] > buf[j + 1]) {
				temp = buf[j];
				buf[j] = buf[j + 1];
				buf[j + 1] = temp;
			}
	temp=0;
	if (len > 2) {
		for(i=1;i<len-1;i++)
			temp+=buf[i];
		temp=temp/(len-2);
	}
	else
		temp=buf[0];
	rt=(char *)malloc(10);
	memset(rt,'\0',10);
	switch(id)
	{
		case SENSOR_CO:			
			sprintf(rt,"%1.1f",temp);
			break;
		case SENSOR_CO2:			
			sprintf(rt,"%1.4f",temp);
			break;
		case SENSOR_HCHO:			
			sprintf(rt,"%1.2f",temp);
			break;	
		case SENSOR_SHIDU:			
			sprintf(rt,"%2.1f",temp);
			break;	
		case SENSOR_TEMP:			
			sprintf(rt,"%2.1f",temp);
			break;	
		case SENSOR_PM25:			
			sprintf(rt,"%d",(int)temp);
			break;	
		case SENSOR_WIND:			
			sprintf(rt,"%1.2f",temp);
			break;
		case SENSOR_PRESS:			
			sprintf(rt,"%d",(int)temp);
			break;	
		case SENSOR_NOISE:			
			sprintf(rt,"%3.1f",temp);
			break;	
		case SENSOR_TVOC:			
			sprintf(rt,"%d",(int)temp);
			break;	
		case SENSOR_O3:			
			sprintf(rt,"%d",(int)temp);
			break;
		case SENSOR_PM10:			
			sprintf(rt,"%1.3f",temp);
			break;
		default:
			break;
	}
	//printfLog("Leave pj %s\n",rt);
	return rt;
}
char *count_pj(char *message)
{
	char *temp;
	//printfLog("Enter count_pj\n");
	if(g_share_memory->pj_cnt[SENSOR_CO]!=0)
	{
		temp=get_pj(SENSOR_CO,g_share_memory->pj[SENSOR_CO],g_share_memory->pj_cnt[SENSOR_CO]);
		message=add_item(message,ID_CAP_CO_EXT,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_CO2]!=0)
	{
		temp=get_pj(SENSOR_CO2,g_share_memory->pj[SENSOR_CO2],g_share_memory->pj_cnt[SENSOR_CO2]);
		message=add_item(message,ID_CAP_CO2,temp);
		free(temp);
	}	
	if(g_share_memory->pj_cnt[SENSOR_HCHO]!=0)
	{
		temp=get_pj(SENSOR_HCHO,g_share_memory->pj[SENSOR_HCHO],g_share_memory->pj_cnt[SENSOR_HCHO]);
		message=add_item(message,ID_CAP_HCHO_EXT,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_SHIDU]!=0)
	{
		temp=get_pj(SENSOR_SHIDU,g_share_memory->pj[SENSOR_SHIDU],g_share_memory->pj_cnt[SENSOR_SHIDU]);
		message=add_item(message,ID_CAP_SHI_DU,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_TEMP]!=0)
	{
		temp=get_pj(SENSOR_TEMP,g_share_memory->pj[SENSOR_TEMP],g_share_memory->pj_cnt[SENSOR_TEMP]);
		message=add_item(message,ID_CAP_TEMPERATURE,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_PM25]!=0)
	{
		temp=get_pj(SENSOR_PM25,g_share_memory->pj[SENSOR_PM25],g_share_memory->pj_cnt[SENSOR_PM25]);
		message=add_item(message,ID_CAP_PM_25,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_WIND]!=0)
	{
		temp=get_pj(SENSOR_WIND,g_share_memory->pj[SENSOR_WIND],g_share_memory->pj_cnt[SENSOR_WIND]);
		message=add_item(message,ID_CAP_FENG_SU,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_PRESS]!=0)
	{
		temp=get_pj(SENSOR_PRESS,g_share_memory->pj[SENSOR_PRESS],g_share_memory->pj_cnt[SENSOR_PRESS]);
		message=add_item(message,ID_CAP_QI_YA,temp);
		free(temp);
	}	
	if(g_share_memory->pj_cnt[SENSOR_NOISE]!=0)
	{
		temp=get_pj(SENSOR_NOISE,g_share_memory->pj[SENSOR_NOISE],g_share_memory->pj_cnt[SENSOR_NOISE]);
		message=add_item(message,ID_CAP_BUZZY,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_TVOC]!=0)
	{
		temp=get_pj(SENSOR_TVOC,g_share_memory->pj[SENSOR_TVOC],g_share_memory->pj_cnt[SENSOR_TVOC]);
		message=add_item(message,ID_CAP_TVOC,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_O3]!=0)
	{
		temp=get_pj(SENSOR_O3,g_share_memory->pj[SENSOR_O3],g_share_memory->pj_cnt[SENSOR_O3]);
		message=add_item(message,ID_CAP_CHOU_YANG,temp);
		free(temp);
	}
	if(g_share_memory->pj_cnt[SENSOR_PM10]!=0)
	{
		temp=get_pj(SENSOR_PM10,g_share_memory->pj[SENSOR_PM10],g_share_memory->pj_cnt[SENSOR_PM10]);
		message=add_item(message,ID_CAP_PM_10,temp);
		free(temp);
	}
	g_share_memory->pj_cnt[SENSOR_CO]=0;
	g_share_memory->pj_cnt[SENSOR_CO2]=0;
	g_share_memory->pj_cnt[SENSOR_HCHO]=0;
	g_share_memory->pj_cnt[SENSOR_SHIDU]=0;
	g_share_memory->pj_cnt[SENSOR_TEMP]=0;
	g_share_memory->pj_cnt[SENSOR_PM25]=0;
	g_share_memory->pj_cnt[SENSOR_WIND]=0;
	g_share_memory->pj_cnt[SENSOR_PRESS]=0;
	g_share_memory->pj_cnt[SENSOR_NOISE]=0;
	g_share_memory->pj_cnt[SENSOR_TVOC]=0;
	g_share_memory->pj_cnt[SENSOR_O3]=0;
	g_share_memory->pj_cnt[SENSOR_PM10]=0;
	//printfLog("Leave count_pj\n");
	return message;
}
void save_pj(char cmd,float value)
{
	//printfLog("Enter save_pj\n");
	if(cmd==atoi(ID_CAP_CO_EXT))
	{
		g_share_memory->pj[SENSOR_CO][g_share_memory->pj_cnt[SENSOR_CO]++]=value;
	}
	else if(cmd==atoi(ID_CAP_CO2))
	{
		g_share_memory->pj[SENSOR_CO2][g_share_memory->pj_cnt[SENSOR_CO2]++]=value;
	}
	else if(cmd==atoi(ID_CAP_HCHO_EXT))
	{
		g_share_memory->pj[SENSOR_HCHO][g_share_memory->pj_cnt[SENSOR_HCHO]++]=value;
	}
	else if(cmd==atoi(ID_CAP_SHI_DU))
	{
		g_share_memory->pj[SENSOR_SHIDU][g_share_memory->pj_cnt[SENSOR_SHIDU]++]=value;
	}
	else if(cmd==atoi(ID_CAP_TEMPERATURE))
	{
		g_share_memory->pj[SENSOR_TEMP][g_share_memory->pj_cnt[SENSOR_TEMP]++]=value;
	}
	else if(cmd==atoi(ID_CAP_PM_25))
	{
		g_share_memory->pj[SENSOR_PM25][g_share_memory->pj_cnt[SENSOR_PM25]++]=value;
	}
	else if(cmd==atoi(ID_CAP_FENG_SU))
	{
		g_share_memory->pj[SENSOR_WIND][g_share_memory->pj_cnt[SENSOR_WIND]++]=value;
	}
	else if(cmd==atoi(ID_CAP_QI_YA))
	{
		g_share_memory->pj[SENSOR_PRESS][g_share_memory->pj_cnt[SENSOR_PRESS]++]=value;
	}
	else if(cmd==atoi(ID_CAP_BUZZY))
	{
		g_share_memory->pj[SENSOR_NOISE][g_share_memory->pj_cnt[SENSOR_NOISE]++]=value;
	}
	else if(cmd==atoi(ID_CAP_TVOC))
	{
		g_share_memory->pj[SENSOR_TVOC][g_share_memory->pj_cnt[SENSOR_TVOC]++]=value;
	}
	else if(cmd==atoi(ID_CAP_CHOU_YANG))
	{
		g_share_memory->pj[SENSOR_O3][g_share_memory->pj_cnt[SENSOR_O3]++]=value;
	}
	else if(cmd==atoi(ID_CAP_PM_10))
	{
		g_share_memory->pj[SENSOR_PM10][g_share_memory->pj_cnt[SENSOR_PM10]++]=value;
	}
	//printfLog("Leave save_pj\n");

	}
char *count_sensor_value(char cmd,char *json,float value)
{
	char *times=NULL;
	char *sent;
	int min,max;
	char *alarm;
	char id[256]={0};
	save_pj(cmd,value);
	if(cmd==atoi(ID_CAP_CO2))
	{
		times=&(g_share_memory->times[SENSOR_CO2]);
		sent=&(g_share_memory->sent[SENSOR_CO2]);
		min=MIN_CO2;
		max=MAX_CO2;
		value=value*10000;
		alarm=&(g_share_memory->alarm[SENSOR_CO2]);
		strcpy(id,ID_CAP_CO2);
	}
	else if(cmd==atoi(ID_CAP_CO))
	{
		times=&(g_share_memory->times[SENSOR_CO]);
		sent=&(g_share_memory->sent[SENSOR_CO]);
		min=MIN_CO;
		max=MAX_CO;
		alarm=&(g_share_memory->alarm[SENSOR_CO]);
		strcpy(id,ID_CAP_CO);
		//printfLog(CAP_PROCESS"CO is %f,min %d,max %d,times %d\n",value,min,max,*times);
	}
	else if(cmd==atoi(ID_CAP_HCHO))
	{
		times=&(g_share_memory->times[SENSOR_HCHO]);
		sent=&(g_share_memory->sent[SENSOR_HCHO]);
		min=MIN_HCHO;
		max=MAX_HCHO;
		alarm=&(g_share_memory->alarm[SENSOR_HCHO]);
		strcpy(id,ID_CAP_HCHO);
	}
	else if(cmd==atoi(ID_CAP_TEMPERATURE))
	{
		times=&(g_share_memory->times[SENSOR_TEMP]);
		sent=&(g_share_memory->sent[SENSOR_TEMP]);
		min=MIN_TEMP;
		max=MAX_TEMP;
		alarm=&(g_share_memory->alarm[SENSOR_TEMP]);
		strcpy(id,ID_CAP_TEMPERATURE);
	}
	else if(cmd==atoi(ID_CAP_SHI_DU))
	{
		times=&(g_share_memory->times[SENSOR_SHIDU]);
		sent=&(g_share_memory->sent[SENSOR_SHIDU]);
		min=MIN_SHIDU;
		max=MAX_SHIDU;
		alarm=&(g_share_memory->alarm[SENSOR_SHIDU]);
		strcpy(id,ID_CAP_SHI_DU);
	}
	else if(cmd==atoi(ID_CAP_PM_25))
	{
		times=&(g_share_memory->times[SENSOR_PM25]);
		sent=&(g_share_memory->sent[SENSOR_PM25]);
		min=MIN_PM25;
		max=MAX_PM25;
		alarm=&(g_share_memory->alarm[SENSOR_PM25]);
		strcpy(id,ID_CAP_PM_25);
	}
	else if(cmd==atoi(ID_CAP_BUZZY))
	{
		times=&(g_share_memory->times[SENSOR_NOISE]);
		sent=&(g_share_memory->sent[SENSOR_NOISE]);
		min=MIN_NOISE;
		max=MAX_NOISE;
		alarm=&(g_share_memory->alarm[SENSOR_NOISE]);
		strcpy(id,ID_CAP_BUZZY);
	}
	else if(cmd==atoi(ID_CAP_QI_YA))
	{
		times=&(g_share_memory->times[SENSOR_PRESS]);
		sent=&(g_share_memory->sent[SENSOR_PRESS]);
		min=MIN_PRESS;
		max=MAX_PRESS;
		alarm=&(g_share_memory->alarm[SENSOR_PRESS]);
		strcpy(id,ID_CAP_QI_YA);
	}
	else if(cmd==atoi(ID_CAP_TVOC))
	{
		times=&(g_share_memory->times[SENSOR_TVOC]);
		sent=&(g_share_memory->sent[SENSOR_TVOC]);
		min=MIN_TVOC;
		max=MAX_TVOC;
		alarm=&(g_share_memory->alarm[SENSOR_TVOC]);
		strcpy(id,ID_CAP_TVOC);
	}
	else if(cmd==atoi(ID_CAP_FENG_SU))
	{
		times=&(g_share_memory->times[SENSOR_WIND]);
		sent=&(g_share_memory->sent[SENSOR_WIND]);
		min=MIN_WIND;
		max=MAX_WIND;
		alarm=&(g_share_memory->alarm[SENSOR_WIND]);
		strcpy(id,ID_CAP_FENG_SU);
	}
	else if(cmd==atoi(ID_CAP_CHOU_YANG))
	{
		times=&(g_share_memory->times[SENSOR_O3]);
		sent=&(g_share_memory->sent[SENSOR_O3]);
		min=MIN_O3;
		max=MAX_O3;
		alarm=&(g_share_memory->alarm[SENSOR_SHIDU]);
		strcpy(id,ID_CAP_CHOU_YANG);
	}
	else if(cmd==atoi(ID_CAP_PM_10))
	{
		times=&(g_share_memory->times[SENSOR_PM10]);
		sent=&(g_share_memory->sent[SENSOR_PM10]);
		min=MIN_PM10;
		max=MAX_PM10;
		alarm=&(g_share_memory->alarm[SENSOR_PM10]);
		strcpy(id,ID_CAP_PM_10);
	}
	if(times!=NULL)
	{
		//printfLog(CAP_PROCESS"count_sensor_value cmd %d,value %d,min %d,max %d,times %d,sent %d,alarm %d,json %s\n",
		//	cmd,value,min,max,*times,*sent,*alarm,json);
		if((int)value<min)
			(*times)++;
		else if((int)value>max)
			(*times)++;
		else
		{
			*times=0;
			if(*alarm&ALARM_BELOW)
				clear_alarm(id,ID_ALERT_BELOW);
			if(*alarm&ALARM_UP)
				clear_alarm(id,ID_ALERT_UP);
			if(*alarm&ALARM_BELOW||*alarm&ALARM_UP)
			{
				*alarm=ALARM_NONE;
				save_sensor_alarm_info();	
			}	
		}
		if(*times==MAX_COUNT_TIMES && !(*alarm))
		{	
			//need send server alarm
			json=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
			json=add_item(json,ID_DEVICE_UID,g_share_memory->uuid);
			json=add_item(json,ID_DEVICE_IP_ADDR,g_share_memory->ip);
			json=add_item(json,ID_DEVICE_PORT,(char *)"9517");
			if((int)value<min)
			{
				*alarm|=ALARM_BELOW;
				json=add_item(json,ID_ALARM_TYPE,ID_ALERT_BELOW);
			}
			else
			{
				*alarm|=ALARM_UP;
				json=add_item(json,ID_ALARM_TYPE,ID_ALERT_UP);
			}
			save_sensor_alarm_info();	
			*sent=0;
		}
		if(*times==MAX_COUNT_TIMES && cmd==atoi(ID_CAP_CO))
		{
			co_flash_alarm();
		}
	}
	//printfLog(CAP_PROCESS"count_sensor_value <== %s\n",json);
	return json;
}
void update_dwin_real_value(char *id,int value, char *data)
{	
	if(strncmp(id,ID_CAP_CO_EXT,strlen(ID_CAP_CO_EXT))==0 &&
			g_share_memory->sensor_off[SENSOR_CO]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_CO]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_CO]=1;
			write_data(ADDR_CO_SHOW_PIC,0x0000);
			write_data(ADDR_CO_SHOW_PIC_PPM,0x0000);
		}
		//printfLog(CAP_PROCESS"show co real %d\n", value);
		write_data(ADDR_CO_REAL_1,value);
		show_main_alarm_co(data);
		
	}
	else if(strncmp(id,ID_CAP_CO2_EXT,strlen(ID_CAP_CO2_EXT))==0&&
			      g_share_memory->sensor_off[SENSOR_CO2]==0)
	{			
		if(g_share_memory->sensor_has_data[SENSOR_CO2]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_CO2]=1;
			write_data(ADDR_CO2_SHOW_PIC,0x0000);
			write_data(ADDR_CO2_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_CO2_REAL_1,value);
		show_main_alarm_co2(data);
	}
	else if(strncmp(id,ID_CAP_HCHO_EXT,strlen(ID_CAP_HCHO_EXT))==0&&
			                  g_share_memory->sensor_off[SENSOR_HCHO]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_HCHO]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_HCHO]=1;
			write_data(ADDR_HCHO_SHOW_PIC,0x0000);
			write_data(ADDR_HCHO_SHOW_PIC_PPM,0x0000);
			//printfLog(CAP_PROCESS"open hcho display\n");
		}
		//printfLog(CAP_PROCESS"HCHO for xiao %d\n",value);
		write_data(ADDR_HCHO_REAL_1,value);
		show_main_alarm_hcho(data);
	}
	else if(strncmp(id,ID_CAP_TVOC_EXT,strlen(ID_CAP_TVOC_EXT))==0&&
			                  g_share_memory->sensor_off[SENSOR_TVOC]==0)
	{						
		if(g_share_memory->sensor_has_data[SENSOR_TVOC]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_TVOC]=1;
			write_data(ADDR_TVOC_SHOW_PIC,0x0000);
			write_data(ADDR_TVOC_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_TVOC_REAL_1,value);
		show_main_alarm_tvoc(data);
	}
	else if(strncmp(id,ID_CAP_CHOU_YANG_EXT,strlen(ID_CAP_CHOU_YANG_EXT))==0&&
			                  g_share_memory->sensor_off[SENSOR_O3]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_O3]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_O3]=1;
			write_data(ADDR_O3_SHOW_PIC,0x0000);
			write_data(ADDR_O3_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_O3_REAL_1,value);
		show_main_alarm_o3(data);
	}
	else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(ID_CAP_TEMPERATURE))==0&&
			                  g_share_memory->sensor_off[SENSOR_TEMP]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_TEMP]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_TEMP]=1;
			write_data(ADDR_TEMP_SHOW_PIC,0x0000);
			write_data(ADDR_TEMP_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_TEMP_REAL_1,value);
		write_data(ADDR_TEMP_REAL_2,value);
		show_main_alarm_temp(data);		
	}
	else if(strncmp(id,ID_CAP_SHI_DU,strlen(ID_CAP_SHI_DU))==0&&
			                  g_share_memory->sensor_off[SENSOR_SHIDU]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_SHIDU]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_SHIDU]=1;
			write_data(ADDR_SHIDU_SHOW_PIC,0x0000);
			write_data(ADDR_SHIDU_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_SHIDU_REAL_1,value);
		write_data(ADDR_SHIDU_REAL_2,value);
		show_main_alarm_shidu(data);		
	}
	else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0&&
			                  g_share_memory->sensor_off[SENSOR_PM25]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_PM25]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_PM25]=1;
			write_data(ADDR_PM25_SHOW_PIC,0x0000);
			write_data(ADDR_PM25_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_PM25_REAL_1,value);
		write_data(ADDR_PM25_REAL_2,value);
		show_main_alarm_pm25(data);		
	}
	else if(strncmp(id,ID_CAP_PM_10,strlen(ID_CAP_PM_10))==0&&
			                  g_share_memory->sensor_off[SENSOR_PM10]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_PM10]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_PM10]=1;
			write_data(ADDR_PM10_SHOW_PIC,0x0000);
			write_data(ADDR_PM10_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_PM10_REAL_1,value);
		write_data(ADDR_PM10_REAL_2,value);
		show_main_alarm_pm10(data);
	}
	else if(strncmp(id,ID_CAP_FENG_SU,strlen(ID_CAP_FENG_SU))==0&&
			                  g_share_memory->sensor_off[SENSOR_WIND]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_WIND]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_WIND]=1;
			write_data(ADDR_WIND_SHOW_PIC,0x0000);
			write_data(ADDR_WIND_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_WIND_REAL_1,value);
		write_data(ADDR_WIND_REAL_2,value);
		write_data(ADDR_CURR_FENGSU,value);		
	}
	else if(strncmp(id,ID_CAP_QI_YA,strlen(ID_CAP_QI_YA))==0&&
			                  g_share_memory->sensor_off[SENSOR_PRESS]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_PRESS]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_PRESS]=1;
			write_data(ADDR_PRESS_SHOW_PIC,0x0000);
			write_data(ADDR_PRESS_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_PRESS_REAL_1,value);
		write_data(ADDR_PRESS_REAL_2,value);
	}
	else if(strncmp(id,ID_CAP_BUZZY,strlen(ID_CAP_BUZZY))==0&&
			                  g_share_memory->sensor_off[SENSOR_NOISE]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_NOISE]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_NOISE]=1;
			write_data(ADDR_NOISE_SHOW_PIC,0x0000);
			write_data(ADDR_NOISE_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_NOISE_REAL_1,value);
		write_data(ADDR_NOISE_REAL_2,value);
		show_main_alarm_noise(data);
	}
	else if(strncmp(id,ID_CAP_CO,strlen(ID_CAP_CO))==0&&
			                  g_share_memory->sensor_off[SENSOR_CO]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_CO]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_CO]=1;
			write_data(ADDR_CO_SHOW_PIC,0x0000);
			write_data(ADDR_CO_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_CO_REAL_2,value);
		show_main_alarm_co(data);
	}
	else if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0&&
			                  g_share_memory->sensor_off[SENSOR_CO2]==0)
	{						
		if(g_share_memory->sensor_has_data[SENSOR_CO2]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_CO2]=1;
			write_data(ADDR_CO2_SHOW_PIC,0x0000);
			write_data(ADDR_CO2_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_CO2_REAL_2,value);
		show_main_alarm_co2(data);
	}
	else if(strncmp(id,ID_CAP_HCHO,strlen(ID_CAP_HCHO))==0&&
			                  g_share_memory->sensor_off[SENSOR_HCHO]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_HCHO]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_HCHO]=1;
			write_data(ADDR_HCHO_SHOW_PIC,0x0000);
			write_data(ADDR_HCHO_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_HCHO_REAL_2,value);
		show_main_alarm_hcho(data);
	}
	else if(strncmp(id,ID_CAP_TVOC,strlen(ID_CAP_TVOC))==0&&
			                  g_share_memory->sensor_off[SENSOR_TVOC]==0)
	{				
		if(g_share_memory->sensor_has_data[SENSOR_TVOC]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_TVOC]=1;
			write_data(ADDR_TVOC_SHOW_PIC,0x0000);
			write_data(ADDR_TVOC_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_TVOC_REAL_2,value);
		show_main_alarm_tvoc(data);
	}
	else if(strncmp(id,ID_CAP_CHOU_YANG,strlen(ID_CAP_CHOU_YANG))==0&&
			                  g_share_memory->sensor_off[SENSOR_O3]==0)
	{
		if(g_share_memory->sensor_has_data[SENSOR_O3]==0)
		{
			g_share_memory->sensor_has_data[SENSOR_O3]=1;
			write_data(ADDR_O3_SHOW_PIC,0x0000);
			write_data(ADDR_O3_SHOW_PIC_PPM,0x0000);
		}
		write_data(ADDR_O3_REAL_2,value);
		show_main_alarm_o3(data);
	}
}
/*
  *build json from cmd
  *update cap data to lcd
  *create alarm and normal message to server
*/
char *build_message(char *cmd,int len,char *message)
{	
	int i=0;
	char sensor_error[]={0x65,0x72,0x72,0x6f,0x72};
	char id[32]={0},data[32]={0},date[32]={0},error[32]={0};
//	unsigned int crc=(cmd[len-2]<<8)|cmd[len-1];
	int message_type=(cmd[2]<<8)|cmd[3];
	sprintf(id,"%d",message_type);
	//printfLog(CAP_PROCESS"crc 0x%04X,message_type %d,len %d \n",crc,message_type,len);
	//if(crc==CRC_check((unsigned char *)cmd,len-2))
	//{
		i=0;
		switch(message_type)
		{
			case TIME_BYTE:
			{	//TIME_BYTE got ,we can send to server now
				sprintf(date,"20%02d-%02d-%02d %02d:%02d",cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
				printfLog(CAP_PROCESS"date is %s\r\n",date);
				g_share_memory->current_time[0]=cmd[5];
				g_share_memory->current_time[1]=cmd[6];
				g_share_memory->current_time[2]=cmd[7];
				g_share_memory->current_time[3]=cmd[8];
				g_share_memory->current_time[4]=cmd[9];
				message=add_item(message,ID_DEVICE_CAP_TIME,date);
				if(warnning_msg!=NULL)
				{	//have alarm msg upload
					warnning_msg=update_alarm(warnning_msg,ID_CAP_CO2,
									&(g_share_memory->alarm[SENSOR_CO2]),&(g_share_memory->sent[SENSOR_CO2]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_CO,
									&(g_share_memory->alarm[SENSOR_CO]),&(g_share_memory->sent[SENSOR_CO]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_HCHO,
									&(g_share_memory->alarm[SENSOR_HCHO]),&(g_share_memory->sent[SENSOR_HCHO]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_SHI_DU,
									&(g_share_memory->alarm[SENSOR_SHIDU]),&(g_share_memory->sent[SENSOR_SHIDU]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_TEMPERATURE,
									&(g_share_memory->alarm[SENSOR_TEMP]),&(g_share_memory->sent[SENSOR_TEMP]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_PM_25,
									&(g_share_memory->alarm[SENSOR_PM25]),&(g_share_memory->sent[SENSOR_PM25]));					
					warnning_msg=update_alarm(warnning_msg,ID_CAP_FENG_SU,
									&(g_share_memory->alarm[SENSOR_WIND]),&(g_share_memory->sent[SENSOR_WIND]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_TVOC,
									&(g_share_memory->alarm[SENSOR_TVOC]),&(g_share_memory->sent[SENSOR_TVOC]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_CHOU_YANG,
									&(g_share_memory->alarm[SENSOR_O3]),&(g_share_memory->sent[SENSOR_O3]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_QI_YA,
									&(g_share_memory->alarm[SENSOR_PRESS]),&(g_share_memory->sent[SENSOR_PRESS]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_BUZZY,
									&(g_share_memory->alarm[SENSOR_NOISE]),&(g_share_memory->sent[SENSOR_NOISE]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_PM_10,
									&(g_share_memory->alarm[SENSOR_PM10]),&(g_share_memory->sent[SENSOR_PM10]));
					if(warnning_msg!=NULL)
						free(warnning_msg);
					warnning_msg=NULL;
				}
				if(g_upload)
				{
					g_upload=0;
					//printfLog(CAP_PROCESS"Upload data msg :\n");
					message=count_pj(message);
					send_server_save_local(date,message,1);
					//printfLog(CAP_PROCESS"Upload data msg 1:\n");
					show_main_his();
					//printfLog(CAP_PROCESS"Upload data msg 2:\n");
					show_main_alarm();	
					//printfLog(CAP_PROCESS"Upload data msg 3:\n");
				}
				free(message);
				message=NULL;
				memset(date,'\0',32);
				//printfLog(CAP_PROCESS"Upload data msg 4:\n");
			}
			break;
			case VERIFY_BYTE:
			{
				//write(fd,server_time,13);
			}
			break;
			case RESEND_BYTE:
			{
				write(g_share_memory->fd_com,g_share_memory->server_time,13);
			}
			break;
			default:
			{
				/*get cap data*/
				if(memcmp(cmd+5,sensor_error,5)==0)
				{	
					//error got from cap board,check uninsert msg
					if(cmd[3]==atoi(ID_CAP_CO2) && !(g_share_memory->alarm[SENSOR_CO2] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_CO2]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_CO2]=0;						
						g_share_memory->sensor_has_data[SENSOR_CO2]=0;
						write_data(ADDR_CO2_SHOW_PIC,0x01);
						write_data(ADDR_CO2_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_CO) && !(g_share_memory->alarm[SENSOR_CO] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_CO]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_CO]=0;
						g_share_memory->sensor_has_data[SENSOR_CO]=0;
						write_data(ADDR_CO_SHOW_PIC,0x01);
						write_data(ADDR_CO_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_HCHO) && !(g_share_memory->alarm[SENSOR_HCHO] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_HCHO]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_HCHO]=0;
						g_share_memory->sensor_has_data[SENSOR_HCHO]=0;
						write_data(ADDR_HCHO_SHOW_PIC,0x01);
						write_data(ADDR_HCHO_SHOW_PIC_PPM,0x01);						
					}
					else if(cmd[3]==atoi(ID_CAP_SHI_DU) && !(g_share_memory->alarm[SENSOR_SHIDU] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_SHIDU]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_SHIDU]=0;
						g_share_memory->sensor_has_data[SENSOR_SHIDU]=0;
						write_data(ADDR_SHIDU_SHOW_PIC,0x01);
						write_data(ADDR_SHIDU_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& !(g_share_memory->alarm[SENSOR_TEMP] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_TEMP]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_TEMP]=0;
						g_share_memory->sensor_has_data[SENSOR_TEMP]=0;
						write_data(ADDR_TEMP_SHOW_PIC,0x01);
						write_data(ADDR_TEMP_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_PM_25)&& !(g_share_memory->alarm[SENSOR_PM25] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_PM25]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_PM25]=0;
						g_share_memory->sensor_has_data[SENSOR_PM25]=0;
						write_data(ADDR_PM25_SHOW_PIC,0x01);
						write_data(ADDR_PM25_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_TVOC) && !(g_share_memory->alarm[SENSOR_TVOC] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_TVOC]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_TVOC]=0;
						g_share_memory->sensor_has_data[SENSOR_TVOC]=0;
						write_data(ADDR_TVOC_SHOW_PIC,0x01);
						write_data(ADDR_TVOC_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_QI_YA) && !(g_share_memory->alarm[SENSOR_PRESS] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_PRESS]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_PRESS]=0;						
						g_share_memory->sensor_has_data[SENSOR_PRESS]=0;
						write_data(ADDR_PRESS_SHOW_PIC,0x01);
						write_data(ADDR_PRESS_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_FENG_SU) && !(g_share_memory->alarm[SENSOR_WIND] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_WIND]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_WIND]=0;
						g_share_memory->sensor_has_data[SENSOR_WIND]=0;
						write_data(ADDR_WIND_SHOW_PIC,0x01);
						write_data(ADDR_WIND_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_CHOU_YANG) && !(g_share_memory->alarm[SENSOR_O3] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_O3]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_O3]=0;
						g_share_memory->sensor_has_data[SENSOR_O3]=0;
						write_data(ADDR_O3_SHOW_PIC,0x01);
						write_data(ADDR_O3_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_BUZZY)&& !(g_share_memory->alarm[SENSOR_NOISE] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_NOISE]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_NOISE]=0;
						g_share_memory->sensor_has_data[SENSOR_NOISE]=0;
						write_data(ADDR_NOISE_SHOW_PIC,0x01);
						write_data(ADDR_NOISE_SHOW_PIC_PPM,0x01);
					}
					else if(cmd[3]==atoi(ID_CAP_PM_10)&& !(g_share_memory->alarm[SENSOR_PM10] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_PM10]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_PM10]=0;
						g_share_memory->sensor_has_data[SENSOR_PM10]=0;
						write_data(ADDR_PM10_SHOW_PIC,0x01);
						write_data(ADDR_PM10_SHOW_PIC_PPM,0x01);
					}
					
					else 
						return message;
					//inform server
					warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_share_memory->uuid);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,g_share_memory->ip);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,(char *)"9517");
					warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UNINSERT);
					memset(error,'\0',32);
					save_sensor_alarm_info();					
					return message;
				}
				else
				{	//normal data or beyond Min & Max data
					float value=0.0;
					//clear the uninsert alarm
					if(cmd[3]==atoi(ID_CAP_CO2) && (g_share_memory->alarm[SENSOR_CO2]&ALARM_UNINSERT))
					{					
						clear_alarm(ID_CAP_CO2,ID_ALERT_UNINSERT);						
						g_share_memory->alarm[SENSOR_CO2]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_CO) && (g_share_memory->alarm[SENSOR_CO]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_CO,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_CO]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_HCHO) && (g_share_memory->alarm[SENSOR_HCHO]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_HCHO,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_HCHO]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_SHI_DU) && (g_share_memory->alarm[SENSOR_SHIDU]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_SHI_DU,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_SHIDU]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& (g_share_memory->alarm[SENSOR_TEMP]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_TEMP]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_PM_25)&& (g_share_memory->alarm[SENSOR_PM25]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_PM_25,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_PM25]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_FENG_SU) && (g_share_memory->alarm[SENSOR_WIND]&ALARM_UNINSERT))
					{					
						clear_alarm(ID_CAP_FENG_SU,ID_ALERT_UNINSERT);						
						g_share_memory->alarm[SENSOR_WIND]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_CHOU_YANG) && (g_share_memory->alarm[SENSOR_O3]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_CHOU_YANG,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_O3]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_BUZZY) && (g_share_memory->alarm[SENSOR_NOISE]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_BUZZY,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_NOISE]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_QI_YA) && (g_share_memory->alarm[SENSOR_PRESS]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_QI_YA,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_PRESS]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_TVOC)&& (g_share_memory->alarm[SENSOR_TVOC]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_TVOC,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_TVOC]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_PM_10)&& (g_share_memory->alarm[SENSOR_PM10]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_PM_10,ID_ALERT_UNINSERT);
						g_share_memory->alarm[SENSOR_PM10]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(message==NULL)
					{
						message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
						message=add_item(message,ID_DEVICE_UID,g_share_memory->uuid);
						message=add_item(message,ID_DEVICE_IP_ADDR,g_share_memory->ip);
						message=add_item(message,ID_DEVICE_PORT,(char *)"9517");	
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
						int temp=1;
						for(i=0;i<cmd[7];i++)
							temp*=10;
						value=((float)((cmd[5]<<8|cmd[6])))/(float)temp;
					}	
					else
					{
							value=(float)(cmd[5]<<8|cmd[6]);
					}
					//printfLog(CAP_PROCESS"1 Value %f\n",value);
					warnning_msg=count_sensor_value(cmd[3],warnning_msg,value);
					//printfLog(CAP_PROCESS"0 id %s data %s\r\n",id,data);
					//real time update cap data
					update_dwin_real_value(id,cmd[5]<<8|cmd[6],data);
					//printfLog(CAP_PROCESS"1 id %s data %s\r\n",id,data);
					//this used to get non-pj value
					if( message_type!=atoi(ID_CAP_CO_EXT) &&message_type!=atoi(ID_CAP_CO2) &&
						message_type!=atoi(ID_CAP_HCHO_EXT)&&message_type!=atoi(ID_CAP_SHI_DU) &&
						message_type!=atoi(ID_CAP_TEMPERATURE) &&message_type!=atoi(ID_CAP_PM_25) &&
						message_type!=atoi(ID_CAP_FENG_SU) &&message_type!=atoi(ID_CAP_QI_YA) &&
						message_type!=atoi(ID_CAP_BUZZY) &&message_type!=atoi(ID_CAP_TVOC) &&
						message_type!=atoi(ID_CAP_CHOU_YANG) &&message_type!=atoi(ID_CAP_PM_10)) {
						//printfLog(CAP_PROCESS"uuuuuu\n");
						message=add_item(message,id,data);
						}
					//printfLog(CAP_PROCESS"2 id %s data %s\r\n==>\n%s\n",id,data,message);
					return message;
				}
			}
			break;
		}
	//}
	//else
	//{
	//	printfLog(CAP_PROCESS"CRC error 0x%04X\r\n",CRC_check((unsigned char *)cmd,len-2));
	//	for(i=0;i<len;i++)
	//		printfLog("0x%02x ",cmd[i]);
	//	printfLog("\n");
	//}
	return message;
}
void return_zero_point(int co)
{
	int crc = 0;
	int i =0;
	char cmd_return_point[]=	{0x6c,ARM_TO_CAP,0x00,0x05,0x04,0x00,0x01,0x00,0x00,0x00,0x00};
	if(co == 1)
	{
		for(i=0;i<11;i++)
			if(g_share_memory->sensor_interface_mem[i] == TYPE_SENSOR_CO_WEISHEN ||
			g_share_memory->sensor_interface_mem[i] == TYPE_SENSOR_CO_DD)
			break;
		printfLog(CAP_PROCESS"CO interface %d %4x\n",i,g_share_memory->sensor_interface_mem[i]);
		printfLog(CAP_PROCESS"CO zero value %d\n",g_share_memory->cur_co);
		cmd_return_point[7]=(g_share_memory->cur_co>>8) & 0xff;
		cmd_return_point[8]=(g_share_memory->cur_co & 0xff);
	}
	else if(co == 0)
	{
		for(i=0;i<11;i++)
			if(g_share_memory->sensor_interface_mem[i] == TYPE_SENSOR_CH2O_WEISHEN ||
				g_share_memory->sensor_interface_mem[i] == TYPE_SENSOR_CH2O_AERSHEN)
				break;
		printfLog(CAP_PROCESS"CH2O interface %d %4x\n",i,g_share_memory->sensor_interface_mem[i]);
		printfLog(CAP_PROCESS"CH2O zero value %d\n",g_share_memory->cur_ch2o);
		cmd_return_point[7]=(g_share_memory->cur_ch2o>>8) & 0xff;
		cmd_return_point[8]=(g_share_memory->cur_ch2o & 0xff);
	}
	else if(co == 2)
	{
		for(i=0;i<11;i++)
			if(g_share_memory->sensor_interface_mem[i] == TYPE_SENSOR_TVOC_1)
				break;
		printfLog(CAP_PROCESS"TVOC interface %d %4x\n",i,g_share_memory->sensor_interface_mem[i]);
		printfLog(CAP_PROCESS"TVOC zero value %d\n",g_share_memory->cur_tvoc);
		cmd_return_point[7]=(g_share_memory->cur_tvoc>>8) & 0xff;
		cmd_return_point[8]=(g_share_memory->cur_tvoc & 0xff);
	}
	cmd_return_point[5]=i+1;
	crc=CRC_check((unsigned char *)cmd_return_point,9);
	cmd_return_point[9]=(crc&0xff00)>>8;cmd_return_point[10]=crc&0x00ff;
	for(i=0;i<sizeof(cmd_return_point);i++)
		printfLog("%02X ",cmd_return_point[i]);
	printfLog("\n");
	send_cmd_to_cap(cmd_return_point,sizeof(cmd_return_point));
}
void show_cap_value(char *buf,int len)
{
	int i=0;	
	switch(buf[0]<<8|buf[1])
	{
		case 60:
			printfLog(CAP_PROCESS"[CAP][CO] ");
			break;
		case 61:
			printfLog(CAP_PROCESS"[CAP][CO2] ");
			break;
		case 62:
			printfLog(CAP_PROCESS"[CAP][HCHO] ");
			break;
		case 63:
			printfLog(CAP_PROCESS"[CAP][TEMP] ");
			break;
		case 64:
			printfLog(CAP_PROCESS"[CAP][SHIDU] ");
			break;
		case 65:
			printfLog(CAP_PROCESS"[CAP][PM2.5] ");
			break;
		case 66:
			printfLog(CAP_PROCESS"[CAP][PM10] ");
			break;
		case 67:
			printfLog(CAP_PROCESS"[CAP][NOISY] ");
			break;
		case 68:
			printfLog(CAP_PROCESS"[CAP][WIND] ");
			break;
		case 69:
			printfLog(CAP_PROCESS"[CAP][PRESS] ");
			break;
		case 70:
			printfLog(CAP_PROCESS"[CAP][O3] ");
			break;
		case 71:
			printfLog(CAP_PROCESS"[CAP][SO2] ");
			break;
		case 72:
			printfLog(CAP_PROCESS"[CAP][DONGQI] ");
			break;
		case 73:
			printfLog(CAP_PROCESS"[CAP][ZIWAI] ");
			break;
		case 74:
			printfLog(CAP_PROCESS"[CAP][TVOC] ");
			break;
		case 75:
			printfLog(CAP_PROCESS"[CAP][BEN] ");
			break;
		case 76:
			printfLog(CAP_PROCESS"[CAP][JIABEN] ");
			break;
		case 77:
			printfLog(CAP_PROCESS"[CAP][ERJIABEN] ");
			break;
		case 78:
			printfLog(CAP_PROCESS"[CAP][ANQI] ");
			break;
		case 79:
			printfLog(CAP_PROCESS"[CAP][HS] ");
			break;
		case 160:
			printfLog(CAP_PROCESS"[CAP][NO] ");
			break;
		case 161:
			printfLog(CAP_PROCESS"[CAP][NO2] ");
			break;
		case 162:
			printfLog(CAP_PROCESS"[CAP][ANQI2] ");
			break;
		case 163:
			printfLog(CAP_PROCESS"[CAP][LIGHT] ");
			break;
		case 164:
			printfLog(CAP_PROCESS"[CAP][WIESHENGWU] ");
			break;
		case 260:
			printfLog(CAP_PROCESS"[CAP][CO_PPM] ");
			break;
		case 261:
			printfLog(CAP_PROCESS"[CAP][CO2_PPM] ");
			break;	
		case 262:
			printfLog(CAP_PROCESS"[CAP][HCHO_PPM] ");
			break;
		case 270:
			printfLog(CAP_PROCESS"[CAP][O3_PPM] ");
			break;
		case 271:
			printfLog(CAP_PROCESS"[CAP][SO2_PPM] ");
			break;
		case 274:
			printfLog(CAP_PROCESS"[CAP][TVOC_PPM] ");
			break;
		case 275:
			printfLog(CAP_PROCESS"[CAP][BENG_PPM] ");
			break;
		case 276:
			printfLog(CAP_PROCESS"[CAP][JIABEN_PPM] ");
			break;
		case 277:
			printfLog(CAP_PROCESS"[CAP][ERJIABENG_PPM] ");
			break;
		case 278:
			printfLog(CAP_PROCESS"[CAP][ANQI_3] ");
			break;
		case 360:
			printfLog(CAP_PROCESS"[CAP][NO_PPM] ");
			break;
		case 361:
			printfLog(CAP_PROCESS"[CAP][NO2_PPM] ");
			break;
	}
	if((buf[0]<<8|buf[1]) != 0x0001)
	{
		for(i=3;i<len+3;i++)
			printfLog("%02x ",buf[i]);
		printfLog("\n");
	}

}

void show_factory(int zero,char *cmd,int len)
{	
	char data[32]={0};
//	unsigned int crc=(cmd[len-2]<<8)|cmd[len-1];
	if(cmd[5]==0x65 && cmd[6]==0x72 && cmd[7]==0x72 && cmd[8]==0x6f && cmd[9]==0x72)
		return ;
	//if(crc==CRC_check((unsigned char *)cmd,len-2))
	{
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
		}	
		//printf("data %s\n",data);
		if(zero)
		{
			if(cmd[3]==atoi(ID_CAP_CO))
			{
				printfLog(CAP_PROCESS"In tun zero mode CO %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_CO,6);
				write_string(ADDR_TUN_ZERO_CO,data,strlen(data));
				g_share_memory->cur_co=(cmd[5]<<8)|cmd[6];
				//return_zero_point(1);
			}
			if(cmd[3]==atoi(ID_CAP_HCHO))
			{	
				printfLog(CAP_PROCESS"In tun zero mode HCHO %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_HCHO,6);
				write_string(ADDR_TUN_ZERO_HCHO,data,strlen(data));
				g_share_memory->cur_ch2o=(cmd[5]<<8)|cmd[6];
				//return_zero_point(0);
			}
			if(cmd[3]==atoi(ID_CAP_TVOC))
			{	
				printfLog(CAP_PROCESS"In tun zero mode TVOC %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_TVOC,6);
				write_string(ADDR_TUN_ZERO_TVOC,data,strlen(data));
				g_share_memory->cur_tvoc=(cmd[5]<<8)|cmd[6];
				//return_zero_point(0);
			}
			if(cmd[3]==atoi(ID_CAP_NO))
			{	
				printfLog(CAP_PROCESS"In tun zero mode NO %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_NO,6);
				write_string(ADDR_TUN_ZERO_NO,data,strlen(data));
				g_share_memory->cur_no=(cmd[5]<<8)|cmd[6];
				//return_zero_point(0);
			}
			if(cmd[3]==atoi(ID_CAP_NO2))
			{	
				printfLog(CAP_PROCESS"In tun zero mode NO2 %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_NO2,6);
				write_string(ADDR_TUN_ZERO_NO2,data,strlen(data));
				g_share_memory->cur_no2=(cmd[5]<<8)|cmd[6];
				//return_zero_point(0);
			}
			if(cmd[3]==atoi(ID_CAP_AN_QI))
			{	
				printfLog(CAP_PROCESS"In tun zero mode NH3 %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_NH3,6);
				write_string(ADDR_TUN_ZERO_NH3,data,strlen(data));
				g_share_memory->cur_nh3=(cmd[5]<<8)|cmd[6];
				//return_zero_point(0);
			}
			if(cmd[3]==atoi(ID_CAP_CHOU_YANG))
			{	
				printfLog(CAP_PROCESS"In tun zero mode O3 %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_O3,6);
				write_string(ADDR_TUN_ZERO_O3,data,strlen(data));
				g_share_memory->cur_o3=(cmd[5]<<8)|cmd[6];
				//return_zero_point(0);
			}			
		}
		else
		{
			if(cmd[3]==g_share_memory->jiaozhun_sensor)				
			{
				clear_buf(ADDR_JIAOZHUN_REAL,10);
				write_string(ADDR_JIAOZHUN_REAL,data,strlen(data));
			}
		}
	}
	//else
	//	printfLog(CAP_PROCESS"CRC failed in zero mode\n");
}
void show_verify_point()
{
	char cmd[64]={0};
	sprintf(cmd,"%6.6f",g_share_memory->p[0]);
	write_string(ADDR_VP_0,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%6.6f",g_share_memory->p[1]);
	write_string(ADDR_VP_1,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%6.6f",g_share_memory->p[2]);
	write_string(ADDR_VP_2,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%6.6f",g_share_memory->p[3]);
	write_string(ADDR_VP_3,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%6.6f",g_share_memory->p[4]);
	write_string(ADDR_VP_4,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%6.6f",g_share_memory->p[5]);
	write_string(ADDR_VP_5,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%6.6f",g_share_memory->p[6]);
	write_string(ADDR_VP_6,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%6.6f",g_share_memory->p[7]);
	write_string(ADDR_VP_7,cmd,strlen(cmd));
	clear_point();
	switch_pic(VERIFY_PAGE);
}
void printf_msg_queue(int id)
{
	struct msqid_ds msg_info;
	int rel = msgctl(id, IPC_STAT, &msg_info);
	if (rel == -1)
		printfLog(CAP_PROCESS"get IPC_STAT failed %s\n", strerror(errno));
	else
		printfLog(CAP_PROCESS"msg_cbytes %d\nmsg_qnum %d\nmsg_qbytes %d\n\
					msg_lspid %d\nmsg_lrpid %d\n",
					msg_info.msg_cbytes,msg_info.msg_qnum,msg_info.msg_qbytes,
					msg_info.msg_lspid,msg_info.msg_lrpid);
}
int is_msg_queue(int id)
{
	struct msqid_ds msg_info;
	int rel = msgctl(id, IPC_STAT, &msg_info);
	if (rel == -1)
		printfLog(CAP_PROCESS"get IPC_STAT failed %s\n", strerror(errno));
	/*else
		printfLog(CAP_PROCESS"msg_cbytes %d\nmsg_qnum %d\nmsg_qbytes %d\n\
					msg_lspid %d\nmsg_lrpid %d\n",
					msg_info.msg_cbytes,msg_info.msg_qnum,msg_info.msg_qbytes,
					msg_info.msg_lspid,msg_info.msg_lrpid);*/
	return msg_info.msg_qnum > 0;
}
unsigned long get_cur_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec*1000 + ts.tv_nsec/1000000);
}
int send_msg(int msgid,unsigned char msg_type,char *text,int len)
{
	static unsigned long ms = 0;
	struct msg_st data;
	data.msg_type = msg_type;
	data.len=len;
	memset(data.text,'\0',512);
	//printfLog(CAP_PROCESS"send msg\n");
	//printfLog(CAP_PROCESS"MSG_TYPE %d\n",msg_type);
	if(text!=NULL)
	{
		memcpy(data.text,text,len);
	}
	//printfLog(CAP_PROCESS"sned_msg ms \n");
	//ms = get_cur_ms();
	if(msgsnd(msgid, (void*)&data, sizeof(struct msg_st)-sizeof(long int), IPC_NOWAIT) == -1)
	{
		printfLog(CAP_PROCESS"msgsnd failed %s\n",strerror(errno));
		printf_msg_queue(msgid);
	}
	//printfLog(CAP_PROCESS"send msg done\n");
	return 0;
}
void	send_cmd_to_cap(char *cmd,int len)
{
	int i=0;
	pthread_mutex_lock(&(g_share_memory->mutex));
	g_share_memory->cap_board_ack=0;
	write(g_share_memory->fd_com,cmd,len);
	while(1)
	{
		if(i>20)
		{
			printfLog(CAP_PROCESS"wait for cap_board ack timeout\n");
			break;
		}
		if (g_share_memory->cap_board_ack != 0)
			printfLog(CAP_PROCESS"cap_board_ack %d\n",g_share_memory->cap_board_ack);
		if(g_share_memory->cap_board_ack)
			break;
		else
			write(g_share_memory->fd_com,cmd,len);
		sleep(1);
		i++;			
	}
	pthread_mutex_unlock(&(g_share_memory->mutex));
}
/*
  *get cmd from lv's cap board
*/
int cap_board_mon()
{
	char 	ch = 0;
	char 	state=STATE_IDLE;
	int 	message_len=0;
	char 	message[8192]={0};
	int 	i=0;
	char 	to_check[8192]={0};
	int 	crc=0;
	int 	message_type=0;
	while(1)
	{
		if(read(g_share_memory->fd_com,&ch,1)==1)
		{
			//printfLog(CAP_PROCESS"==> %02X\n",ch);
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
					//show_cap_value(to_check+2,message_len);
					if(crc!=CRC_check((unsigned char *)to_check,message_len+5))
					{
						printfLog(CAP_PROCESS"CRC error 0x%04X\r\n",CRC_check((unsigned char *)to_check,message_len+5));
						for(i=0;i<message_len+5;i++)
							printfLog("0x%02x ",to_check[i]);
						printfLog("\n");
					}
					else
						send_msg(g_share_memory->msgid,0x33,to_check,message_len+7);
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
void cap_data_handle()
{
	struct msg_st data;
	int 	message_type=0;
	int 	message_len=0;
	int 	i=0;
	//static unsigned long ms = 0;
	//unsigned char *message=NULL;
	//if (is_msg_queue(g_share_memory->msgid))
	//{
	//printfLog(CAP_PROCESS"Enter cap_data_handle\n");
	//printfLog(CAP_PROCESS"cap_data \n");
	//ms = get_cur_ms();
	if(msgrcv(g_share_memory->msgid, (void*)&data, sizeof(struct msg_st)-sizeof(long int), 0x33 , 0)>=0)
	{
		//printfLog(CAP_PROCESS"msgget len: %d\n", data.len);		
		char *cmd=(char *)malloc(data.len);
		memset(cmd,'\0',data.len);
		memcpy(cmd,data.text,data.len);
		char *message=(char *)malloc(data.len-7);
		memset(message,'\0',data.len-7);
		memcpy(message,data.text+5,data.len-7);

		message_type=((cmd[2]<<8)|cmd[3]);
		message_len=data.len-7;
		if(message_type==0x0005)
		{
			g_share_memory->cap_board_ack=1;
			printfLog(CAP_PROCESS"Got Ack from cap board\n");
			free(cmd);
			free(message);
			return ;
		}
		else if (message_type == 0x0006)
		{
			memset(g_share_memory->hw_ver, 0, 20);
			memcpy(g_share_memory->hw_ver, message, 13);
			printfLog(CAP_PROCESS"Got hw_ver %s from cap board\n",
						g_share_memory->hw_ver);
			free(cmd);
			free(message);
			return;
		}
		if(g_share_memory->factory_mode==NORMAL_MODE)
		{
			if(message_type == 0x0004)
			{
				for(i=0;i<message_len;i=i+2)
				{
					g_share_memory->sensor_interface_mem[i/2]=(message[i]<<8)|message[i+1];
					printfLog(CAP_PROCESS"sensor_interface[%d] = %4x\n",i/2,g_share_memory->sensor_interface_mem[i/2]);
				}
			}
			else
			{
				//unsigned long build_ms = get_cur_ms();
				//printfLog(CAP_PROCESS"enter build message\n");
				post_message=build_message(cmd,message_len+7,post_message);
				//printfLog(CAP_PROCESS"build_ms \n");
			}
		}
		else if(g_share_memory->factory_mode==TUN_ZERO_MODE)
		{
			if(message_type!=0x0001 && message_type!=0x0005)
				show_factory(1,cmd,message_len+7);
			if(post_message!=NULL)
			{
				free(post_message);
				post_message=NULL;
			}
		}
		else
		{
			if(post_message!=NULL)
			{
				free(post_message);
				post_message=NULL;
			}
			if(message_type == 0x0003)
			{							
				g_share_memory->y=message[message_len-1];
				printfLog(CAP_PROCESS". = %d\n",message[message_len-1]);
				for(i=0;i<16;i=i+2)
				{
					g_share_memory->p[i/2]=(message[i]<<8)|message[i+1];
					if(g_share_memory->y!=0)
					{
						int m=1,j=0;
						for(j=0;j<g_share_memory->y;j++)
							m=m*10;
						g_share_memory->p[i/2]=g_share_memory->p[i/2]/(float)m;	
					}
					printfLog(CAP_PROCESS"verify_point[%d] = %d\n",i/2,(message[i]<<8)|message[i+1]);
				}
				for(i=16;i<32;i=i+2)
				{
					g_share_memory->x[(i-16)/2]=(message[i]<<8)|message[i+1];
					if(g_share_memory->y!=0)
					{
						int m=1,j=0;
						for(j=0;j<g_share_memory->y;j++)
							m=m*10;
						g_share_memory->x[(i-16)/2]=g_share_memory->x[(i-16)/2]/(float)m;	
					}
					printfLog(CAP_PROCESS"xiuzhen[%d] = %d\n",(i-16)/2,(message[i]<<8)|message[i+1]);
				}
				show_verify_point();
			}
			else
			{
				if(message_type!=0x0001 && message_type!=0x0005)
					show_factory(0,cmd,message_len+7);
			}
		}
		free(cmd);
		//printfLog(CAP_PROCESS"free cmd \n");
		free(message);
		//printfLog(CAP_PROCESS"free message \n");
	}
	else
	{		
		if (errno != EINTR) {
			printfLog(CAP_PROCESS"msgrcv failed with error: %s\n", strerror(errno));
			printf_msg_queue(g_share_memory->msgid);
		}
	}
	//}
}
/*
 * open com port
 * create a thread to mon cap board
*/
int cap_init()
{
	int fpid = 0;
	g_share_memory->msgid = msgget((key_t)1234, 0666 | IPC_CREAT);  
	if(g_share_memory->msgid == -1)  
	{  
		printfLog(CAP_PROCESS"msgget failed with error: %d\n", errno);  
		exit(-1);  
	}
	else
		printfLog(CAP_PROCESS"msgid %d\n",g_share_memory->msgid);
	fpid=fork();
	if(fpid==0)
	{
		g_share_memory = (struct share_memory *)shmat(shmid_share_memory,0, 0);	
		while(1)
			cap_board_mon();
	}
	else
		printfLog(CAP_PROCESS"[PID]%d cap process read data\n",fpid);
	fpid=fork();
	if(fpid==0)
	{
		int i=0;
		sensor_history.co= (struct nano *)shmat(shmid_history_co,0, 0);
		sensor_history.co2 = (struct nano *)shmat(shmid_history_co2,0, 0);
		sensor_history.hcho= (struct nano *)shmat(shmid_history_hcho,0, 0);
		sensor_history.temp= (struct nano *)shmat(shmid_history_temp,0, 0);
		sensor_history.shidu= (struct nano *)shmat(shmid_history_shidu,0, 0);
		sensor_history.pm25= (struct nano *)shmat(shmid_history_pm25,0, 0);		
		sensor_history.pm10= (struct nano *)shmat(shmid_history_pm10,0, 0);
		sensor_history.noise = (struct nano *)shmat(shmid_history_noise,0, 0);
		sensor_history.press= (struct nano *)shmat(shmid_history_press,0, 0);
		sensor_history.tvoc= (struct nano *)shmat(shmid_history_tvoc,0, 0);
		sensor_history.o3= (struct nano *)shmat(shmid_history_o3,0, 0);
		sensor_history.wind= (struct nano *)shmat(shmid_history_wind,0, 0);
		g_share_memory = (struct share_memory *)shmat(shmid_share_memory,0, 0);		
		g_share_memory->pj_cnt[SENSOR_CO]=0;
		g_share_memory->pj_cnt[SENSOR_CO2]=0;
		g_share_memory->pj_cnt[SENSOR_HCHO]=0;
		g_share_memory->pj_cnt[SENSOR_SHIDU]=0;
		g_share_memory->pj_cnt[SENSOR_TEMP]=0;
		g_share_memory->pj_cnt[SENSOR_PM25]=0;
		g_share_memory->pj_cnt[SENSOR_WIND]=0;
		g_share_memory->pj_cnt[SENSOR_PRESS]=0;
		g_share_memory->pj_cnt[SENSOR_NOISE]=0;
		g_share_memory->pj_cnt[SENSOR_TVOC]=0;
		g_share_memory->pj_cnt[SENSOR_O3]=0;
		g_share_memory->pj_cnt[SENSOR_PM10]=0;
		for(i=0;i<SENSOR_NO;i++)
		g_share_memory->sensor_has_data[i]=0;
		for(i=0;i<SENSOR_NO;i++)
		g_share_memory->sensor_off[i]=0;
		write_data(ADDR_O3_SHOW_PIC,0x01);
		write_data(ADDR_TVOC_SHOW_PIC,0x01);
		write_data(ADDR_PRESS_SHOW_PIC,0x01);
		write_data(ADDR_NOISE_SHOW_PIC,0x01);
		write_data(ADDR_WIND_SHOW_PIC,0x01);
		write_data(ADDR_SHIDU_SHOW_PIC,0x01);
		write_data(ADDR_TEMP_SHOW_PIC,0x01);
		write_data(ADDR_CO2_SHOW_PIC,0x01);
		write_data(ADDR_CO_SHOW_PIC,0x01);
		write_data(ADDR_PM25_SHOW_PIC,0x01);
		write_data(ADDR_PM10_SHOW_PIC,0x01);
		write_data(ADDR_HCHO_SHOW_PIC,0x01);
		write_data(ADDR_O3_SHOW_PIC_PPM,0x01);
		write_data(ADDR_TVOC_SHOW_PIC_PPM,0x01);
		write_data(ADDR_PRESS_SHOW_PIC_PPM,0x01);
		write_data(ADDR_NOISE_SHOW_PIC_PPM,0x01);
		write_data(ADDR_WIND_SHOW_PIC_PPM,0x01);
		write_data(ADDR_SHIDU_SHOW_PIC_PPM,0x01);
		write_data(ADDR_TEMP_SHOW_PIC_PPM,0x01);
		write_data(ADDR_CO2_SHOW_PIC_PPM,0x01);
		write_data(ADDR_CO_SHOW_PIC_PPM,0x01);
		write_data(ADDR_PM25_SHOW_PIC_PPM,0x01);
		write_data(ADDR_PM10_SHOW_PIC_PPM,0x01);
		write_data(ADDR_HCHO_SHOW_PIC_PPM,0x01);
		signal(SIGALRM, set_upload_flag);
		alarm(20);
		while(1)
			cap_data_handle();
	}
	else
		printfLog(CAP_PROCESS"[PID]%d cap process handle data\n",fpid);
	return 0;
}
