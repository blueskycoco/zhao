#include "uart.h"
#include "netlib.h"
#include "log.h"
#define CAP_PROCESS "[CAP_PROCESS]"

struct history g_history;
int g_upload=0;
char *post_message=NULL,*warnning_msg=NULL;
extern char g_uuid[256];
extern char ip[20];

sensor_alarm sensor;
//set g_upload flag to make 10 mins upload once.
void set_upload_flag(int a)
{
	printfLog(CAP_PROCESS"set upload flag\n");
	g_upload=1;
	alarm(600);
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
			set_upload_data(ID_CAP_CO,g_history.sensor_history[SENSOR_CO],
			g_history.cnt[SENSOR_CO],data,date);
		
		data=doit_data(message,ID_CAP_CO2);
		if(data!=NULL)
			set_upload_data(ID_CAP_CO2,g_history.sensor_history[SENSOR_CO2],
			g_history.cnt[SENSOR_CO2],data,date);
		
		data=doit_data(message,ID_CAP_SHI_DU);
		if(data!=NULL)
			set_upload_data(ID_CAP_SHI_DU,g_history.sensor_history[SENSOR_SHIDU],
			g_history.cnt[SENSOR_SHIDU],data,date);
		
		data=doit_data(message,ID_CAP_HCHO);
		if(data!=NULL)
			set_upload_data(ID_CAP_HCHO,g_history.sensor_history[SENSOR_HCHO]),
			g_history.cnt[SENSOR_HCHO],data,date);
		
		data=doit_data(message,ID_CAP_TEMPERATURE);
		if(data!=NULL)
			set_upload_data(ID_CAP_TEMPERATURE,g_history.sensor_history[SENSOR_TEMP]),
			g_history.cnt[SENSOR_TEMP],data,date);
		
		data=doit_data(message,ID_CAP_PM_25);
		if(data!=NULL)
			set_upload_data(ID_CAP_PM_25,g_history.sensor_history[SENSOR_PM25]),
			g_history.cnt[SENSOR_PM25],data,date);
	}
	send_web_post(URL,message,9,&rcv);
	if(rcv != NULL)
	{	
		int len = strlen(rcv);
		free(rcv);
		rcv=NULL;
	}			
}
void clear_alarm(char *id,char *alarm_type)
{
	char *clear_msg=NULL;
	clear_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_CLEAR_WARNING);
	clear_msg=add_item(clear_msg,ID_DEVICE_UID,g_uuid);
	clear_msg=add_item(clear_msg,ID_DEVICE_IP_ADDR,ip);
	clear_msg=add_item(clear_msg,ID_DEVICE_PORT,"9517");						
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
		if(json!=NULL)
			free(json);
		json=NULL;
		*sent=1;
	}
	return json;
}
/*
  *build json from cmd
  *update cap data to lcd
  *create alarm and normal message to server
*/
char *build_message(int fd,char *cmd,int len,char *message)
{	
	int i=0;
	char sensor_error[]={0x65,0x72,0x72,0x6f,0x72};
	char id[32]={0},data[32]={0},date[32]={0},error[32]={0};
	unsigned int crc=(cmd[len-2]<<8)|cmd[len-1];
	int message_type=(cmd[2]<<8)|cmd[3];
	sprintf(id,"%d",message_type);
	//printfLog(CAP_PROCESS"crc 0x%04X,message_type %d,len %d \n",crc,message_type,len);
	if(crc==CRC_check(cmd,len-2))
	{
		i=0;
		switch(message_type)
		{
			case TIME_BYTE:
			{	//TIME_BYTE got ,we can send to server now
				sprintf(date,"20%02d-%02d-%02d %02d:%02d",cmd[5],cmd[6],cmd[7],cmd[8],cmd[9],cmd[10]);
				printfLog(CAP_PROCESS"date is %s\r\n",date);
				message=add_item(message,ID_DEVICE_CAP_TIME,date);
				if(warnning_msg!=NULL)
				{	//have alarm msg upload
					warnning_msg=update_alarm(warnning_msg,ID_CAP_CO2,
									&(sensor.alarm[SENSOR_CO2]),&(sensor.sent[SENSOR_CO2]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_CO,
									&(sensor.alarm[SENSOR_CO]),&(sensor.sent[SENSOR_CO]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_HCHO,
									&(sensor.alarm[SENSOR_HCHO]),&(sensor.sent[SENSOR_HCHO]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_SHI_DU,
									&(sensor.alarm[SENSOR_SHIDU]),&(sensor.sent[SENSOR_SHIDU]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_TEMPERATURE,
									&(sensor.alarm[SENSOR_TEMP]),&(sensor.sent[SENSOR_TEMP]));
					warnning_msg=update_alarm(warnning_msg,ID_CAP_PM_25,
									&(sensor.alarm[SENSOR_PM25]),&(sensor.sent[SENSOR_PM25]));
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
				write(fd,server_time,13);
			}
			break;
			default:
			{
				/*get cap data*/
				if(memcmp(cmd+5,sensor_error,5)==0)
				{	//check uninsert msg
					if(cmd[3]==atoi(ID_CAP_CO2) && !sensor.co2)
					{
						sensor.co2|=ALARM_UNINSERT;
						sensor.co2_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_CO) && !sensor.co)
					{
						sensor.co|=ALARM_UNINSERT;
						sensor.co_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_HCHO) && !sensor.hcho)
					{
						sensor.hcho|=ALARM_UNINSERT;
						sensor.hcho_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_SHI_DU) && !sensor.shidu)
					{
						sensor.shidu|=ALARM_UNINSERT;
						sensor.shidu_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& !sensor.temp)
					{
						sensor.temp|=ALARM_UNINSERT;
						sensor.temp_send=0;
					}
					else if(cmd[3]==atoi(ID_CAP_PM_25)&& !sensor.pm25)
					{
						sensor.pm25|=ALARM_UNINSERT;
						sensor.pm25_send=0;
					}
					else 
						return message;
					//inform server
					//sprintf(error,"%dth sensor possible error",message_type);
					warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
					//warnning_msg=add_item(warnning_msg,ID_ALERT_CAP_FAILED,error);
					//warnning_msg=add_item(warnning_msg,ID_ALARM_SENSOR,id);
					warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UNINSERT);
					memset(error,'\0',32);
					save_sensor_alarm_info();					
					return message;
				}
				else
				{	//normal data or beyond Min & Max data
					int value=0;
					//clear the uninsert alarm
					if(cmd[3]==atoi(ID_CAP_CO2) && (sensor.co2&ALARM_UNINSERT))
					{					
						clear_alarm(ID_CAP_CO2,ID_ALERT_UNINSERT);						
						sensor.co2&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_CO) && (sensor.co&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_CO,ID_ALERT_UNINSERT);
						sensor.co&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_HCHO) && (sensor.hcho&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_HCHO,ID_ALERT_UNINSERT);
						sensor.hcho&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_SHI_DU) && (sensor.shidu&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_SHI_DU,ID_ALERT_UNINSERT);
						sensor.shidu&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& (sensor.temp&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_UNINSERT);
						sensor.temp&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_PM_25)&& (sensor.pm25&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_PM_25,ID_ALERT_UNINSERT);
						sensor.pm25&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(message==NULL)
					{
						message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
						message=add_item(message,ID_DEVICE_UID,g_uuid);
						message=add_item(message,ID_DEVICE_IP_ADDR,ip);
						message=add_item(message,ID_DEVICE_PORT,"9517");	
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
						//if(g_upload)
						{
							int temp=1;
							for(i=0;i<cmd[7];i++)
								temp*=10;
							value=((cmd[5]<<8|cmd[6]))/temp;
						}
					}	
					else
					{
						//if(g_upload)
							value=(cmd[5]<<8|cmd[6]);
					}
					//printfLog(CAP_PROCESS"Value %d\n",value);
					#if 1
					//if(g_upload)
					{
						if(cmd[3]==atoi(ID_CAP_CO2))
						{
							if(value<MIN_CO2)
								sensortimes.co2++;
							else if(value>MAX_CO2)
								sensortimes.co2++;
							else
							{
								sensortimes.co2=0;
								if(sensor.co2&ALARM_BELOW)
									clear_alarm(ID_CAP_CO2,ID_ALERT_BELOW);
								if(sensor.co2&ALARM_UP)
									clear_alarm(ID_CAP_CO2,ID_ALERT_UP);
								if(sensor.co2&ALARM_BELOW||sensor.co2&ALARM_UP)
								{
									sensor.co2=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.co2==MAX_COUNT_TIMES && !sensor.co2)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_CO2)
								{
									sensor.co2|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.co2|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.co2_send=0;
							}
						}
						if(cmd[3]==atoi(ID_CAP_CO))
						{
							if(value<MIN_CO)
								sensortimes.co++;
							else if(value>MAX_CO)
								sensortimes.co++;
							else
							{
								sensortimes.co=0;								
								if(sensor.co&ALARM_BELOW)
									clear_alarm(ID_CAP_CO,ID_ALERT_BELOW);
								if(sensor.co&ALARM_UP)
									clear_alarm(ID_CAP_CO,ID_ALERT_UP);
								if(sensor.co&ALARM_BELOW||sensor.co&ALARM_UP)
								{
									sensor.co=ALARM_NONE;
									save_sensor_alarm_info();	
								}
							}
							if(sensortimes.co==MAX_COUNT_TIMES && !sensor.co)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_CO)
								{
									sensor.co|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.co|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();
								sensor.co_send=0;								
							}
						}
						if(cmd[3]==atoi(ID_CAP_HCHO))
						{
							if(value<MIN_HCHO)
								sensortimes.hcho++;
							else if(value>MAX_HCHO)
								sensortimes.hcho++;
							else
							{
								sensortimes.hcho=0;
								if(sensor.hcho&ALARM_BELOW)
									clear_alarm(ID_CAP_HCHO,ID_ALERT_BELOW);
								if(sensor.hcho&ALARM_UP)
									clear_alarm(ID_CAP_HCHO,ID_ALERT_UP);	
								if(sensor.hcho&ALARM_BELOW||sensor.hcho&ALARM_UP)
								{
									sensor.hcho=ALARM_NONE;
									save_sensor_alarm_info();	
								}
							}
							if(sensortimes.hcho==MAX_COUNT_TIMES && !sensor.hcho)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_HCHO)
								{
									sensor.hcho|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.hcho|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.hcho_send=0;				
							}
						}
						if(cmd[3]==atoi(ID_CAP_SHI_DU))
						{
							if(value<MIN_SHIDU)
								sensortimes.shidu++;
							else if(value>MAX_SHIDU)
								sensortimes.shidu++;
							else
							{
								sensortimes.shidu=0;								
								if(sensor.shidu&ALARM_BELOW)
									clear_alarm(ID_CAP_SHI_DU,ID_ALERT_BELOW);
								if(sensor.shidu&ALARM_UP)
									clear_alarm(ID_CAP_SHI_DU,ID_ALERT_UP);
								if(sensor.shidu&ALARM_BELOW||sensor.shidu&ALARM_UP)
								{
									sensor.shidu=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.shidu==MAX_COUNT_TIMES && !sensor.shidu)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_SHIDU)
								{
									sensor.shidu|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.shidu|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.shidu_send=0;				
							}
						}
						if(cmd[3]==atoi(ID_CAP_TEMPERATURE))
						{
							if(value<MIN_TEMP)
								sensortimes.temp++;
							else if(value>MAX_TEMP)
								sensortimes.temp++;
							else
							{
								sensortimes.temp=0;
								if(sensor.temp&ALARM_BELOW)
									clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_BELOW);
								if(sensor.temp&ALARM_UP)
									clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_UP);
								if(sensor.temp&ALARM_BELOW||sensor.temp&ALARM_UP)
								{
									sensor.temp=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.temp==MAX_COUNT_TIMES&& !sensor.temp)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_TEMP)
								{
									sensor.temp|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.temp|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.temp_send=0;				
							}
						}
						if(cmd[3]==atoi(ID_CAP_PM_25))
						{
							if(value<MIN_PM25)
								sensortimes.pm25++;
							else if(value>MAX_PM25)
								sensortimes.pm25++;
							else
							{
								sensortimes.pm25=0;
								if(sensor.pm25&ALARM_BELOW)
									clear_alarm(ID_CAP_PM_25,ID_ALERT_BELOW);
								if(sensor.pm25&ALARM_UP)
									clear_alarm(ID_CAP_PM_25,ID_ALERT_UP);
								if(sensor.pm25&ALARM_BELOW||sensor.pm25&ALARM_UP)
								{
									sensor.pm25=ALARM_NONE;
									save_sensor_alarm_info();	
								}	
							}
							if(sensortimes.pm25==MAX_COUNT_TIMES&& !sensor.pm25)
							{	
								//need send server alarm
								warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
								warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
								if(value<MIN_PM25)
								{
									sensor.pm25|=ALARM_BELOW;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_BELOW);
								}
								else
								{
									sensor.pm25|=ALARM_UP;
									warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UP);
								}
								save_sensor_alarm_info();	
								sensor.pm25_send=0;				
							}
						}			
					}
					#endif
					//real time update cap data
					if(strncmp(id,ID_CAP_CO,strlen(ID_CAP_CO))==0)
					{
						write_data(fd_lcd,VAR_DATE_TIME_1,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
					{						
						write_data(fd_lcd,VAR_DATE_TIME_2,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_HCHO,strlen(ID_CAP_HCHO))==0)
					{
						write_data(fd_lcd,VAR_DATE_TIME_3,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(ID_CAP_TEMPERATURE))==0)
					{
						write_data(fd_lcd,VAR_DATE_TIME_4,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_SHI_DU,strlen(ID_CAP_SHI_DU))==0)
					{
						write_data(fd_lcd,VAR_ALARM_TYPE_1,cmd[5]<<8|cmd[6]);
					}
					else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
					{
						write_data(fd_lcd,VAR_ALARM_TYPE_2,cmd[5]<<8|cmd[6]);
					}
					message=add_item(message,id,data);
					//printfLog(CAP_PROCESS"id %s data %s\r\n==>\n%s\n",id,data,post_message);
					return message;
				}
			}
			break;
		}
	}
	else
	{
		printfLog(CAP_PROCESS"CRC error 0x%04X\r\n",CRC_check(cmd,len-2));
		for(i=0;i<len;i++)
			printfLog(CAP_PROCESS"0x%02x ",cmd[i]);
	}
	return message;
}

