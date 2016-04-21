#include "cap.h"
#include "misc.h"
#include "dwin.h"
#include "netlib.h"
#include "xfer.h"
#define CAP_PROCESS "[CAP_PROCESS] "

int g_upload=0;
char *post_message=NULL,*warnning_msg=NULL;
extern char g_uuid[256];
extern char ip[20];
//set g_upload flag to make 10 mins upload once.
void set_upload_flag(int a)
{
	printfLog(CAP_PROCESS"set upload flag\n");
	g_upload=1;
	alarm(600);
}
//format sensor history data
void set_upload_data(char *id,struct nano *history,long *cnt,char *data,char *date)
{
	memset(history[*cnt].data,'\0',10);
	if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
		sprintf(history[*cnt].data,"%04d",atoi(data));
	else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
		sprintf(history[*cnt].data,"%03d",atoi(data));
	else				
		strcpy(history[*cnt].data,data);	
	memset(history[*cnt].time,'\0',20);
	strcpy(history[*cnt].time,date);							
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
		char *data=doit_data(message,ID_CAP_CO);
		if(data!=NULL)
			set_upload_data(ID_CAP_CO,&(sensor_history.co[g_share_memory->cnt[SENSOR_CO]]),
			&(g_share_memory->cnt[SENSOR_CO]),data,date);
		
		data=doit_data(message,ID_CAP_CO2);
		if(data!=NULL)
			set_upload_data(ID_CAP_CO2,&(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]]),
			&(g_share_memory->cnt[SENSOR_CO2]),data,date);
		
		data=doit_data(message,ID_CAP_SHI_DU);
		if(data!=NULL)
			set_upload_data(ID_CAP_SHI_DU,&(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]]),
			&(g_share_memory->cnt[SENSOR_SHIDU]),data,date);
		
		data=doit_data(message,ID_CAP_HCHO);
		if(data!=NULL)
			set_upload_data(ID_CAP_HCHO,&(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]]),
			&(g_share_memory->cnt[SENSOR_HCHO]),data,date);
		
		data=doit_data(message,ID_CAP_TEMPERATURE);
		if(data!=NULL)
			set_upload_data(ID_CAP_TEMPERATURE,&(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]]),
			&(g_share_memory->cnt[SENSOR_TEMP]),data,date);
		
		data=doit_data(message,ID_CAP_PM_25);
		if(data!=NULL)
			set_upload_data(ID_CAP_PM_25,&(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]]),
			&(g_share_memory->cnt[SENSOR_PM25]),data,date);
	}
	send_web_post(URL,message,9,&rcv);
	if(rcv != NULL)
	{	
		//int len = strlen(rcv);
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
char *count_sensor_value(char cmd,char *json,int value)
{
	char *times=NULL;
	char *sent;
	int min,max;
	char *alarm;
	char id[256]={0};
	if(cmd==atoi(ID_CAP_CO2))
	{
		times=&(g_share_memory->times[SENSOR_CO2]);
		sent=&(g_share_memory->sent[SENSOR_CO2]);
		min=MIN_CO2;
		max=MAX_CO2;
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
	}
	else if(cmd==atoi(ID_CAP_CO))
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
	if(times!=NULL)
	{
		//printfLog(CAP_PROCESS"count_sensor_value cmd %d,value %d,min %d,max %d,times %d,sent %d,alarm %d,json %s\n",
		//	cmd,value,min,max,*times,*sent,*alarm,json);
		if(value<min)
			(*times)++;
		else if(value>max)
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
			if(cmd==atoi(ID_CAP_CO))
				co_flash_alarm();
			json=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
			json=add_item(json,ID_DEVICE_UID,g_share_memory->uuid);
			json=add_item(json,ID_DEVICE_IP_ADDR,g_share_memory->ip);
			json=add_item(json,ID_DEVICE_PORT,(char *)"9517");
			if(value<min)
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
	}
	//printfLog(CAP_PROCESS"count_sensor_value <== %s\n",json);
	return json;
}
void update_dwin_real_value(char *id,int value)
{
	if(strncmp(id,ID_CAP_CO,strlen(ID_CAP_CO))==0)
	{
		write_data(VAR_DATE_TIME_1,value);
	}
	else if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
	{						
		write_data(VAR_DATE_TIME_2,value);
	}
	else if(strncmp(id,ID_CAP_HCHO,strlen(ID_CAP_HCHO))==0)
	{
		write_data(VAR_DATE_TIME_3,value);
	}
	else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(ID_CAP_TEMPERATURE))==0)
	{
		write_data(VAR_DATE_TIME_4,value);
	}
	else if(strncmp(id,ID_CAP_SHI_DU,strlen(ID_CAP_SHI_DU))==0)
	{
		write_data(VAR_ALARM_TYPE_1,value);
	}
	else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
	{
		write_data(VAR_ALARM_TYPE_2,value);
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
	unsigned int crc=(cmd[len-2]<<8)|cmd[len-1];
	int message_type=(cmd[2]<<8)|cmd[3];
	sprintf(id,"%d",message_type);
	//printfLog(CAP_PROCESS"crc 0x%04X,message_type %d,len %d \n",crc,message_type,len);
	if(crc==CRC_check((unsigned char *)cmd,len-2))
	{
		i=0;
		switch(message_type)
		{
			case TIME_BYTE:
			{	//TIME_BYTE got ,we can send to server now
				sprintf(date,"20%02d-%02d-%02d %02d:%02d",cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
				printfLog(CAP_PROCESS"date is %s\r\n",date);
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
					if(warnning_msg!=NULL)
						free(warnning_msg);
					warnning_msg=NULL;
				}
				if(g_upload)
				{
					g_upload=0;
					printfLog(CAP_PROCESS"Upload data msg :\n");
					send_server_save_local(date,message,1);
				}
				free(message);
				message=NULL;
				memset(date,'\0',32);
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
					}
					else if(cmd[3]==atoi(ID_CAP_CO) && !(g_share_memory->alarm[SENSOR_CO] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_CO]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_CO]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_HCHO) && !(g_share_memory->alarm[SENSOR_HCHO] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_HCHO]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_HCHO]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_SHI_DU) && !(g_share_memory->alarm[SENSOR_SHIDU] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_SHIDU]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_SHIDU]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& !(g_share_memory->alarm[SENSOR_TEMP] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_TEMP]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_TEMP]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_PM_25)&& !(g_share_memory->alarm[SENSOR_PM25] & ALARM_UNINSERT))
					{
						g_share_memory->alarm[SENSOR_PM25]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_PM25]=0;
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
					int value=0;
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
						value=((cmd[5]<<8|cmd[6]))/temp;
					}	
					else
					{
							value=(cmd[5]<<8|cmd[6]);
					}
					//printfLog(CAP_PROCESS"1 Value %d\n",value);
					warnning_msg=count_sensor_value(cmd[3],warnning_msg,value);
					//printfLog(CAP_PROCESS"0 id %s data %s\r\n",id,data);
					//real time update cap data
					update_dwin_real_value(id,cmd[5]<<8|cmd[6]);
					//printfLog(CAP_PROCESS"1 id %s data %s\r\n",id,data);
					message=add_item(message,id,data);
					//printfLog(CAP_PROCESS"2 id %s data %s\r\n==>\n%s\n",id,data,message);
					return message;
				}
			}
			break;
		}
	}
	else
	{
		printfLog(CAP_PROCESS"CRC error 0x%04X\r\n",CRC_check((unsigned char *)cmd,len-2));
		for(i=0;i<len;i++)
			printfLog("0x%02x ",cmd[i]);
	}
	return message;
}
void return_zero_point(int co)
{
	int crc = 0;
	int i =0;
	char cmd_return_point[]=	{0x6c,ARM_TO_CAP,0x00,0x05,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
	if(co)
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
	else
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
	cmd_return_point[5]=i+1;
	crc=CRC_check((unsigned char *)cmd_return_point,9);
	cmd_return_point[9]=(crc&0xff00)>>8;cmd_return_point[10]=crc&0x00ff;
	for(i=0;i<sizeof(cmd_return_point);i++)
		printfLog("%02X ",cmd_return_point[i]);
	printfLog("\n");
	write(g_share_memory->fd_com,cmd_return_point,sizeof(cmd_return_point));
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
			printfLog(CAP_PROCESS"[CAP][FENGSU] ");
			break;
		case 69:
			printfLog(CAP_PROCESS"[CAP][QIYA] ");
			break;
		case 70:
			printfLog(CAP_PROCESS"[CAP][CHOUYANG] ");
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
			printfLog(CAP_PROCESS"[CAP][CO_2] ");
			break;
		case 262:
			printfLog(CAP_PROCESS"[CAP][CH2O] ");
			break;
		case 270:
			printfLog(CAP_PROCESS"[CAP][CHOYANG2] ");
			break;
		case 271:
			printfLog(CAP_PROCESS"[CAP][SO2_2] ");
			break;
		case 274:
			printfLog(CAP_PROCESS"[CAP][TVOC2] ");
			break;
		case 275:
			printfLog(CAP_PROCESS"[CAP][BENG_2] ");
			break;
		case 276:
			printfLog(CAP_PROCESS"[CAP][JIABEN_2] ");
			break;
		case 277:
			printfLog(CAP_PROCESS"[CAP][ERJIABENG_2] ");
			break;
		case 278:
			printfLog(CAP_PROCESS"[CAP][ANQI_3] ");
			break;
		case 360:
			printfLog(CAP_PROCESS"[CAP][NO_2] ");
			break;
		case 361:
			printfLog(CAP_PROCESS"[CAP][NO2_2] ");
			break;
	}
	for(i=3;i<len+3;i++)
		printfLog("%02x ",buf[i]);
	printfLog("\n");

}

