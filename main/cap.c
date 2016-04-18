#include "uart.h"
#include "netlib.h"
#include "log.h"
#define CAP_PROCESS "[CAP_PROCESS]"

struct share_memory *g_share_memory;
struct history *sensor_history;
key_t	shmid_history;
key_t	shmid_share_memory;
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
			set_upload_data(ID_CAP_CO,&(sensor_history[g_share_memory->cnt[SENSOR_CO]].co),
			g_share_memory->cnt[SENSOR_CO],data,date);
		
		data=doit_data(message,ID_CAP_CO2);
		if(data!=NULL)
			set_upload_data(ID_CAP_CO2,&(sensor_history[g_share_memory->cnt[SENSOR_CO2]].co2),
			g_share_memory->cnt[SENSOR_CO2],data,date);
		
		data=doit_data(message,ID_CAP_SHI_DU);
		if(data!=NULL)
			set_upload_data(ID_CAP_SHI_DU,&(sensor_history[g_share_memory->cnt[SENSOR_SHIDU]].shidu),
			g_share_memory->cnt[SENSOR_SHIDU],data,date);
		
		data=doit_data(message,ID_CAP_HCHO);
		if(data!=NULL)
			set_upload_data(ID_CAP_HCHO,&(sensor_history[g_share_memory->cnt[SENSOR_HCHO]].hcho),
			g_share_memory->cnt[SENSOR_HCHO],data,date);
		
		data=doit_data(message,ID_CAP_TEMPERATURE);
		if(data!=NULL)
			set_upload_data(ID_CAP_TEMPERATURE,&(sensor_history[g_share_memory->cnt[SENSOR_TEMP]].temp),
			g_share_memory->cnt[SENSOR_TEMP],data,date);
		
		data=doit_data(message,ID_CAP_PM_25);
		if(data!=NULL)
			set_upload_data(ID_CAP_PM_25,&(sensor_history[g_share_memory->cnt[SENSOR_PM25]].pm25),
			g_share_memory->cnt[SENSOR_PM25],data,date);
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
		*sent=1;
	}
	return json;
}
char *count_sensor_value(char cmd,char *json,int value)
{
	char *times;
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
		json=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
		json=add_item(json,ID_DEVICE_UID,g_uuid);
		json=add_item(json,ID_DEVICE_IP_ADDR,ip);
		json=add_item(json,ID_DEVICE_PORT,"9517");
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
	return json;
}
void update_dwin_real_value(char *id,int value)
{
	if(strncmp(id,ID_CAP_CO,strlen(ID_CAP_CO))==0)
	{
		write_dwin_real(VAR_DATE_TIME_1,value);
	}
	else if(strncmp(id,ID_CAP_CO2,strlen(ID_CAP_CO2))==0)
	{						
		write_dwin_real(VAR_DATE_TIME_2,value);
	}
	else if(strncmp(id,ID_CAP_HCHO,strlen(ID_CAP_HCHO))==0)
	{
		write_dwin_real(VAR_DATE_TIME_3,value);
	}
	else if(strncmp(id,ID_CAP_TEMPERATURE,strlen(ID_CAP_TEMPERATURE))==0)
	{
		write_dwin_real(VAR_DATE_TIME_4,value);
	}
	else if(strncmp(id,ID_CAP_SHI_DU,strlen(ID_CAP_SHI_DU))==0)
	{
		write_dwin_real(VAR_ALARM_TYPE_1,value);
	}
	else if(strncmp(id,ID_CAP_PM_25,strlen(ID_CAP_PM_25))==0)
	{
		write_dwin_real(VAR_ALARM_TYPE_2,value);
	}
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
				write(fd,server_time,13);
			}
			break;
			default:
			{
				/*get cap data*/
				if(memcmp(cmd+5,sensor_error,5)==0)
				{	
					//error got from cap board,check uninsert msg
					if(cmd[3]==atoi(ID_CAP_CO2) && !(g_share_memory->alarm[SENSOR_CO2]))
					{
						g_share_memory->alarm[SENSOR_CO2]|=ALARM_UNINSERT;
						g_share_memory->sent[SENSOR_CO2]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_CO) && !(g_share_memory->alarm[SENSOR_CO]))
					{
						sensor.alarm[SENSOR_CO]|=ALARM_UNINSERT;
						sensor.sent[SENSOR_CO]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_HCHO) && !(g_share_memory->alarm[SENSOR_HCHO]))
					{
						sensor.alarm[SENSOR_HCHO]|=ALARM_UNINSERT;
						sensor.sent[SENSOR_HCHO]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_SHI_DU) && !(g_share_memory->alarm[SENSOR_SHIDU]))
					{
						sensor.alarm[SENSOR_SHIDU]|=ALARM_UNINSERT;
						sensor.sent[SENSOR_SHIDU]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& !(g_share_memory->alarm[SENSOR_TEMP]))
					{
						sensor.alarm[SENSOR_TEMP]|=ALARM_UNINSERT;
						sensor.sent[SENSOR_TEMP]=0;
					}
					else if(cmd[3]==atoi(ID_CAP_PM_25)&& !(g_share_memory->alarm[SENSOR_PM25]))
					{
						sensor.alarm[SENSOR_PM25]|=ALARM_UNINSERT;
						sensor.sent[SENSOR_PM25]=0;
					}
					else 
						return message;
					//inform server
					warnning_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_WARNING);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_UID,g_uuid);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_IP_ADDR,ip);
					warnning_msg=add_item(warnning_msg,ID_DEVICE_PORT,"9517");
					warnning_msg=add_item(warnning_msg,ID_ALARM_TYPE,ID_ALERT_UNINSERT);
					memset(error,'\0',32);
					save_sensor_alarm_info();					
					return message;
				}
				else
				{	//normal data or beyond Min & Max data
					int value=0;
					//clear the uninsert alarm
					if(cmd[3]==atoi(ID_CAP_CO2) && (sensor.alarm[SENSOR_CO2]&ALARM_UNINSERT))
					{					
						clear_alarm(ID_CAP_CO2,ID_ALERT_UNINSERT);						
						sensor.alarm[SENSOR_CO2]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_CO) && (sensor.alarm[SENSOR_CO]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_CO,ID_ALERT_UNINSERT);
						sensor.alarm[SENSOR_CO]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_HCHO) && (sensor.alarm[SENSOR_HCHO]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_HCHO,ID_ALERT_UNINSERT);
						sensor.alarm[SENSOR_HCHO]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_SHI_DU) && (sensor.alarm[SENSOR_SHIDU]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_SHI_DU,ID_ALERT_UNINSERT);
						sensor.alarm[SENSOR_SHIDU]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_TEMPERATURE)&& (sensor.alarm[SENSOR_TEMP]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_TEMPERATURE,ID_ALERT_UNINSERT);
						sensor.alarm[SENSOR_TEMP]&=~ALARM_UNINSERT;	
						save_sensor_alarm_info();
					}
					if(cmd[3]==atoi(ID_CAP_PM_25)&& (sensor.alarm[SENSOR_PM25]&ALARM_UNINSERT))
					{
						clear_alarm(ID_CAP_PM_25,ID_ALERT_UNINSERT);
						sensor.alarm[SENSOR_PM25]&=~ALARM_UNINSERT;	
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
						int temp=1;
						for(i=0;i<cmd[7];i++)
							temp*=10;
						value=((cmd[5]<<8|cmd[6]))/temp;
					}	
					else
					{
							value=(cmd[5]<<8|cmd[6]);
					}
					//printfLog(CAP_PROCESS"Value %d\n",value);
					warnning_msg=count_sensor_value(cmd[3],warnning_msg,value);
					//real time update cap data
					update_dwin_real_value(id,cmd[5]<<8|cmd[6]);
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
		sensor_history = (struct history *)shmat(shmid_history,0, 0);
		g_share_memory = (struct share_memory *)shmat(shmid_share_memory,0, 0);
		signal(SIGALRM, set_upload_flag);
		alarm(600);
		while(1)
			cap_board_mon(fd);
	}
	return 0;
}