/*
  *get cmd from lv's cap board
*/
int cap_board_mon(int fd)
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
		if(read(fd,&ch,1)==1)
		{
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
					post_message=build_message(fd,cmd,message_len+7,post_message);
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
	int fd = 0;
	/*if((shmid_history = shmget(IPC_PRIVATE, sizeof(struct history)*100000, PERM)) == -1 )
	{
        printfLog(CAP_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));  
        exit(1);
    }*/
	if((fd=open_com_port("/dev/ttySP0"))<0)
	{
		printfLog(CAP_PROCESS"open_port cap error");
		return -1;
	}
	if(set_opt(fd,9600,8,'N',1)<0)
	{
		printfLog(CAP_PROCESS"set_opt cap error");
		return -1;
	}

	fpid=fork();
	if(fpid==0)
	{
		g_history.sensor_history[SENSOR_CO] 	= (struct nano *)shmat(g_history.shmid_history[SENSOR_CO], 0, 0);
		g_history.sensor_history[SENSOR_CO2] 	= (struct nano *)shmat(g_history.shmid_history[SENSOR_CO2], 0, 0);
		g_history.sensor_history[SENSOR_HCHO] 	= (struct nano *)shmat(g_history.shmid_history[SENSOR_HCHO], 0, 0);
		g_history.sensor_history[SENSOR_TEMP] 	= (struct nano *)shmat(g_history.shmid_history[SENSOR_TEMP], 0, 0);
		g_history.sensor_history[SENSOR_SHIDU] 	= (struct nano *)shmat(g_history.shmid_history[SENSOR_SHIDU], 0, 0);
		g_history.sensor_history[SENSOR_PM25] 	= (struct nano *)shmat(g_history.shmid_history[SENSOR_PM25], 0, 0);
		g_history.cnt[SENSOR_CO] 	= (long *)shmat(g_history.shmid_cnt[SENSOR_CO], 0, 0);
		g_history.cnt[SENSOR_CO2] 	= (long *)shmat(g_history.shmid_cnt[SENSOR_CO2], 0, 0);
		g_history.cnt[SENSOR_HCHO] 	= (long *)shmat(g_history.shmid_cnt[SENSOR_HCHO], 0, 0);
		g_history.cnt[SENSOR_TEMP] 	= (long *)shmat(g_history.shmid_cnt[SENSOR_TEMP], 0, 0);
		g_history.cnt[SENSOR_SHIDU] = (long *)shmat(g_history.shmid_cnt[SENSOR_SHIDU], 0, 0);
		g_history.cnt[SENSOR_PM25] 	= (long *)shmat(g_history.shmid_cnt[SENSOR_PM25], 0, 0);
		signal(SIGALRM, set_upload_flag);
		alarm(600);
		while(1)
			cap_board_mon(fd);
	}
	return 0;
}