void show_factory(int zero,char *cmd,int len)
{	
	char data[32]={0};
	unsigned int crc=(cmd[len-2]<<8)|cmd[len-1];
	if(cmd[5]==0x65 && cmd[6]==0x72 && cmd[7]==0x72 && cmd[8]==0x6f && cmd[9]==0x72)
		return ;
	if(crc==CRC_check((unsigned char *)cmd,len-2))
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
				clear_buf(ADDR_TUN_ZERO_CO,4);
				write_string(ADDR_TUN_ZERO_CO,data,strlen(data));
				g_share_memory->cur_co=(cmd[5]<<8)|cmd[6];
				//return_zero_point(1);
			}
			if(cmd[3]==atoi(ID_CAP_HCHO))
			{	
				printfLog(CAP_PROCESS"In tun zero mode HCHO %d %d %d %s\n",cmd[5],cmd[6],cmd[7],data);
				clear_buf(ADDR_TUN_ZERO_HCHO,4);
				write_string(ADDR_TUN_ZERO_HCHO,data,strlen(data));
				g_share_memory->cur_ch2o=(cmd[5]<<8)|cmd[6];
				//return_zero_point(0);
			}
		}
		else
		{
			if(cmd[3]==g_share_memory->jiaozhun_sensor)				
			{
				clear_buf(ADDR_REAL_VALUE,4);
				write_string(ADDR_REAL_VALUE,data,strlen(data));
			}
		}
	}
	else
		printfLog(CAP_PROCESS"CRC failed in zero mode\n");
}
void show_verify_point()
{
	char cmd[64]={0};
	sprintf(cmd,"%f",g_share_memory->p[0]);
	write_string(ADDR_VERIFY_P_0,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%f",g_share_memory->p[1]);
	write_string(ADDR_VERIFY_P_1,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%f",g_share_memory->p[2]);
	write_string(ADDR_VERIFY_P_2,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%f",g_share_memory->p[3]);
	write_string(ADDR_VERIFY_P_3,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%f",g_share_memory->p[4]);
	write_string(ADDR_VERIFY_P_4,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%f",g_share_memory->p[5]);
	write_string(ADDR_VERIFY_P_5,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%f",g_share_memory->p[6]);
	write_string(ADDR_VERIFY_P_6,cmd,strlen(cmd));
	memset(cmd,'\0',64);
	sprintf(cmd,"%f",g_share_memory->p[7]);
	write_string(ADDR_VERIFY_P_7,cmd,strlen(cmd));
	clear_point();
}

/*
  *get cmd from lv's cap board
*/
int cap_board_mon()
{
	char 	ch = 0;
	char 	state=STATE_IDLE;
	int 	message_len=0;
	char 	message[10]={0};
	int 	i=0;
	char 	to_check[20]={0};
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
					char *cmd=(char *)malloc(message_len+7);
					memset(cmd,'\0',message_len+7);
					memcpy(cmd,to_check,message_len+7);
					show_cap_value(to_check+2,message_len);
					printfLog(CAP_PROCESS"factory_mode %d,message_type %d\n",g_share_memory->factory_mode,
						message_type);
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
							post_message=build_message(cmd,message_len+7,post_message);
					}
					else if(g_share_memory->factory_mode==TUN_ZERO_MODE)
					{
						if(message_type!=0x0001)
						show_factory(1,cmd,message_len+7);
					}
					else
					{
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
									g_share_memory->p[i/2]=g_share_memory->p[i/2]/m;	
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
									g_share_memory->x[(i-16)/2]=g_share_memory->x[(i-16)/2]/m;	
								}
								printfLog(CAP_PROCESS"xiuzhen[%d] = %d\n",(i-16)/2,(message[i]<<8)|message[i+1]);
							}
							show_verify_point();
						}
						else
						show_factory(0,cmd,message_len+7);
					}
					free(cmd);
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
/*
 * open com port
 * create a thread to mon cap board
*/
int cap_init()
{
	int fpid = 0;
	/*if((shmid_history = shmget(IPC_PRIVATE, sizeof(struct history)*100000, PERM)) == -1 )
	{
        printfLog(CAP_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);
    }*/
	if((g_share_memory->fd_com=open_com_port("/dev/ttySP0"))<0)
	{
		printfLog(CAP_PROCESS"open_port cap error");
		return -1;
	}
	if(set_opt(g_share_memory->fd_com,9600,8,'N',1)<0)
	{
		printfLog(CAP_PROCESS"set_opt cap error");
		return -1;
	}
	fpid=fork();
	if(fpid==0)
	{
		sensor_history.co= (struct nano *)shmat(shmid_history_co,0, 0);
		sensor_history.co2 = (struct nano *)shmat(shmid_history_co2,0, 0);
		sensor_history.hcho= (struct nano *)shmat(shmid_history_hcho,0, 0);
		sensor_history.temp= (struct nano *)shmat(shmid_history_temp,0, 0);
		sensor_history.shidu= (struct nano *)shmat(shmid_history_shidu,0, 0);
		sensor_history.pm25= (struct nano *)shmat(shmid_history_pm25,0, 0);
		g_share_memory = (struct share_memory *)shmat(shmid_share_memory,0, 0);
		signal(SIGALRM, set_upload_flag);
		alarm(600);
		while(1)
			cap_board_mon();
	}
	else
		printfLog(CAP_PROCESS"[PID]%d cap process\n",fpid);
	return 0;
}
