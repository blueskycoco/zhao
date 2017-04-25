#include "cap.h"
#include "dwin.h"
#include "misc.h"
#include "netlib.h"
#include "xfer.h"
#include "history.h"
#include <stdbool.h>
int lcd_state=1;
char logged=0,g_index=0,interface_select=0,last_g_index=0,cur_index=0;
extern char g_uuid[256];
extern char ip[20];
char g_history_log[SENSOR_NO][1024];
int g_history_index[SENSOR_NO]={0};
#define LCD_PROCESS	"[LCD_PROCESS] "
typedef struct _data
{
    int wYear;
    int wMonth;
    int wDay;
	int wHour;
	int wMin;
}date;
//count dist from mf to time in minitutes
long dataD(date mf, date time);
void write_data(unsigned int Index,int data)
{
	//int i = 0;
	char cmd[]={0x5a,0xa5,0x05,0x82,0x00,0x00,0x00,0x00};
	cmd[4]=(Index&0xff00)>>8;cmd[5]=Index&0x00ff;
	cmd[6]=(data&0xff00)>>8;cmd[7]=data&0x00ff;
	//for(i = 0;i<sizeof(cmd);i++)
	//	printfLog(LCD_PROCESS"%02x ",cmd[i]);
	//printfLog(LCD_PROCESS"\n");
	write(g_share_memory->fd_lcd,cmd,8);
}
void write_data1(unsigned int Index,int data)
{
	//int i = 0;
	char cmd[]={0x5a,0xa5,0x04,0x82,0x00,0x00,0x00};
	cmd[4]=(Index&0xff00)>>8;cmd[5]=Index&0x00ff;
	cmd[7]=data&0x00ff;
	write(g_share_memory->fd_lcd,cmd,7);
}

void switch_pic(unsigned char Index)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x04,0x00};
	cmd[5]=Index;
	cur_index=Index;
	write(g_share_memory->fd_lcd,cmd,6);
}
void lcd_on(int page)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x01,0x40};
	switch_pic(page);
	usleep(200000);
	if(g_share_memory->black_lcd)
	{
		write(g_share_memory->fd_lcd,cmd,6);
	}
	lcd_state=1;
	printfLog(LCD_PROCESS"lcd on\n");
}
int read_sleeping_state()
{	
	char ch;
	int i=0;
	int timeout = 0;
	char cmd_read_tp_flag[]	={0x5a,0xa5,0x03,0x81,0x05,0x01};
	char cmd_clear_tp_flag[]={0x5a,0xa5,0x03,0x80,0x05,0x00};
	write(g_share_memory->fd_lcd,cmd_read_tp_flag,6);
	while(1)	
	{	
		if(read(g_share_memory->fd_lcd,&ch,1)==1)
		{
			if(i==6 && ch == 0x5a)
			{
				write(g_share_memory->fd_lcd,cmd_clear_tp_flag,6);
				lcd_on(g_index);					
				if(g_share_memory->sleep!=0)
					alarm(g_share_memory->sleep*60);
				else
					alarm(5*60);
				return 1;
			}
			if((i==0 && ch == 0x5a) ||
			   (i==1 && ch == 0xa5) ||
   		       (i==2 && ch == 0x04) ||
   		       (i==3 && ch == 0x81) ||
   		       (i==4 && ch == 0x05) ||
   		       (i==5 && ch == 0x01))
				i++;
			else
				return 0;
			
		}
		else
		{
			timeout++;
			if(timeout>100)
				return 0;
			usleep(1000);
		}
	}
	return 0;
}
void sleeping_page_show()
{
	int i = 0;
	int j = 0;
	char cmd_clear_tp_flag[]={0x5a,0xa5,0x03,0x80,0x05,0x00};
	write(g_share_memory->fd_lcd,cmd_clear_tp_flag,6);
	while (1) {
		for (i = SLEEPING_PAGE_1; i < SLEEPING_PAGE_10; i++) {
			switch_pic(i);
			//printfLog(LCD_PROCESS"switch tp pic %d\n",i);
			for (j = 0; j < 50; j++) {				
				if(read_sleeping_state())
					return ;
				usleep(100000);
			}
		}
	}
	printfLog(LCD_PROCESS"return sleeping_page_show\n");
}
void lcd_off(int a)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x01,0x00};
	if(g_share_memory->sleep!=0 && 
		cur_index!=VERIFY_PAGE &&
		cur_index!=XFER_SETTING_PAGE &&
		cur_index!=TIME_SETTING_PAGE &&
		cur_index!=LOGIN_PAGE &&
		cur_index!=UPLOADING_SETTING_PAGE &&
		cur_index!=100)
	{
		printfLog(LCD_PROCESS"lcd off\n");
		if(g_share_memory->black_lcd)
		{
			write(g_share_memory->fd_lcd,cmd,6);
			switch_pic(OFF_PAGE);
			lcd_state=0;
		}
		else
			sleeping_page_show();
	}
}

void write_string(unsigned int addr,char *data,int len)
{
	int i=0;
	char *cmd=(char *)malloc(len+6);
	cmd[0]=0x5a;cmd[1]=0xa5;cmd[2]=len+3;cmd[3]=0x82;
	cmd[4]=(addr&0xff00)>>8;cmd[5]=addr&0x00ff;
	for(i=0;i<len;i++)
		cmd[6+i]=data[i];
	//for(i=0;i<len+6;i++)
	//	printfLog(LCD_PROCESS"%02x ",cmd[i]);
	//printfLog(LCD_PROCESS"\n<len %d>\n",len);
	write(g_share_memory->fd_lcd,cmd,len+6);
	free(cmd);
}
void clear_point()
{
	char off0[]={0x5a,0xa5,0x15,0x82,0x08,0x31,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x00,0xb7,0x00,0xea,0x00,0xc2,0x00,0xe7,0x00,0xb7};

	char off1[]={0x5a,0xa5,0x15,0x82,0x08,0x3a,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x00,0xeb,0x00,0xea,0x00,0xf6,0x00,0xe7,0x00,0xeb};

	char off2[]={0x5a,0xa5,0x15,0x82,0x08,0x43,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x01,0x1e,0x00,0xea,0x01,0x2a,0x00,0xe7,0x01,0x1e};

	char off3[]={0x5a,0xa5,0x15,0x82,0x08,0x4c,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x01,0x51,0x00,0xea,0x01,0x5e,0x00,0xe7,0x01,0x51};

	char off4[]={0x5a,0xa5,0x15,0x82,0x08,0x55,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x01,0x86,0x00,0xea,0x01,0x92,0x00,0xe7,0x01,0x86};

	char off5[]={0x5a,0xa5,0x15,0x82,0x08,0x5e,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x01,0xba,0x00,0xea,0x01,0xc6,0x00,0xe7,0x01,0xba};

	char off6[]={0x5a,0xa5,0x15,0x82,0x08,0x67,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x01,0xed,0x00,0xea,0x01,0xf9,0x00,0xe7,0x01,0xed};

	char off7[]={0x5a,0xa5,0x15,0x82,0x08,0x70,0x00,0x06,0x00,0x01,0x00,0x13,
				0x00,0xe7,0x02,0x21,0x00,0xea,0x02,0x2c,0x00,0xe7,0x02,0x21};
	write(g_share_memory->fd_lcd,off0,sizeof(off0));
	write(g_share_memory->fd_lcd,off1,sizeof(off1));
	write(g_share_memory->fd_lcd,off2,sizeof(off2));
	write(g_share_memory->fd_lcd,off3,sizeof(off3));
	write(g_share_memory->fd_lcd,off4,sizeof(off4));
	write(g_share_memory->fd_lcd,off5,sizeof(off5));
	write(g_share_memory->fd_lcd,off6,sizeof(off6));
	write(g_share_memory->fd_lcd,off7,sizeof(off7));
	clear_buf(ADDR_XIUZHENG,10);
	clear_buf(ADDR_JIAOZHUN_REAL,10);
}

void clear_buf(int addr,int len)
{
	char *tmp=(char *)malloc(len);
	memset(tmp,0,len);
	write_string(addr,tmp,len);
	free(tmp);
}
void draw_curve(int *data,int len)
{
	int i;
	char *cmd=(char *)malloc(len*2+5);
	cmd[0]=0x5a;cmd[1]=0xa5;cmd[2]=len*2+2;cmd[3]=0x84;cmd[4]=0x01;
	for(i=len-1;i>=0;i--)
	{
		cmd[5+2*i]=(data[i]&0xff00)>>8;
		cmd[6+2*i]=data[i]&0x00ff;
	}
	//for(i=0;i<len*2+5;i++)
	//	printfLog(LCD_PROCESS"%02x ",cmd[i]);
	//printfLog(LCD_PROCESS"\n<len %d>\n",len);
	write(g_share_memory->fd_lcd,cmd,len*2+5);
	free(cmd);
}
void clear_curve()
{
	char cmd[]={0x5a,0xa5,0x02,0xeb,0x5a};
	write(g_share_memory->fd_lcd,cmd,5);
}
void show_sensor_network()
{
	int pic=0;
	ping_server();
	printfLog(LCD_PROCESS"sensor co %d, co2 %d, hcho %d, shidu %d, temp %d, pm25 %d,noise %d, wind %d, o3 %d, press %d, pm10 %d, tvoc %d\nnetwork_state %d\n",
		g_share_memory->sensor_state[SENSOR_CO],g_share_memory->sensor_state[SENSOR_CO2],g_share_memory->sensor_state[SENSOR_HCHO],
		g_share_memory->sensor_state[SENSOR_SHIDU],g_share_memory->sensor_state[SENSOR_TEMP],g_share_memory->sensor_state[SENSOR_PM25],		
		g_share_memory->sensor_state[SENSOR_NOISE],g_share_memory->sensor_state[SENSOR_WIND],g_share_memory->sensor_state[SENSOR_O3],
		g_share_memory->sensor_state[SENSOR_PRESS],g_share_memory->sensor_state[SENSOR_PM10],g_share_memory->sensor_state[SENSOR_TVOC],
		g_share_memory->network_state);
	
	if((g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25]
		||g_share_memory->sensor_state[SENSOR_NOISE] || g_share_memory->sensor_state[SENSOR_WIND] || g_share_memory->sensor_state[SENSOR_O3] 
		|| g_share_memory->sensor_state[SENSOR_PRESS] || g_share_memory->sensor_state[SENSOR_PM10] || g_share_memory->sensor_state[SENSOR_TVOC])
		&&(!g_share_memory->network_state))
		pic=STATE_PAGE_I_I;
	else if((g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25]
		||g_share_memory->sensor_state[SENSOR_NOISE] || g_share_memory->sensor_state[SENSOR_WIND] || g_share_memory->sensor_state[SENSOR_O3] 
		|| g_share_memory->sensor_state[SENSOR_PRESS] || g_share_memory->sensor_state[SENSOR_PM10] || g_share_memory->sensor_state[SENSOR_TVOC])
		&&(g_share_memory->network_state))
		pic=STATE_PAGE_I_O;
	else if(!(g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25]
		||g_share_memory->sensor_state[SENSOR_NOISE] || g_share_memory->sensor_state[SENSOR_WIND] || g_share_memory->sensor_state[SENSOR_O3] 
		|| g_share_memory->sensor_state[SENSOR_PRESS] || g_share_memory->sensor_state[SENSOR_PM10] || g_share_memory->sensor_state[SENSOR_TVOC])
		&&(g_share_memory->network_state))
		pic=STATE_PAGE_O_O;
	else if(!(g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25]
		||g_share_memory->sensor_state[SENSOR_NOISE] || g_share_memory->sensor_state[SENSOR_WIND] || g_share_memory->sensor_state[SENSOR_O3] 
		|| g_share_memory->sensor_state[SENSOR_PRESS] || g_share_memory->sensor_state[SENSOR_PM10] || g_share_memory->sensor_state[SENSOR_TVOC])
		&&(!g_share_memory->network_state))
		pic=STATE_PAGE_O_I;
	switch_pic(pic);
	g_index=pic;
}
int count_all_pages(struct nano *item,int cnt)
{
	char time1[256]={0};
	char time2[256]={0};
	int i=0,j=0;
	int pages=0;
	for(i=0;i<cnt;)
	{
		strcpy(time1,item[i].time);
		for(j=i+1;j<i+7;j++)
		{
			strcpy(time2,item[j].time);
			//printfLog("time1 %s, time2 %s \n",time1,time2);
			if(strncmp(time1,time2,11)!=0)
			{
				break;
			}
		}
		pages++;
		i=j;
		//printfLog("pages %d, i %d \n",pages,i);
	}
	return pages;
}
char *remove_p(char *buf)
{
	int i=0,j=0;
	char *out=(char *)malloc(strlen(buf)+1);
	memset(out,0,strlen(buf)+1);
	while(buf[i]!='\0')
	{
		if(buf[i]!='.')
			out[j++]=buf[i];
		i++;
	}
	//printfLog(LCD_PROCESS"out is %s\n",out);
	return out;
}
int show_history(char *id,int offset,int page)
{	
	char tmp[4]={0};
	char *out=NULL;
	int offs = 0;
	char time1[255]={0};
	char time2[255]={0};
	if(strncmp(id,ID_CAP_CHOU_YANG,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"o3_cnt %d\n",g_share_memory->cnt[SENSOR_O3]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-1].time,4);
		write_data(ADDR_LIST_O3_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-1].time+5,2);
		write_data(ADDR_LIST_O3_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-1].time+8,2);
		write_data(ADDR_LIST_O3_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_O3]-offset-1)>=0)
		{//6:2
			write_data(ADDR_O3_PAGE_N,page+1);
			write_data(ADDR_O3_PAGE_ALL,count_all_pages(sensor_history.o3,g_share_memory->cnt[SENSOR_O3]));
			memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-1].time+11,2);
			write_data(ADDR_O3_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-1].time+14,2);
			write_data(ADDR_O3_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-1].data);
			write_data(ADDR_O3_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_O3]-offset-2)>=0){
			strcpy(time2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_O3_1_1,0x00);
				write_data(ADDR_HISTORY_O3_1_2,0x00);
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-2].time+11,2);
				write_data(ADDR_O3_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-2].time+14,2);
				write_data(ADDR_O3_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-2].data);
				write_data(ADDR_O3_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_O3_1_1,0x01);
				write_data(ADDR_HISTORY_O3_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_O3]-offset-3)>=0){
			strcpy(time2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_O3_2_1,0x00);
				write_data(ADDR_HISTORY_O3_2_2,0x00);
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-3].time+11,2);
				write_data(ADDR_O3_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-3].time+14,2);
				write_data(ADDR_O3_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-3].data);
				write_data(ADDR_O3_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_O3_2_1,0x01);
				write_data(ADDR_HISTORY_O3_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_O3]-offset-4)>=0){
			strcpy(time2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_O3_3_1,0x00);
				write_data(ADDR_HISTORY_O3_3_2,0x00);
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-4].time+11,2);
				write_data(ADDR_O3_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-4].time+14,2);
				write_data(ADDR_O3_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-4].data);
				write_data(ADDR_O3_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_O3_3_1,0x01);
				write_data(ADDR_HISTORY_O3_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_O3]-offset-5)>=0){
			strcpy(time2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_O3_4_1,0x00);
				write_data(ADDR_HISTORY_O3_4_2,0x00);
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-5].time+11,2);
				write_data(ADDR_O3_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-5].time+14,2);
				write_data(ADDR_O3_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-5].data);
				write_data(ADDR_O3_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_O3_4_1,0x01);
				write_data(ADDR_HISTORY_O3_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_O3]-offset-6)>=0){
			strcpy(time2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_O3_5_1,0x00);
				write_data(ADDR_HISTORY_O3_5_2,0x00);
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-6].time+11,2);
				write_data(ADDR_O3_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-6].time+14,2);
				write_data(ADDR_O3_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-6].data);
				write_data(ADDR_O3_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_O3_5_1,0x01);
				write_data(ADDR_HISTORY_O3_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_O3]-offset-7)>=0){
			strcpy(time2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_O3_6_1,0x00);
				write_data(ADDR_HISTORY_O3_6_2,0x00);
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-7].time+11,2);
				write_data(ADDR_O3_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-7].time+14,2);
				write_data(ADDR_O3_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-offset-7].data);
				write_data(ADDR_O3_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_O3_6_1,0x01);
				write_data(ADDR_HISTORY_O3_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_TVOC,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"tvoc_cnt %d\n",g_share_memory->cnt[SENSOR_TVOC]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-1].time,4);
		write_data(ADDR_LIST_TVOC_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-1].time+5,2);
		write_data(ADDR_LIST_TVOC_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-1].time+8,2);
		write_data(ADDR_LIST_TVOC_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_TVOC]-offset-1)>=0)
		{//6:2
			write_data(ADDR_TVOC_PAGE_N,page+1);
			write_data(ADDR_TVOC_PAGE_ALL,count_all_pages(sensor_history.tvoc,g_share_memory->cnt[SENSOR_TVOC]));
			memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-1].time+11,2);
			write_data(ADDR_TVOC_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-1].time+14,2);
			write_data(ADDR_TVOC_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-1].data);
			write_data(ADDR_TVOC_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_TVOC]-offset-2)>=0){
			strcpy(time2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TVOC_1_1,0x00);
				write_data(ADDR_HISTORY_TVOC_1_2,0x00);
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-2].time+11,2);
				write_data(ADDR_TVOC_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-2].time+14,2);
				write_data(ADDR_TVOC_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-2].data);
				write_data(ADDR_TVOC_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TVOC_1_1,0x01);
				write_data(ADDR_HISTORY_TVOC_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TVOC]-offset-3)>=0){
			strcpy(time2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TVOC_2_1,0x00);
				write_data(ADDR_HISTORY_TVOC_2_2,0x00);
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-3].time+11,2);
				write_data(ADDR_TVOC_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-3].time+14,2);
				write_data(ADDR_TVOC_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-3].data);
				write_data(ADDR_TVOC_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TVOC_2_1,0x01);
				write_data(ADDR_HISTORY_TVOC_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TVOC]-offset-4)>=0){
			strcpy(time2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TVOC_3_1,0x00);
				write_data(ADDR_HISTORY_TVOC_3_2,0x00);
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-4].time+11,2);
				write_data(ADDR_TVOC_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-4].time+14,2);
				write_data(ADDR_TVOC_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-4].data);
				write_data(ADDR_TVOC_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TVOC_3_1,0x01);
				write_data(ADDR_HISTORY_TVOC_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TVOC]-offset-5)>=0){
			strcpy(time2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TVOC_4_1,0x00);
				write_data(ADDR_HISTORY_TVOC_4_2,0x00);
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-5].time+11,2);
				write_data(ADDR_TVOC_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-5].time+14,2);
				write_data(ADDR_TVOC_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-5].data);
				write_data(ADDR_TVOC_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TVOC_4_1,0x01);
				write_data(ADDR_HISTORY_TVOC_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TVOC]-offset-6)>=0){
			strcpy(time2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TVOC_5_1,0x00);
				write_data(ADDR_HISTORY_TVOC_5_2,0x00);
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-6].time+11,2);
				write_data(ADDR_TVOC_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-6].time+14,2);
				write_data(ADDR_TVOC_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-6].data);
				write_data(ADDR_TVOC_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TVOC_5_1,0x01);
				write_data(ADDR_HISTORY_TVOC_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TVOC]-offset-7)>=0){
			strcpy(time2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TVOC_6_1,0x00);
				write_data(ADDR_HISTORY_TVOC_6_2,0x00);
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-7].time+11,2);
				write_data(ADDR_TVOC_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-7].time+14,2);
				write_data(ADDR_TVOC_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-offset-7].data);
				write_data(ADDR_TVOC_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TVOC_6_1,0x01);
				write_data(ADDR_HISTORY_TVOC_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_QI_YA,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"press_cnt %d\n",g_share_memory->cnt[SENSOR_PRESS]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-1].time,4);
		write_data(ADDR_LIST_PRESS_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-1].time+5,2);
		write_data(ADDR_LIST_PRESS_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-1].time+8,2);
		write_data(ADDR_LIST_PRESS_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_PRESS]-offset-1)>=0)
		{//6:2
			write_data(ADDR_PRESS_PAGE_N,page+1);
			write_data(ADDR_PRESS_PAGE_ALL,count_all_pages(sensor_history.press,g_share_memory->cnt[SENSOR_PRESS]));
			memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-1].time+11,2);
			write_data(ADDR_PRESS_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-1].time+14,2);
			write_data(ADDR_PRESS_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-1].data);
			write_data(ADDR_PRESS_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_PRESS]-offset-2)>=0){
			strcpy(time2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PRESS_1_1,0x00);
				write_data(ADDR_HISTORY_PRESS_1_2,0x00);
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-2].time+11,2);
				write_data(ADDR_PRESS_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-2].time+14,2);
				write_data(ADDR_PRESS_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-2].data);
				write_data(ADDR_PRESS_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PRESS_1_1,0x01);
				write_data(ADDR_HISTORY_PRESS_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PRESS]-offset-3)>=0){
			strcpy(time2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PRESS_2_1,0x00);
				write_data(ADDR_HISTORY_PRESS_2_2,0x00);
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-3].time+11,2);
				write_data(ADDR_PRESS_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-3].time+14,2);
				write_data(ADDR_PRESS_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-3].data);
				write_data(ADDR_PRESS_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PRESS_2_1,0x01);
				write_data(ADDR_HISTORY_PRESS_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PRESS]-offset-4)>=0){
			strcpy(time2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PRESS_3_1,0x00);
				write_data(ADDR_HISTORY_PRESS_3_2,0x00);
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-4].time+11,2);
				write_data(ADDR_PRESS_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-4].time+14,2);
				write_data(ADDR_PRESS_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-4].data);
				write_data(ADDR_PRESS_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PRESS_3_1,0x01);
				write_data(ADDR_HISTORY_PRESS_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PRESS]-offset-5)>=0){
			strcpy(time2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PRESS_4_1,0x00);
				write_data(ADDR_HISTORY_PRESS_4_2,0x00);
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-5].time+11,2);
				write_data(ADDR_PRESS_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-5].time+14,2);
				write_data(ADDR_PRESS_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-5].data);
				write_data(ADDR_PRESS_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PRESS_4_1,0x01);
				write_data(ADDR_HISTORY_PRESS_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PRESS]-offset-6)>=0){
			strcpy(time2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PRESS_5_1,0x00);
				write_data(ADDR_HISTORY_PRESS_5_2,0x00);
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-6].time+11,2);
				write_data(ADDR_PRESS_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-6].time+14,2);
				write_data(ADDR_PRESS_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-6].data);
				write_data(ADDR_PRESS_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PRESS_5_1,0x01);
				write_data(ADDR_HISTORY_PRESS_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PRESS]-offset-7)>=0){
			strcpy(time2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PRESS_6_1,0x00);
				write_data(ADDR_HISTORY_PRESS_6_2,0x00);
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-7].time+11,2);
				write_data(ADDR_PRESS_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-7].time+14,2);
				write_data(ADDR_PRESS_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-offset-7].data);
				write_data(ADDR_PRESS_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PRESS_6_1,0x01);
				write_data(ADDR_HISTORY_PRESS_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_BUZZY,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"noise_cnt %d\n",g_share_memory->cnt[SENSOR_NOISE]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-1].time,4);
		write_data(ADDR_LIST_NOISE_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-1].time+5,2);
		write_data(ADDR_LIST_NOISE_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-1].time+8,2);
		write_data(ADDR_LIST_NOISE_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_NOISE]-offset-1)>=0)
		{//6:2
			write_data(ADDR_NOISE_PAGE_N,page+1);
			write_data(ADDR_NOISE_PAGE_ALL,count_all_pages(sensor_history.noise,g_share_memory->cnt[SENSOR_NOISE]));
			memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-1].time+11,2);
			write_data(ADDR_NOISE_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-1].time+14,2);
			write_data(ADDR_NOISE_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-1].data);
			write_data(ADDR_NOISE_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_NOISE]-offset-2)>=0){
			strcpy(time2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_NOISE_1_1,0x00);
				write_data(ADDR_HISTORY_NOISE_1_2,0x00);
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-2].time+11,2);
				write_data(ADDR_NOISE_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-2].time+14,2);
				write_data(ADDR_NOISE_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-2].data);
				write_data(ADDR_NOISE_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_NOISE_1_1,0x01);
				write_data(ADDR_HISTORY_NOISE_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_NOISE]-offset-3)>=0){
			strcpy(time2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_NOISE_2_1,0x00);
				write_data(ADDR_HISTORY_NOISE_2_2,0x00);
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-3].time+11,2);
				write_data(ADDR_NOISE_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-3].time+14,2);
				write_data(ADDR_NOISE_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-3].data);
				write_data(ADDR_NOISE_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_NOISE_2_1,0x01);
				write_data(ADDR_HISTORY_NOISE_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_NOISE]-offset-4)>=0){
			strcpy(time2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_NOISE_3_1,0x00);
				write_data(ADDR_HISTORY_NOISE_3_2,0x00);
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-4].time+11,2);
				write_data(ADDR_NOISE_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-4].time+14,2);
				write_data(ADDR_NOISE_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-4].data);
				write_data(ADDR_NOISE_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_NOISE_3_1,0x01);
				write_data(ADDR_HISTORY_NOISE_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_NOISE]-offset-5)>=0){
			strcpy(time2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_NOISE_4_1,0x00);
				write_data(ADDR_HISTORY_NOISE_4_2,0x00);
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-5].time+11,2);
				write_data(ADDR_NOISE_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-5].time+14,2);
				write_data(ADDR_NOISE_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-5].data);
				write_data(ADDR_NOISE_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_NOISE_4_1,0x01);
				write_data(ADDR_HISTORY_NOISE_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_NOISE]-offset-6)>=0){
			strcpy(time2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_NOISE_5_1,0x00);
				write_data(ADDR_HISTORY_NOISE_5_2,0x00);
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-6].time+11,2);
				write_data(ADDR_NOISE_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-6].time+14,2);
				write_data(ADDR_NOISE_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-6].data);
				write_data(ADDR_NOISE_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_NOISE_5_1,0x01);
				write_data(ADDR_HISTORY_NOISE_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_NOISE]-offset-7)>=0){
			strcpy(time2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_NOISE_6_1,0x00);
				write_data(ADDR_HISTORY_NOISE_6_2,0x00);
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-7].time+11,2);
				write_data(ADDR_NOISE_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-7].time+14,2);
				write_data(ADDR_NOISE_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-offset-7].data);
				write_data(ADDR_NOISE_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_NOISE_6_1,0x01);
				write_data(ADDR_HISTORY_NOISE_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_FENG_SU,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"wind_cnt %d\n",g_share_memory->cnt[SENSOR_WIND]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-1].time,4);
		write_data(ADDR_LIST_WIND_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-1].time+5,2);
		write_data(ADDR_LIST_WIND_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-1].time+8,2);
		write_data(ADDR_LIST_WIND_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_WIND]-offset-1)>=0)
		{//6:2
			write_data(ADDR_WIND_PAGE_N,page+1);
			write_data(ADDR_WIND_PAGE_ALL,count_all_pages(sensor_history.wind,g_share_memory->cnt[SENSOR_WIND]));
			memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-1].time+11,2);
			write_data(ADDR_WIND_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-1].time+14,2);
			write_data(ADDR_WIND_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-1].data);
			write_data(ADDR_WIND_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_WIND]-offset-2)>=0){
			strcpy(time2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_WIND_1_1,0x00);
				write_data(ADDR_HISTORY_WIND_1_2,0x00);
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-2].time+11,2);
				write_data(ADDR_WIND_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-2].time+14,2);
				write_data(ADDR_WIND_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-2].data);
				write_data(ADDR_WIND_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_WIND_1_1,0x01);
				write_data(ADDR_HISTORY_WIND_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_WIND]-offset-3)>=0){
			strcpy(time2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_WIND_2_1,0x00);
				write_data(ADDR_HISTORY_WIND_2_2,0x00);
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-3].time+11,2);
				write_data(ADDR_WIND_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-3].time+14,2);
				write_data(ADDR_WIND_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-3].data);
				write_data(ADDR_WIND_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_WIND_2_1,0x01);
				write_data(ADDR_HISTORY_WIND_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_WIND]-offset-4)>=0){
			strcpy(time2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_WIND_3_1,0x00);
				write_data(ADDR_HISTORY_WIND_3_2,0x00);
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-4].time+11,2);
				write_data(ADDR_WIND_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-4].time+14,2);
				write_data(ADDR_WIND_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-4].data);
				write_data(ADDR_WIND_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_WIND_3_1,0x01);
				write_data(ADDR_HISTORY_WIND_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_WIND]-offset-5)>=0){
			strcpy(time2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_WIND_4_1,0x00);
				write_data(ADDR_HISTORY_WIND_4_2,0x00);
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-5].time+11,2);
				write_data(ADDR_WIND_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-5].time+14,2);
				write_data(ADDR_WIND_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-5].data);
				write_data(ADDR_WIND_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_WIND_4_1,0x01);
				write_data(ADDR_HISTORY_WIND_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_WIND]-offset-6)>=0){
			strcpy(time2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_WIND_5_1,0x00);
				write_data(ADDR_HISTORY_WIND_5_2,0x00);
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-6].time+11,2);
				write_data(ADDR_WIND_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-6].time+14,2);
				write_data(ADDR_WIND_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-6].data);
				write_data(ADDR_WIND_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_WIND_5_1,0x01);
				write_data(ADDR_HISTORY_WIND_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_WIND]-offset-7)>=0){
			strcpy(time2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_WIND_6_1,0x00);
				write_data(ADDR_HISTORY_WIND_6_2,0x00);
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-7].time+11,2);
				write_data(ADDR_WIND_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-7].time+14,2);
				write_data(ADDR_WIND_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-offset-7].data);
				write_data(ADDR_WIND_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_WIND_6_1,0x01);
				write_data(ADDR_HISTORY_WIND_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"shidu_cnt %d\n",g_share_memory->cnt[SENSOR_SHIDU]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time,4);
		write_data(ADDR_LIST_SHIDU_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time+5,2);
		write_data(ADDR_LIST_SHIDU_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time+8,2);
		write_data(ADDR_LIST_SHIDU_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_SHIDU]-offset-1)>=0)
		{//6:2
			write_data(ADDR_SHIDU_PAGE_N,page+1);
			write_data(ADDR_SHIDU_PAGE_ALL,count_all_pages(sensor_history.shidu,g_share_memory->cnt[SENSOR_SHIDU]));
			memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time+11,2);
			write_data(ADDR_SHIDU_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time+14,2);
			write_data(ADDR_SHIDU_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].data);
			write_data(ADDR_SHIDU_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_SHIDU]-offset-2)>=0){
			strcpy(time2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_SHIDU_1_1,0x00);
				write_data(ADDR_HISTORY_SHIDU_1_2,0x00);
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].time+11,2);
				write_data(ADDR_SHIDU_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].time+14,2);
				write_data(ADDR_SHIDU_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].data);
				write_data(ADDR_SHIDU_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_SHIDU_1_1,0x01);
				write_data(ADDR_HISTORY_SHIDU_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_SHIDU]-offset-3)>=0){
			strcpy(time2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_SHIDU_2_1,0x00);
				write_data(ADDR_HISTORY_SHIDU_2_2,0x00);
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].time+11,2);
				write_data(ADDR_SHIDU_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].time+14,2);
				write_data(ADDR_SHIDU_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].data);
				write_data(ADDR_SHIDU_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_SHIDU_2_1,0x01);
				write_data(ADDR_HISTORY_SHIDU_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_SHIDU]-offset-4)>=0){
			strcpy(time2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_SHIDU_3_1,0x00);
				write_data(ADDR_HISTORY_SHIDU_3_2,0x00);
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].time+11,2);
				write_data(ADDR_SHIDU_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].time+14,2);
				write_data(ADDR_SHIDU_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].data);
				write_data(ADDR_SHIDU_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_SHIDU_3_1,0x01);
				write_data(ADDR_HISTORY_SHIDU_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_SHIDU]-offset-5)>=0){
			strcpy(time2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_SHIDU_4_1,0x00);
				write_data(ADDR_HISTORY_SHIDU_4_2,0x00);
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].time+11,2);
				write_data(ADDR_SHIDU_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].time+14,2);
				write_data(ADDR_SHIDU_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].data);
				write_data(ADDR_SHIDU_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_SHIDU_4_1,0x01);
				write_data(ADDR_HISTORY_SHIDU_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_SHIDU]-offset-6)>=0){
			strcpy(time2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_SHIDU_5_1,0x00);
				write_data(ADDR_HISTORY_SHIDU_5_2,0x00);
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].time+11,2);
				write_data(ADDR_SHIDU_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].time+14,2);
				write_data(ADDR_SHIDU_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].data);
				write_data(ADDR_SHIDU_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_SHIDU_5_1,0x01);
				write_data(ADDR_HISTORY_SHIDU_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_SHIDU]-offset-7)>=0){
			strcpy(time2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_SHIDU_6_1,0x00);
				write_data(ADDR_HISTORY_SHIDU_6_2,0x00);
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].time+11,2);
				write_data(ADDR_SHIDU_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].time+14,2);
				write_data(ADDR_SHIDU_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].data);
				write_data(ADDR_SHIDU_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_SHIDU_6_1,0x01);
				write_data(ADDR_HISTORY_SHIDU_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"temp_cnt %d\n",g_share_memory->cnt[SENSOR_TEMP]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time,4);
		write_data(ADDR_LIST_TEMP_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time+5,2);
		write_data(ADDR_LIST_TEMP_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time+8,2);
		write_data(ADDR_LIST_TEMP_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_TEMP]-offset-1)>=0)
		{//6:2
			write_data(ADDR_TEMP_PAGE_N,page+1);
			write_data(ADDR_TEMP_PAGE_ALL,count_all_pages(sensor_history.temp,g_share_memory->cnt[SENSOR_TEMP]));
			memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time+11,2);
			write_data(ADDR_TEMP_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time+14,2);
			write_data(ADDR_TEMP_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].data);
			write_data(ADDR_TEMP_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_TEMP]-offset-2)>=0){
			strcpy(time2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TEMP_1_1,0x00);
				write_data(ADDR_HISTORY_TEMP_1_2,0x00);
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].time+11,2);
				write_data(ADDR_TEMP_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].time+14,2);
				write_data(ADDR_TEMP_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].data);
				write_data(ADDR_TEMP_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TEMP_1_1,0x01);
				write_data(ADDR_HISTORY_TEMP_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TEMP]-offset-3)>=0){
			strcpy(time2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TEMP_2_1,0x00);
				write_data(ADDR_HISTORY_TEMP_2_2,0x00);
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].time+11,2);
				write_data(ADDR_TEMP_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].time+14,2);
				write_data(ADDR_TEMP_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].data);
				write_data(ADDR_TEMP_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TEMP_2_1,0x01);
				write_data(ADDR_HISTORY_TEMP_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TEMP]-offset-4)>=0){
			strcpy(time2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TEMP_3_1,0x00);
				write_data(ADDR_HISTORY_TEMP_3_2,0x00);
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].time+11,2);
				write_data(ADDR_TEMP_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].time+14,2);
				write_data(ADDR_TEMP_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].data);
				write_data(ADDR_TEMP_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TEMP_3_1,0x01);
				write_data(ADDR_HISTORY_TEMP_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TEMP]-offset-5)>=0){
			strcpy(time2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TEMP_4_1,0x00);
				write_data(ADDR_HISTORY_TEMP_4_2,0x00);
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].time+11,2);
				write_data(ADDR_TEMP_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].time+14,2);
				write_data(ADDR_TEMP_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].data);
				write_data(ADDR_TEMP_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TEMP_4_1,0x01);
				write_data(ADDR_HISTORY_TEMP_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TEMP]-offset-6)>=0){
			strcpy(time2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TEMP_5_1,0x00);
				write_data(ADDR_HISTORY_TEMP_5_2,0x00);
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].time+11,2);
				write_data(ADDR_TEMP_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].time+14,2);
				write_data(ADDR_TEMP_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].data);
				write_data(ADDR_TEMP_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TEMP_5_1,0x01);
				write_data(ADDR_HISTORY_TEMP_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_TEMP]-offset-7)>=0){
			strcpy(time2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_TEMP_6_1,0x00);
				write_data(ADDR_HISTORY_TEMP_6_2,0x00);
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].time+11,2);
				write_data(ADDR_TEMP_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].time+14,2);
				write_data(ADDR_TEMP_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].data);
				write_data(ADDR_TEMP_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_TEMP_6_1,0x01);
				write_data(ADDR_HISTORY_TEMP_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"co2_cnt %d\n",g_share_memory->cnt[SENSOR_CO2]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time,4);
		write_data(ADDR_LIST_CO2_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time+5,2);
		write_data(ADDR_LIST_CO2_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time+8,2);
		write_data(ADDR_LIST_CO2_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_CO2]-offset-1)>=0)
		{//6:2
			write_data(ADDR_CO2_PAGE_N,page+1);
			write_data(ADDR_CO2_PAGE_ALL,count_all_pages(sensor_history.co2,g_share_memory->cnt[SENSOR_CO2]));
			memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time+11,2);
			write_data(ADDR_CO2_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time+14,2);
			write_data(ADDR_CO2_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].data);
			write_data(ADDR_CO2_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_CO2]-offset-2)>=0){
			strcpy(time2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO2_1_1,0x00);
				write_data(ADDR_HISTORY_CO2_1_2,0x00);
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].time+11,2);
				write_data(ADDR_CO2_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].time+14,2);
				write_data(ADDR_CO2_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].data);
				write_data(ADDR_CO2_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO2_1_1,0x01);
				write_data(ADDR_HISTORY_CO2_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO2]-offset-3)>=0){
			strcpy(time2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO2_2_1,0x00);
				write_data(ADDR_HISTORY_CO2_2_2,0x00);
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].time+11,2);
				write_data(ADDR_CO2_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].time+14,2);
				write_data(ADDR_CO2_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].data);
				write_data(ADDR_CO2_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO2_2_1,0x01);
				write_data(ADDR_HISTORY_CO2_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO2]-offset-4)>=0){
			strcpy(time2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO2_3_1,0x00);
				write_data(ADDR_HISTORY_CO2_3_2,0x00);
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].time+11,2);
				write_data(ADDR_CO2_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].time+14,2);
				write_data(ADDR_CO2_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].data);
				write_data(ADDR_CO2_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO2_3_1,0x01);
				write_data(ADDR_HISTORY_CO2_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO2]-offset-5)>=0){
			strcpy(time2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO2_4_1,0x00);
				write_data(ADDR_HISTORY_CO2_4_2,0x00);
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].time+11,2);
				write_data(ADDR_CO2_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].time+14,2);
				write_data(ADDR_CO2_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].data);
				write_data(ADDR_CO2_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO2_4_1,0x01);
				write_data(ADDR_HISTORY_CO2_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO2]-offset-6)>=0){
			strcpy(time2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO2_5_1,0x00);
				write_data(ADDR_HISTORY_CO2_5_2,0x00);
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].time+11,2);
				write_data(ADDR_CO2_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].time+14,2);
				write_data(ADDR_CO2_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].data);
				write_data(ADDR_CO2_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO2_5_1,0x01);
				write_data(ADDR_HISTORY_CO2_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO2]-offset-7)>=0){
			strcpy(time2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO2_6_1,0x00);
				write_data(ADDR_HISTORY_CO2_6_2,0x00);
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].time+11,2);
				write_data(ADDR_CO2_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].time+14,2);
				write_data(ADDR_CO2_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].data);
				write_data(ADDR_CO2_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO2_6_1,0x01);
				write_data(ADDR_HISTORY_CO2_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_CO,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"co_cnt %d\n",g_share_memory->cnt[SENSOR_CO]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time,4);
		write_data(ADDR_LIST_CO_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time+5,2);
		write_data(ADDR_LIST_CO_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time+8,2);
		write_data(ADDR_LIST_CO_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_CO]-offset-1)>=0)
		{//6:2
			write_data(ADDR_CO_PAGE_N,page+1);
			write_data(ADDR_CO_PAGE_ALL,count_all_pages(sensor_history.co,g_share_memory->cnt[SENSOR_CO]));
			memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time+11,2);
			write_data(ADDR_CO_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time+14,2);
			write_data(ADDR_CO_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].data);
			write_data(ADDR_CO_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_CO]-offset-2)>=0){
			strcpy(time2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO_1_1,0x00);
				write_data(ADDR_HISTORY_CO_1_2,0x00);
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].time+11,2);
				write_data(ADDR_CO_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].time+14,2);
				write_data(ADDR_CO_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].data);
				write_data(ADDR_CO_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO_1_1,0x01);
				write_data(ADDR_HISTORY_CO_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO]-offset-3)>=0){
			strcpy(time2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO_2_1,0x00);
				write_data(ADDR_HISTORY_CO_2_2,0x00);
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].time+11,2);
				write_data(ADDR_CO_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].time+14,2);
				write_data(ADDR_CO_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].data);
				write_data(ADDR_CO_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO_2_1,0x01);
				write_data(ADDR_HISTORY_CO_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO]-offset-4)>=0){
			strcpy(time2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO_3_1,0x00);
				write_data(ADDR_HISTORY_CO_3_2,0x00);
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].time+11,2);
				write_data(ADDR_CO_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].time+14,2);
				write_data(ADDR_CO_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].data);
				write_data(ADDR_CO_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO_3_1,0x01);
				write_data(ADDR_HISTORY_CO_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO]-offset-5)>=0){
			strcpy(time2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO_4_1,0x00);
				write_data(ADDR_HISTORY_CO_4_2,0x00);
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].time+11,2);
				write_data(ADDR_CO_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].time+14,2);
				write_data(ADDR_CO_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].data);
				write_data(ADDR_CO_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO_4_1,0x01);
				write_data(ADDR_HISTORY_CO_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO]-offset-6)>=0){
			strcpy(time2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO_5_1,0x00);
				write_data(ADDR_HISTORY_CO_5_2,0x00);
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].time+11,2);
				write_data(ADDR_CO_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].time+14,2);
				write_data(ADDR_CO_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].data);
				write_data(ADDR_CO_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO_5_1,0x01);
				write_data(ADDR_HISTORY_CO_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_CO]-offset-7)>=0){
			strcpy(time2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_CO_6_1,0x00);
				write_data(ADDR_HISTORY_CO_6_2,0x00);
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].time+11,2);
				write_data(ADDR_CO_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].time+14,2);
				write_data(ADDR_CO_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].data);
				write_data(ADDR_CO_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_CO_6_1,0x01);
				write_data(ADDR_HISTORY_CO_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"pm25_cnt %d\n",g_share_memory->cnt[SENSOR_PM25]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time,4);
		write_data(ADDR_LIST_PM25_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time+5,2);
		write_data(ADDR_LIST_PM25_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time+8,2);
		write_data(ADDR_LIST_PM25_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_PM25]-offset-1)>=0)
		{//6:2
			write_data(ADDR_PM25_PAGE_N,page+1);
			write_data(ADDR_PM25_PAGE_ALL,count_all_pages(sensor_history.pm25,g_share_memory->cnt[SENSOR_PM25]));
			memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time+11,2);
			write_data(ADDR_PM25_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time+14,2);
			write_data(ADDR_PM25_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].data);
			write_data(ADDR_PM25_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_PM25]-offset-2)>=0){
			strcpy(time2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM25_1_1,0x00);
				write_data(ADDR_HISTORY_PM25_1_2,0x00);
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].time+11,2);
				write_data(ADDR_PM25_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].time+14,2);
				write_data(ADDR_PM25_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].data);
				write_data(ADDR_PM25_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM25_1_1,0x01);
				write_data(ADDR_HISTORY_PM25_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM25]-offset-3)>=0){
			strcpy(time2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM25_2_1,0x00);
				write_data(ADDR_HISTORY_PM25_2_2,0x00);
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].time+11,2);
				write_data(ADDR_PM25_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].time+14,2);
				write_data(ADDR_PM25_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].data);
				write_data(ADDR_PM25_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM25_2_1,0x01);
				write_data(ADDR_HISTORY_PM25_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM25]-offset-4)>=0){
			strcpy(time2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM25_3_1,0x00);
				write_data(ADDR_HISTORY_PM25_3_2,0x00);
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].time+11,2);
				write_data(ADDR_PM25_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].time+14,2);
				write_data(ADDR_PM25_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].data);
				write_data(ADDR_PM25_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM25_3_1,0x01);
				write_data(ADDR_HISTORY_PM25_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM25]-offset-5)>=0){
			strcpy(time2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM25_4_1,0x00);
				write_data(ADDR_HISTORY_PM25_4_2,0x00);
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].time+11,2);
				write_data(ADDR_PM25_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].time+14,2);
				write_data(ADDR_PM25_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].data);
				write_data(ADDR_PM25_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM25_4_1,0x01);
				write_data(ADDR_HISTORY_PM25_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM25]-offset-6)>=0){
			strcpy(time2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM25_5_1,0x00);
				write_data(ADDR_HISTORY_PM25_5_2,0x00);
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].time+11,2);
				write_data(ADDR_PM25_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].time+14,2);
				write_data(ADDR_PM25_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].data);
				write_data(ADDR_PM25_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM25_5_1,0x01);
				write_data(ADDR_HISTORY_PM25_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM25]-offset-7)>=0){
			strcpy(time2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM25_6_1,0x00);
				write_data(ADDR_HISTORY_PM25_6_2,0x00);
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].time+11,2);
				write_data(ADDR_PM25_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].time+14,2);
				write_data(ADDR_PM25_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].data);
				write_data(ADDR_PM25_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM25_6_1,0x01);
				write_data(ADDR_HISTORY_PM25_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_PM_10,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"pm10_cnt %d\n",g_share_memory->cnt[SENSOR_PM10]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-1].time,4);
		write_data(ADDR_LIST_PM10_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-1].time+5,2);
		write_data(ADDR_LIST_PM10_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-1].time+8,2);
		write_data(ADDR_LIST_PM10_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_PM10]-offset-1)>=0)
		{//6:2
			write_data(ADDR_PM10_PAGE_N,page+1);
			write_data(ADDR_PM10_PAGE_ALL,count_all_pages(sensor_history.pm10,g_share_memory->cnt[SENSOR_PM10]));
			memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-1].time+11,2);
			write_data(ADDR_PM10_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-1].time+14,2);
			write_data(ADDR_PM10_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-1].data);
			write_data(ADDR_PM10_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_PM10]-offset-2)>=0){
			strcpy(time2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM10_1_1,0x00);
				write_data(ADDR_HISTORY_PM10_1_2,0x00);
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-2].time+11,2);
				write_data(ADDR_PM10_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-2].time+14,2);
				write_data(ADDR_PM10_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-2].data);
				write_data(ADDR_PM10_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM10_1_1,0x01);
				write_data(ADDR_HISTORY_PM10_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM10]-offset-3)>=0){
			strcpy(time2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM10_2_1,0x00);
				write_data(ADDR_HISTORY_PM10_2_2,0x00);
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-3].time+11,2);
				write_data(ADDR_PM10_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-3].time+14,2);
				write_data(ADDR_PM10_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-3].data);
				write_data(ADDR_PM10_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM10_2_1,0x01);
				write_data(ADDR_HISTORY_PM10_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM10]-offset-4)>=0){
			strcpy(time2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM10_3_1,0x00);
				write_data(ADDR_HISTORY_PM10_3_2,0x00);
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-4].time+11,2);
				write_data(ADDR_PM10_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-4].time+14,2);
				write_data(ADDR_PM10_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-4].data);
				write_data(ADDR_PM10_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM10_3_1,0x01);
				write_data(ADDR_HISTORY_PM10_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM10]-offset-5)>=0){
			strcpy(time2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM10_4_1,0x00);
				write_data(ADDR_HISTORY_PM10_4_2,0x00);
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-5].time+11,2);
				write_data(ADDR_PM10_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-5].time+14,2);
				write_data(ADDR_PM10_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-5].data);
				write_data(ADDR_PM10_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM10_4_1,0x01);
				write_data(ADDR_HISTORY_PM10_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM10]-offset-6)>=0){
			strcpy(time2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM10_5_1,0x00);
				write_data(ADDR_HISTORY_PM10_5_2,0x00);
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-6].time+11,2);
				write_data(ADDR_PM10_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-6].time+14,2);
				write_data(ADDR_PM10_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-6].data);
				write_data(ADDR_PM10_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM10_5_1,0x01);
				write_data(ADDR_HISTORY_PM10_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_PM10]-offset-7)>=0){
			strcpy(time2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_PM10_6_1,0x00);
				write_data(ADDR_HISTORY_PM10_6_2,0x00);
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-7].time+11,2);
				write_data(ADDR_PM10_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-7].time+14,2);
				write_data(ADDR_PM10_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-offset-7].data);
				write_data(ADDR_PM10_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_PM10_6_1,0x01);
				write_data(ADDR_HISTORY_PM10_6_2,0x01);
			}
			}
		}
	}
	if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
		printfLog(LCD_PROCESS"hcho_cnt %d\n",g_share_memory->cnt[SENSOR_HCHO]);
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time,4);
		write_data(ADDR_LIST_HCHO_YEAR,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time+5,2);
		write_data(ADDR_LIST_HCHO_MON,atoi(tmp));
		memset(tmp,'\0',4);
		memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time+8,2);
		write_data(ADDR_LIST_HCHO_DAY,atoi(tmp));

		if((g_share_memory->cnt[SENSOR_HCHO]-offset-1)>=0)
		{//6:2
			write_data(ADDR_HCHO_PAGE_N,page+1);
			write_data(ADDR_HCHO_PAGE_ALL,count_all_pages(sensor_history.hcho,g_share_memory->cnt[SENSOR_HCHO]));
			memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time+11,2);
			write_data(ADDR_HCHO_TIME_0,atoi(tmp));
			memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time+14,2);
			write_data(ADDR_HCHO_TIME_0_1,atoi(tmp));
			out=remove_p(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].data);
			write_data(ADDR_HCHO_DATA_0,atoi(out));
			free(out);
			offs++;
			strcpy(time1,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time);
			if((g_share_memory->cnt[SENSOR_HCHO]-offset-2)>=0){
			strcpy(time2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_HCHO_1_1,0x00);
				write_data(ADDR_HISTORY_HCHO_1_2,0x00);
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].time+11,2);
				write_data(ADDR_HCHO_TIME_1,atoi(tmp));
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].time+14,2);
				write_data(ADDR_HCHO_TIME_1_1,atoi(tmp));
				out=remove_p(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].data);
				write_data(ADDR_HCHO_DATA_1,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_HCHO_1_1,0x01);
				write_data(ADDR_HISTORY_HCHO_1_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_HCHO]-offset-3)>=0){
			strcpy(time2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_HCHO_2_1,0x00);
				write_data(ADDR_HISTORY_HCHO_2_2,0x00);
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].time+11,2);
				write_data(ADDR_HCHO_TIME_2,atoi(tmp));
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].time+14,2);
				write_data(ADDR_HCHO_TIME_2_1,atoi(tmp));
				out=remove_p(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].data);
				write_data(ADDR_HCHO_DATA_2,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_HCHO_2_1,0x01);
				write_data(ADDR_HISTORY_HCHO_2_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_HCHO]-offset-4)>=0){
			strcpy(time2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_HCHO_3_1,0x00);
				write_data(ADDR_HISTORY_HCHO_3_2,0x00);
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].time+11,2);
				write_data(ADDR_HCHO_TIME_3,atoi(tmp));
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].time+14,2);
				write_data(ADDR_HCHO_TIME_3_1,atoi(tmp));
				out=remove_p(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].data);
				write_data(ADDR_HCHO_DATA_3,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_HCHO_3_1,0x01);
				write_data(ADDR_HISTORY_HCHO_3_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_HCHO]-offset-5)>=0){
			strcpy(time2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_HCHO_4_1,0x00);
				write_data(ADDR_HISTORY_HCHO_4_2,0x00);
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].time+11,2);
				write_data(ADDR_HCHO_TIME_4,atoi(tmp));
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].time+14,2);
				write_data(ADDR_HCHO_TIME_4_1,atoi(tmp));
				out=remove_p(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].data);
				write_data(ADDR_HCHO_DATA_4,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_HCHO_4_1,0x01);
				write_data(ADDR_HISTORY_HCHO_4_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_HCHO]-offset-6)>=0){
			strcpy(time2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_HCHO_5_1,0x00);
				write_data(ADDR_HISTORY_HCHO_5_2,0x00);
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].time+11,2);
				write_data(ADDR_HCHO_TIME_5,atoi(tmp));
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].time+14,2);
				write_data(ADDR_HCHO_TIME_5_1,atoi(tmp));
				out=remove_p(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].data);
				write_data(ADDR_HCHO_DATA_5,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_HCHO_5_1,0x01);
				write_data(ADDR_HISTORY_HCHO_5_2,0x01);
			}
			}
			if((g_share_memory->cnt[SENSOR_HCHO]-offset-7)>=0){
			strcpy(time2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].time);
			if(strncmp(time1,time2,11)==0)
			{
				write_data(ADDR_HISTORY_HCHO_6_1,0x00);
				write_data(ADDR_HISTORY_HCHO_6_2,0x00);
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].time+11,2);
				write_data(ADDR_HCHO_TIME_6,atoi(tmp));
				memcpy(tmp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].time+14,2);
				write_data(ADDR_HCHO_TIME_6_1,atoi(tmp));
				out=remove_p(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].data);
				write_data(ADDR_HCHO_DATA_6,atoi(out));
				free(out);
				offs++;
			}
			else
			{
				write_data(ADDR_HISTORY_HCHO_6_1,0x01);
				write_data(ADDR_HISTORY_HCHO_6_2,0x01);
			}
			}
		}
	}
	return offs;
}
bool execute_cmd(char *cmd)
{
	char ret[256] = {0};
	FILE *fp;
	if ((fp=popen(cmd,"r"))!=NULL)
	{
		memset(ret,0,256);
		fread(ret,sizeof(char),sizeof(ret),fp);
		pclose(fp);
		printfLog(LCD_PROCESS"%s\n%s\n", cmd,ret);		
	}

	if(strlen(ret)!=0)
		return true;

	return false;
}

void wifi_handle()
{
	char ap_name[256]={0};
	char ap_passwd[256]={0};
	char ret[256]={0};
	char cmd[256]={0};
	int i;
	FILE *fp;
	//clear_buf(fd,WIFI_AP_NAME_ADDR,20);
	//clear_buf(fd,WIFI_AP_PWD_ADDR,20);
	if(read_dgus(ADDR_AP_NAME,10,ap_name) && read_dgus(ADDR_AP_KEY,10,ap_passwd))
	{
		if (strlen(ap_passwd)!=0) {
			printfLog(LCD_PROCESS"AP Name %s \nAP Pwd %s\n",ap_name,ap_passwd);
			if(strlen(ap_passwd)<8||strlen(ap_name)==0)
				return ;
			switch_pic(100);
			sprintf(cmd,"wpa_cli -ira0 status");
			if ((fp=popen(cmd,"r"))!=NULL)
			{
				memset(ret,0,256);
				fread(ret,sizeof(char),sizeof(ret),fp);
				pclose(fp);
				printfLog(LCD_PROCESS"wpa_cli status \n%s\n", ret);
				if(strstr(ret,"Failed") || strlen(ret)==0)
				{
					//system("ifconfig ra0 up");
					//sleep(5);
					//system("wpa_supplicant -ira0 -c/etc/wpa_supplicant.conf -B");
					execute_cmd("wpa_supplicant -ira0 -c/etc/wpa_supplicant.conf -B");
					//sleep(6);
				}
			}
			for(i=0;i<100;i++)
			{
				sprintf(cmd,"wpa_cli -ira0 get_network %d ssid",i);
				if((fp=popen(cmd,"r"))!=NULL)
				{
					memset(ret,0,256);
					fread(ret,sizeof(char),sizeof(ret),fp);
					pclose(fp);
					printfLog(LCD_PROCESS"wpa_cli -ira0 get_network return %s %d\n",ret,strlen(ret));
					if(strstr(ret,"FAIL")!=NULL)
						break;
					if(strstr(ret,ap_name)!=NULL)
						break;					
				}
			}
			if(strstr(ret,"FAIL")!=NULL)
			{
				memset(cmd,0,256);
				sprintf(cmd,"wpa_cli -ira0 add_network");
				if((fp=popen(cmd,"r"))!=NULL)
				{
					memset(ret,0,256);
					fread(ret,sizeof(char),sizeof(ret),fp);
					pclose(fp);
					ret[strlen(ret)-1]='\0';
					printfLog(LCD_PROCESS"add_network return %s\n",ret);
					i=atoi(ret);
				}
			}
			printfLog(LCD_PROCESS"i is %d\n",i);
			sprintf(cmd,"wpa_cli -ira0 set_network %d key_mgmt WPA-PSK",i);
			//printfLog(LCD_PROCESS"exec %s\n",cmd);
			execute_cmd(cmd);	
			sprintf(cmd,"wpa_cli -ira0 set_network %d psk \\\"%s\\\"",i,ap_passwd);
			//printfLog(LCD_PROCESS"exec %s\n",cmd);
			execute_cmd(cmd);		
			sprintf(cmd,"wpa_cli -ira0 set_network %d ssid \\\"%s\\\"",i,ap_name);
			//printfLog(LCD_PROCESS"exec %s\n",cmd);
			execute_cmd(cmd);
			sprintf(cmd,"wpa_cli -ira0 enable_network %d",i);
			//printfLog(LCD_PROCESS"exec %s\n",cmd);
			execute_cmd(cmd);
			////sprintf(cmd,"wpa_cli -ira0 select_network %d",i);
			////printf("exec %s\n",cmd);
			////system(cmd);
			sprintf(cmd,"wpa_cli -ira0 save_config");
			//printfLog(LCD_PROCESS"exec %s\n",cmd);		
			execute_cmd(cmd);
			//sprintf(cmd,"udhcpc -i ra0");
			//printfLog(LCD_PROCESS"exec %s\n",cmd);
			////system(cmd);
		}
		else
		{
			//wep handle
			switch_pic(100);
			printfLog(LCD_PROCESS"AP Name %s \n",ap_name);
			sprintf(cmd,"wpa_cli status");
			if ((fp=popen(cmd,"r"))!=NULL)
			{
				memset(ret,0,256);
				fread(ret,sizeof(char),sizeof(ret),fp);
				pclose(fp);
				printfLog(LCD_PROCESS"wpa_cli status %s\n", ret);
				if(NULL == strstr(ret,"Failed"))
				{
					//sprintf(cmd,"wpa_cli -ira0 terminate");
					//printfLog(LCD_PROCESS"exec %s\n",cmd);
					//system(cmd);
					execute_cmd("wpa_cli terminate");
					//sleep(5);
					//sprintf(cmd,"ifconfig ra0 up");
					//printfLog(LCD_PROCESS"exec %s\n",cmd);
					//system(cmd);
					execute_cmd("ifconfig ra0 up");
					//sleep(5);
				}
			}
			sprintf(cmd,"iwconfig ra0 essid %s", ap_name);
			int i=0;
			while(execute_cmd(cmd))
			{
				sleep(1);
				i++;
				if(i>3)
					break;
			}
		}
		sprintf(cmd,"udhcpc -i ra0 -q -n");
		execute_cmd(cmd);
	}
}

void show_curve(char *id,int* offset)
{
	int buf[144]={0};
	char temp[10]={0},temp2[10]={0};
	char hour2[3]={0};
	int i=0,j=0;
	clear_curve();	
	if(strncmp(id,ID_CAP_PM_10,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-1].time,4);
		write_data(ADDR_CURVE_PM10_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_PM10_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_PM10_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_PM10]-*offset)>0)
		{	
			memcpy(temp,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					if(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-i-1].data[2]=='0')
						buf[atoi(hour2)*5+j]=(atoi(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-i-1].data+3)*11)/3;
					else
						buf[atoi(hour2)*5+j]=(atoi(sensor_history.pm10[g_share_memory->cnt[SENSOR_PM10]-*offset-i-1].data+2)*11)/3;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_CO,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-1].time,4);
		write_data(ADDR_CURVE_CO_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_CO_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_CO_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_CO]-*offset)>0)
		{	
			memcpy(temp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					buf[atoi(hour2)*5+j]=atoi(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-i-1].data)*4;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-1].time,4);
		write_data(ADDR_CURVE_CO2_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_CO2_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_CO2_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_CO2]-*offset)>0)
		{	
			memcpy(temp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					buf[atoi(hour2)*5+j]=(atoi(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-i-1].data)*7)/20;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-1].time,4);
		write_data(ADDR_CURVE_TEMP_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_TEMP_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_TEMP_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_TEMP]-*offset)>0)
		{	
			memcpy(temp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					buf[atoi(hour2)*5+j]=atoi(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-i-1].data)*21;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-1].time,4);
		write_data(ADDR_CURVE_SHIDU_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_SHIDU_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_SHIDU_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_SHIDU]-*offset)>0)
		{	
			memcpy(temp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					buf[atoi(hour2)*5+j]=atoi(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-i-1].data)*9;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_FENG_SU,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-1].time,4);
		write_data(ADDR_CURVE_WIND_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_WIND_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_WIND_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_WIND]-*offset)>0)
		{	
			memcpy(temp,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					buf[atoi(hour2)*5+j]=(atoi(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-i-1].data)*100+
									atoi(sensor_history.wind[g_share_memory->cnt[SENSOR_WIND]-*offset-i-1].data+2))*3;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_BUZZY,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-*offset-1].time,4);
		write_data(ADDR_CURVE_NOISE_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_NOISE_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_NOISE_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_NOISE]-*offset)>0)
		{	
			memcpy(temp,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					buf[atoi(hour2)*5+j]=(atoi(sensor_history.noise[g_share_memory->cnt[SENSOR_NOISE]-*offset-i-1].data)*11)/2;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_QI_YA,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-1].time,4);
		write_data(ADDR_CURVE_PRESS_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_PRESS_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_PRESS_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_PRESS]-*offset)>0)
		{	
			memcpy(temp,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					if(atoi(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-i-1].data)>=400)
						buf[atoi(hour2)*5+j]=atoi(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-i-1].data)-400;
					else
						buf[atoi(hour2)*5+j]=atoi(sensor_history.press[g_share_memory->cnt[SENSOR_PRESS]-*offset-i-1].data);
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_TVOC,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-*offset-1].time,4);
		write_data(ADDR_CURVE_TVOC_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_TVOC_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_TVOC_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_TVOC]-*offset)>0)
		{	
			memcpy(temp,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*4;
					buf[atoi(hour2)*5+j]=(atoi(sensor_history.tvoc[g_share_memory->cnt[SENSOR_TVOC]-*offset-i-1].data+2)*2)/3;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_CHOU_YANG,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-1].time,4);
		write_data(ADDR_CURVE_O3_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_O3_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_O3_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_O3]-*offset)>0)
		{	
			memcpy(temp,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-i-1].time+11,2);
					//buf[atoi(hour2)*5+j]=atoi(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].data+2)*10;
					buf[atoi(hour2)*5+j]=((atoi(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-i-1].data)*1000+
									atoi(sensor_history.o3[g_share_memory->cnt[SENSOR_O3]-*offset-i-1].data+2)*10)*2)/3;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}							
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-1].time,4);
		write_data(ADDR_CURVE_PM25_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_PM25_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_PM25_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_PM25]-*offset)>0)
		{	
			memcpy(temp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].time+11,2);
					buf[atoi(hour2)*5+j]=(atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*11)/3;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}
		else
			*offset=0;
	}
	if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-1].time,4);
		write_data(ADDR_CURVE_HCHO_YEAR,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-1].time+5,2);
		write_data(ADDR_CURVE_HCHO_MON,atoi(temp));
		memset(temp,'\0',10);
		memcpy(temp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-1].time+8,2);
		write_data(ADDR_CURVE_HCHO_DAY,atoi(temp));
		if((g_share_memory->cnt[SENSOR_HCHO]-*offset)>0)
		{	
			memcpy(temp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-1].time,10);
			while(1)
			{
				memcpy(temp2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{
					memcpy(hour2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].time+11,2);
					buf[atoi(hour2)*5+j]=atoi(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].data+2)*7;
					j=j+1;
					if(j==5)
						j=0;
					i++;
				}
				else
					break;
			}
		}		
		else
			*offset=0;
	}	
	
	j=120;
	draw_curve(buf,j);
}
int read_dgus(int addr,char len,char *out)
{
	char ch,i;
	int timeout=0;
	char cmd[]={0x5a,0xa5,0x04,0x83,0x00,0x00,0x00};
	cmd[4]=(addr & 0xff00)>>8;
	cmd[5]=addr & 0x00ff;
	cmd[6]=len;
	write(g_share_memory->fd_lcd,cmd,7);
	i=0;
	while(1)	
	{	
		if(read(g_share_memory->fd_lcd,&ch,1)==1)
		{
			if(i>=7 && ch!=0xff)
				out[i-7]=ch;
			i++;
			//printfLog(LCD_PROCESS"==> %x\n",ch);
			if(i==(2*len+7))
				return 1;
		}
		else
		{
			timeout++;
			if(timeout>100)
				return 0;
			usleep(1000);
		}
	}
	return 0;
}
void display_time(int year,int mon,int day,int hour,int min,int seconds)
{
	char buf[5]={0};
	sprintf(buf,"%d",year);
	write_string(ADDR_YEAR,buf,4);
	memset(buf,0,5);
	sprintf(buf,"%d",day);
	write_string(ADDR_DAY,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",mon);
	write_string(ADDR_MON,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",hour);
	write_string(ADDR_HOUR,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",min);
	write_string(ADDR_MIN,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",seconds);
	write_string(ADDR_SEC,buf,2);
}
int verify_pwd(char *username,char *passwd)
{
	int result=0;
	if((strlen(username)==strlen("root") && strlen(passwd)==strlen("84801058")) && strncmp(username,"root",strlen("root"))==0 && strncmp(passwd,"84801058",strlen("84801058"))==0)
	{
		logged=1;
		return 1;
	}
	char *verify_msg=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_VERIFY_USER);
	verify_msg=add_item(verify_msg,ID_DEVICE_UID,g_share_memory->uuid);
	verify_msg=add_item(verify_msg,ID_DEVICE_IP_ADDR,g_share_memory->ip);
	verify_msg=add_item(verify_msg,ID_DEVICE_PORT,(char *)"9517");
	verify_msg=add_item(verify_msg,ID_USER_NAME_TYPE,username);
	verify_msg=add_item(verify_msg,ID_USER_PWD_TYPE,passwd);
	char *rcv=NULL;
	send_web_post(URL,verify_msg,9,&rcv);
	free(verify_msg);
	verify_msg=NULL;
	if(rcv!=NULL)
	{	
		if(strstr(rcv,"ok"))
		{
			result=1;
			logged=1;
		}
		free(rcv);
		rcv=NULL;
	}	
	return result;
}
void interface_to_string(int interface,char *str)
{
	//char str[20];
	memset(str,'\0',256);	
	switch (interface)
	{
		case TYPE_SENSOR_CO_WEISHEN:
			strcpy(str,"CO_1");
			break;
		case TYPE_SENSOR_CO_DD:
			strcpy(str,"CO_2");
			break;
		case TYPE_SENSOR_CO2_WEISHEN:
			strcpy(str,"CO2_1");
			break; 
		case TYPE_SENSOR_CO2_RUDIAN:
			strcpy(str,"CO2_2");
			break;
		case TYPE_SENSOR_CH2O_WEISHEN:
			strcpy(str,"CH2O_1");
			break;
		case TYPE_SENSOR_CH2O_AERSHEN:
			strcpy(str,"CH2O_2");
			break;
		case TYPE_SENSOR_PM25_WEISHEN:
			strcpy(str,"PM2.5_1");
			break;
		case TYPE_SENSOR_PM25_WEISHEN2:
			strcpy(str,"PM2.5_2");
			break;
		case TYPE_SENSOR_WENSHI_RUSHI:
			strcpy(str,"温湿_1");
			break;
		case TYPE_SENSOR_QIYA_RUSHI:
			strcpy(str,"气压_1");
			break;
		case TYPE_SENSOR_FENGSU:
			strcpy(str,"风速_1");
			break;
		case TYPE_SENSOR_ZHAOSHEN:
			strcpy(str,"噪声_1");
			break;
		case TYPE_SENSOR_TVOC_1:
			strcpy(str,"TVOC_1");
			break;		
		case TYPE_SENSOR_O3_1:
			strcpy(str,"O3_1");
			break;
		default:
			strcpy(str,"未知传感器");
			break;
	}
	//code_convert("utf-8","gbk",str,strlen(str),name,256);
}

void log_in()
{
	char user_name[256]={0};
	char passwd[256]={0};
	if(read_dgus(ADDR_LOGIN_USER_NAME,16,user_name) && read_dgus(ADDR_LOGIN_USER_KEY,16,passwd))
	{
		printfLog(LCD_PROCESS"User Name %s \nUser Pwd %s\n",user_name,passwd);
		if(verify_pwd(user_name,passwd))
		{
			//switch_pic(g_index);
			if(g_index==SENSOR_SETTING_PAGE)
				ctl_fan(0);
			return;
		}
	}
	switch_pic(last_g_index);
	g_index=last_g_index;
}
void show_cur_select_intr(int sel)
{
	char name[256]={0};
	clear_buf(ADDR_CURRENT_SELECT,10);
	interface_to_string(sel,name);
	write_string(ADDR_CURRENT_SELECT,name,strlen(name));
}
void show_cur_interface(int page)
{
	char name[256]={0};
	//if(page==INTERFACE_PAGE)
	//{
		clear_buf(ADDR_INTERFACE_0,10);
		clear_buf(ADDR_INTERFACE_1,10);
		clear_buf(ADDR_INTERFACE_2,10);
		clear_buf(ADDR_INTERFACE_3,10);
		clear_buf(ADDR_INTERFACE_4,10);
		clear_buf(ADDR_INTERFACE_5,10);
		clear_buf(ADDR_INTERFACE_6,10);
		clear_buf(ADDR_INTERFACE_7,10);
		clear_buf(ADDR_INTERFACE_8,10);
		clear_buf(ADDR_INTERFACE_9,10);
		clear_buf(ADDR_INTERFACE_10,10);	
		clear_buf(ADDR_INTERFACE_11,10);
		interface_to_string(g_share_memory->sensor_interface_mem[0],name);
		write_string(ADDR_INTERFACE_0,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[1],name);
		write_string(ADDR_INTERFACE_1,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[2],name);
		write_string(ADDR_INTERFACE_2,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[3],name);
		write_string(ADDR_INTERFACE_3,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[4],name);
		write_string(ADDR_INTERFACE_4,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[5],name);
		write_string(ADDR_INTERFACE_5,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[6],name);
		write_string(ADDR_INTERFACE_6,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[7],name);
		write_string(ADDR_INTERFACE_7,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[8],name);
		write_string(ADDR_INTERFACE_8,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[9],name);
		write_string(ADDR_INTERFACE_9,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[10],name);
		write_string(ADDR_INTERFACE_10,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[11],name);
		write_string(ADDR_INTERFACE_11,name,strlen(name));
	/*}
	else
	{
		clear_buf(ADDR_VERIFY_HCHO,10);
		clear_buf(ADDR_VERIFY_PM25,10);
		clear_buf(ADDR_VERIFY_INT3,10);
		clear_buf(ADDR_VERIFY_INT4,10);
		clear_buf(ADDR_VERIFY_INT5,10);
		clear_buf(ADDR_VERIFY_INT6,10);
		clear_buf(ADDR_VERIFY_INT7,10);
		clear_buf(ADDR_VERIFY_INT8,10);
		clear_buf(ADDR_VERIFY_WENSHI,10);
		clear_buf(ADDR_VERIFY_FENGSU,10);	
		clear_buf(ADDR_VERIFY_QIYA,10);	
		interface_to_string(g_share_memory->sensor_interface_mem[0],name);
		write_string(ADDR_VERIFY_HCHO,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[1],name);
		write_string(ADDR_VERIFY_PM25,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[2],name);
		write_string(ADDR_VERIFY_INT3,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[3],name);
		write_string(ADDR_VERIFY_INT4,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[4],name);
		write_string(ADDR_VERIFY_INT5,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[5],name);
		write_string(ADDR_VERIFY_INT6,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[6],name);
		write_string(ADDR_VERIFY_INT7,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[7],name);
		write_string(ADDR_VERIFY_INT8,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[8],name);
		write_string(ADDR_VERIFY_WENSHI,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[9],name);
		write_string(ADDR_VERIFY_FENGSU,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[10],name);
		write_string(ADDR_VERIFY_QIYA,name,strlen(name));
	}*/
	
}
void set_lcd_time(char *buf)
{
	int i;
	char cmd[]={0x5a,0xa5,0x0a,0x80,0x1f,0x5a,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	for(i=0;i<7;i++)
		cmd[i+6]=buf[i];
	//for(i=0;i<sizeof(cmd);i++)
	//	printf("%02x ",cmd[i]);
	//printf("\n");
	write(g_share_memory->fd_lcd,cmd,sizeof(cmd));
}

void manul_set_time()
{
	char year[5]={0};
	char mon[3]={0};
	char day[3]={0};
	char hour[3]={0};
	char min[3]={0};
	char second[3]={0};
	if(read_dgus(ADDR_YEAR,2,year) && read_dgus(ADDR_DAY,1,day)
		&& read_dgus(ADDR_MON,1,mon) && read_dgus(ADDR_HOUR,1,hour)
		&& read_dgus(ADDR_MIN,1,min) && read_dgus(ADDR_SEC,1,second))
	{
		if(atoi(year)>0 && atoi(mon)>0 && atoi(mon)<=12 && atoi(day)>0 && atoi(day)<=31
			&& atoi(hour)>=0 && atoi(hour)<=23 && atoi(min)>=0 && atoi(min)<=59
			&&atoi(second)>=0 && atoi(second)<=59)
		{
			printfLog(LCD_PROCESS"year %s \nmon %s\nday %s\nhour %s\nmin %s\nseconds %s\n",year,mon,day,hour,min,second);
			g_share_memory->server_time[0]=0x6c;g_share_memory->server_time[1]=ARM_TO_CAP;
			g_share_memory->server_time[2]=0x00;g_share_memory->server_time[3]=0x01;g_share_memory->server_time[4]=0x06;
			g_share_memory->server_time[5]=atoi(year)-2000;g_share_memory->server_time[6]=atoi(mon);
			g_share_memory->server_time[7]=atoi(day);g_share_memory->server_time[8]=atoi(hour);
			g_share_memory->server_time[9]=atoi(min);g_share_memory->server_time[10]=atoi(second);
			int crc=CRC_check((unsigned char *)g_share_memory->server_time,11);
			g_share_memory->server_time[11]=(crc&0xff00)>>8;g_share_memory->server_time[12]=crc&0x00ff;
			send_cmd_to_cap(g_share_memory->server_time,13);
			int week=CaculateWeekDay(g_share_memory->server_time[5],g_share_memory->server_time[6],g_share_memory->server_time[7]);
			char rtc_time[7];
			rtc_time[0]=(g_share_memory->server_time[5]/10)*16+(g_share_memory->server_time[5]%10);
			rtc_time[1]=(g_share_memory->server_time[6]/10)*16+(g_share_memory->server_time[6]%10);
			rtc_time[2]=(g_share_memory->server_time[7]/10)*16+(g_share_memory->server_time[7]%10);
			rtc_time[3]=week+1;
			rtc_time[4]=(g_share_memory->server_time[8]/10)*16+(g_share_memory->server_time[8]%10);
			rtc_time[5]=(g_share_memory->server_time[9]/10)*16+(g_share_memory->server_time[9]%10);
			rtc_time[6]=(g_share_memory->server_time[10]/10)*16+(g_share_memory->server_time[10]%10);
			set_lcd_time(rtc_time);
		}
		//set_time(server_time[5]+2000,server_time[6],server_time[7],server_time[8],server_time[9],server_time[10]);
	}
}
void jiaozhun(int on,char sensor,char jp)
{
	char cmd_verify[]=	{0x6c,ARM_TO_CAP,0x00,0x04,0x01,0x00,0x00,0x00};
	int i;
	if(on)
	{
		switch_pic(100);
		g_index=VERIFY_PAGE;
		if(g_share_memory->factory_mode!=SENSOR_VERIFY_MODE)
		{
			printfLog(LCD_PROCESS"Begin to JiaoZhun %d\n",sensor);
			g_share_memory->factory_mode=SENSOR_VERIFY_MODE;
			cmd_verify[5]=sensor+1;
			int crc=CRC_check((unsigned char *)cmd_verify,6);
			cmd_verify[6]=(crc&0xff00)>>8;cmd_verify[7]=crc&0x00ff;		
			for(i=0;i<sizeof(cmd_verify);i++)
				printfLog("%02X ",cmd_verify[i]);
			printfLog("\n");
			send_cmd_to_cap(cmd_verify,sizeof(cmd_verify));
		}
	}
	else
	{
		
		switch_pic(SENSOR_SETTING_PAGE);
		g_index=SENSOR_SETTING_PAGE;
		if(g_share_memory->factory_mode==SENSOR_VERIFY_MODE)
		{
			g_share_memory->factory_mode=NORMAL_MODE;
			printfLog(LCD_PROCESS"End to JiaoZhun %d\n",sensor);
		}
	}
}
int getxiuzhen()
{
	char data[10]={0};
	int d=0,p_get=0,y=0;
	if(read_dgus(ADDR_XIUZHENG,5,data))
	{
		printfLog(LCD_PROCESS"===>%s\n",data);
		if(strchr(data,'.')!=NULL)
		{
			char data2[64]={0};
			int i,j=0;
			for(i=0;i<10;i++)
			{
				if(data[i]!='.')
					data2[j++]=data[i];
				else
					p_get=1;
				if(p_get)
					y++;
				if(g_share_memory->y==y-1)
					break;
			}
			/*printfLog("y is %d\n",y);
			if(y<g_share_memory->y+1)
			{
				for(i=0;i<g_share_memory->y-y+1;i++)
					data2[y+i+1]='0';		
			}*/
			d=atoi(data2);
		}
		else
			d=atoi(data);
	}
	return d;
}

void send_return(char sensor,char jp)
{	
	int i;
	char cmd_return[]=	{0x6c,ARM_TO_CAP,0x00,0x05,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
	int xz=getxiuzhen();
	if(g_share_memory->y!=0)
	{
		int m=1,j=0;
		for(j=0;j<g_share_memory->y;j++)
			m=m*10;
		g_share_memory->x[(int)jp]=(float)xz/(float)m;	
	}
	else
		g_share_memory->x[(int)jp]=xz;		
	printfLog("getxiuzhen %d,y %d, x %3.6f\n",xz,g_share_memory->y,g_share_memory->x[(int)jp]);
	cmd_return[5]=sensor+1;
	cmd_return[6]=jp+1;
	cmd_return[7]=(xz>>8)&0xff;
	cmd_return[8]=xz&0xff;
	int crc=CRC_check((unsigned char *)cmd_return,9);
	cmd_return[9]=(crc&0xff00)>>8;cmd_return[10]=crc&0x00ff;		
	for(i=0;i<sizeof(cmd_return);i++)
		printfLog(LCD_PROCESS"%02X ",cmd_return[i]);
	printfLog(LCD_PROCESS"\n");
	send_cmd_to_cap(cmd_return,sizeof(cmd_return));
}
char *Get_Type(int index)
{
	if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_CO_WEISHEN ||
		g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_CO_DD)
		return ID_CAP_CO;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_CO2_WEISHEN ||
		g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_CO2_RUDIAN)
		return ID_CAP_CO2;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_CH2O_WEISHEN ||
		g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_CH2O_AERSHEN)
		return ID_CAP_HCHO;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_PM25_WEISHEN ||
		g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_PM25_WEISHEN2)
		return ID_CAP_PM_25;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_WENSHI_RUSHI)
		return ID_CAP_TEMPERATURE;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_QIYA_RUSHI)
		return ID_CAP_QI_YA;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_ZHAOSHEN)
		return ID_CAP_BUZZY;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_FENGSU)
		return ID_CAP_FENG_SU;
	else if(g_share_memory->sensor_interface_mem[index]==TYPE_SENSOR_TVOC_1)
		return ID_CAP_TVOC;
	else
		return ID_CAP_CO;
	return NULL;
}
void show_point(int index,char sensor)
{
	char cmd0[]={0x5a,0xa5,0x15,0x82,0x0b,0x59,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x00,0x99};
	
	char cmd1[]={0x5a,0xa5,0x15,0x82,0x0b,0x62,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x00,0xcd};
	
	char cmd2[]={0x5a,0xa5,0x15,0x82,0x0b,0x6b,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x01,0x00};
	
	char cmd3[]={0x5a,0xa5,0x15,0x82,0x0b,0x74,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x01,0x33};
	
	char cmd4[]={0x5a,0xa5,0x15,0x82,0x0b,0x7d,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x01,0x68};
	
	char cmd5[]={0x5a,0xa5,0x15,0x82,0x0b,0x86,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x01,0x9c};
	
	char cmd6[]={0x5a,0xa5,0x15,0x82,0x0b,0x8f,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x01,0xcf};
	
	char cmd7[]={0x5a,0xa5,0x15,0x82,0x0b,0x98,0x00,0x06,0x00,0x01,0x00,0x23,
				0x02,0x8e,0x01,0x28,0x02,0xa1,0x01,0x3a,0x00,0xe3,0x02,0x03};
	
	char off0[]={0x5a,0xa5,0x15,0x82,0x0b,0x59,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x00,0xb7,0x00,0xea,0x00,0xc2,0x00,0xe3,0x00,0x99};

	char off1[]={0x5a,0xa5,0x15,0x82,0x0b,0x62,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x00,0xeb,0x00,0xea,0x00,0xf6,0x00,0xe3,0x00,0xcd};

	char off2[]={0x5a,0xa5,0x15,0x82,0x0b,0x6b,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x01,0x1e,0x00,0xea,0x01,0x2a,0x00,0xe3,0x01,0x00};

	char off3[]={0x5a,0xa5,0x15,0x82,0x0b,0x74,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x01,0x51,0x00,0xea,0x01,0x5e,0x00,0xe3,0x01,0x33};

	char off4[]={0x5a,0xa5,0x15,0x82,0x0b,0x7d,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x01,0x86,0x00,0xea,0x01,0x92,0x00,0xe3,0x01,0x68};

	char off5[]={0x5a,0xa5,0x15,0x82,0x0b,0x86,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x01,0xba,0x00,0xea,0x01,0xc6,0x00,0xe3,0x01,0x9c};

	char off6[]={0x5a,0xa5,0x15,0x82,0x0b,0x8f,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x01,0xed,0x00,0xea,0x01,0xf9,0x00,0xe3,0x01,0xcf};

	char off7[]={0x5a,0xa5,0x15,0x82,0x0b,0x98,0x00,0x06,0x00,0x01,0x00,0x28,
				0x00,0xe7,0x02,0x21,0x00,0xea,0x02,0x2c,0x00,0xe3,0x02,0x03};
	char cmd[64]={0};
	sprintf(cmd,"%6.6f",g_share_memory->x[index]);
	clear_buf(ADDR_XIUZHENG,10);
	write_string(ADDR_XIUZHENG,cmd,strlen(cmd));
	//send_return(fd,sensor,index);
	switch(index)
	{
		case 0:
			{
				write(g_share_memory->fd_lcd,cmd0,sizeof(cmd0));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
		case 1:
			{
				write(g_share_memory->fd_lcd,cmd1,sizeof(cmd1));
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
		case 2:
			{
				write(g_share_memory->fd_lcd,cmd2,sizeof(cmd2));
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
		case 3:
			{
				write(g_share_memory->fd_lcd,cmd3,sizeof(cmd3));
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
		case 4:
			{
				write(g_share_memory->fd_lcd,cmd4,sizeof(cmd4));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
		case 5:
			{
				write(g_share_memory->fd_lcd,cmd5,sizeof(cmd5));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
		case 6:
			{
				write(g_share_memory->fd_lcd,cmd6,sizeof(cmd6));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
		case 7:
			{
				write(g_share_memory->fd_lcd,cmd7,sizeof(cmd7));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
			}
			break;
		default:
			{
				write(g_share_memory->fd_lcd,off0,sizeof(off0));
				write(g_share_memory->fd_lcd,off1,sizeof(off1));
				write(g_share_memory->fd_lcd,off2,sizeof(off2));
				write(g_share_memory->fd_lcd,off3,sizeof(off3));
				write(g_share_memory->fd_lcd,off4,sizeof(off4));
				write(g_share_memory->fd_lcd,off5,sizeof(off5));
				write(g_share_memory->fd_lcd,off6,sizeof(off6));
				write(g_share_memory->fd_lcd,off7,sizeof(off7));
			}
			break;
	}
}
void set_interface()
{
	int i = 0;
	char cmd[] = {0x6c,ARM_TO_CAP,0x00,0x03,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};	
	for(i=0;i<22;i=i+2)
	{
		cmd[5+i]=(g_share_memory->sensor_interface_mem[i/2]>>8) & 0xff;
		cmd[5+i+1]=g_share_memory->sensor_interface_mem[i/2]&0xff;
		printfLog(LCD_PROCESS"%02x \n",(cmd[5+i]<<8)|cmd[6+i]);
	}
	int crc=CRC_check((unsigned char *)cmd,sizeof(cmd)-2);
	cmd[sizeof(cmd)-2]=(crc&0xff00)>>8;cmd[sizeof(cmd)-1]=crc&0x00ff; 	
	printfLog(LCD_PROCESS"going to set_interface begin\n");
	for(i=0;i<sizeof(cmd);i++)
		printfLog("%02x ",cmd[i]);
	printfLog("\ngoing to set_interface end\n");
	send_cmd_to_cap(cmd,sizeof(cmd));
}
void ctl_fan(int on)
{
	//int i =0;
	char cmd[]=	{0x6c,ARM_TO_CAP,0x00,0x09,0x01,0x00,0x00,0x00};
	if(on)
	{
		cmd[5]=0x01;
		printfLog("open fan\n");
	}
	else
		printfLog("close fan\n");
	int crc=CRC_check((unsigned char *)cmd,6);
	cmd[6]=(crc&0xff00)>>8;cmd[7]=crc&0x00ff;		
	//for(i=0;i<sizeof(cmd);i++)
	//	printfLog("%02X ",cmd[i]);
	//printfLog("\n");
	send_cmd_to_cap(cmd,sizeof(cmd));
}
void ctl_audio(int on)
{
	//int i =0;
	char cmd[]=	{0x6c,ARM_TO_CAP,0x00,0x0a,0x01,0x00,0x00,0x00};
	if(on)
	{
		cmd[5]=0x01;
		printfLog("open audio\n");
	}
	else
		printfLog("close audio\n");
	int crc=CRC_check((unsigned char *)cmd,6);
	cmd[6]=(crc&0xff00)>>8;cmd[7]=crc&0x00ff;		
	//for(i=0;i<sizeof(cmd);i++)
	//	printfLog("%02X ",cmd[i]);
	//printfLog("\n");
	send_cmd_to_cap(cmd,sizeof(cmd));
}

void tun_zero_return()
{
	char cmd_request_verify[]=	{0x6c,ARM_TO_CAP,0x00,0x07,0x01,0x00,0x00,0x00};	
	int crc = 0;
	int i =0;
	printfLog(LCD_PROCESS"Stop Co,ch2o tun zero\n");
	crc=CRC_check((unsigned char *)cmd_request_verify,6);
	cmd_request_verify[6]=(crc&0xff00)>>8;cmd_request_verify[7]=crc&0x00ff;		
	for(i=0;i<sizeof(cmd_request_verify);i++)
		printfLog("%02X ",cmd_request_verify[i]);
	printfLog("\n");
	send_cmd_to_cap(cmd_request_verify,sizeof(cmd_request_verify));
}
void tun_zero(int on)
{
	char cmd_request_verify[]=	{0x6c,ARM_TO_CAP,0x00,0x07,0x01,0x00,0x00,0x00};	
	int crc = 0;
	int i =0;
	//send cap board start co & hcho
	//for(i=0;i<11;i++)
	//	if(sensor_interface_mem[i] == TYPE_SENSOR_CO_WEISHEN ||
	//	sensor_interface_mem[i] == TYPE_SENSOR_CO_DD)
	//	break;
	//printf("CO interface %d %4x\n",i,sensor_interface_mem[i]);
	if(on)
	{		
		clear_buf(ADDR_TUN_ZERO_HCHO,6);
		clear_buf(ADDR_TUN_ZERO_CO,6);
		clear_buf(ADDR_TUN_ZERO_TVOC,6);
		printfLog(LCD_PROCESS"Start Co,ch2o tun zero\n");
		cmd_request_verify[5]=0x01;
	}
	else
	{
		if(g_share_memory->factory_mode==TUN_ZERO_MODE)
		{
			switch_pic(100);
			return_zero_point(1);//co
			sleep(1);
			return_zero_point(0);//ch2o
			sleep(1);
			return_zero_point(2);//tvoc
			sleep(1);
		}
		printfLog(LCD_PROCESS"Stop Co,ch2o tun zero\n");
		#if 0
		for(i=0;i<11;i++)
			if(sensor_interface_mem[i] == TYPE_SENSOR_CH2O_WEISHEN ||
				sensor_interface_mem[i] == TYPE_SENSOR_CH2O_AERSHEN)
				break;
		printf("CH2O interface %d %4x\n",i,sensor_interface_mem[i]);
		printf("CH2O zero value\n",g_zero_info->cur_ch2o);
		cmd_return_point[5]=i+1;
		cmd_return_point[7]=(g_zero_info->cur_ch2o>>8) & 0xff;
		cmd_return_point[8]=(g_zero_info->cur_ch2o & 0xff);
		crc=CRC_check(cmd_return_point,9);
		cmd_return_point[9]=(crc&0xff00)>>8;cmd_return_point[10]=crc&0x00ff;
		for(i=0;i<sizeof(cmd_return_point);i++)
			printf("%02X ",cmd_return_point[i]);
		printf("\n");
		write(fd_com,cmd_return_point,sizeof(cmd_return_point));
		for(i=0;i<11;i++)
			if(sensor_interface_mem[i] == TYPE_SENSOR_CO_WEISHEN ||
				sensor_interface_mem[i] == TYPE_SENSOR_CO_DD)
				break;
		printf("CO interface %d %4x\n",i,sensor_interface_mem[i]);
		printf("CO zero value\n",g_zero_info->cur_co);
		cmd_return_point[5]=i+1;
		cmd_return_point[7]=(g_zero_info->cur_co>>8) & 0xff;
		cmd_return_point[8]=(g_zero_info->cur_co & 0xff);
		crc=CRC_check(cmd_return_point,9);
		for(i=0;i<sizeof(cmd_return_point);i++)
			printf("%02X ",cmd_return_point[i]);
		printf("\n");
		cmd_return_point[9]=(crc&0xff00)>>8;cmd_return_point[10]=crc&0x00ff;
		write(fd_com,cmd_return_point,sizeof(cmd_return_point));
		#endif
	}	
	//cmd_request_verify[5]=i+1;
	crc=CRC_check((unsigned char *)cmd_request_verify,6);
	cmd_request_verify[6]=(crc&0xff00)>>8;cmd_request_verify[7]=crc&0x00ff;		
	for(i=0;i<sizeof(cmd_request_verify);i++)
		printfLog("%02X ",cmd_request_verify[i]);
	printfLog("\n");
	send_cmd_to_cap(cmd_request_verify,sizeof(cmd_request_verify));
	//sleep(1);
	//for(i=0;i<11;i++)
	//	if(sensor_interface_mem[i] == TYPE_SENSOR_CH2O_WEISHEN ||
	//		sensor_interface_mem[i] == TYPE_SENSOR_CH2O_AERSHEN)
	//		break;
	//printf("CH2O interface %d %4x\n",i,sensor_interface_mem[i]);
	//cmd_request_verify[5]=i+1;
	//crc=CRC_check(cmd_request_verify,6);
	//cmd_request_verify[6]=(crc&0xff00)>>8;cmd_request_verify[7]=crc&0x00ff;	
	//for(i=0;i<sizeof(cmd_request_verify);i++)
	//	printf("%02X ",cmd_request_verify[i]);
	//printf("\n");
	//write(fd_com,cmd_request_verify,sizeof(cmd_request_verify));
}
void handle_clean_alarm(int addr)
{
	clear_buf(addr,6);
	switch (addr)
	{
		case ADDR_ALAM_CO:			
			memset(g_share_memory->sensor_alarm_val.co,0,10);
			break;
		case ADDR_ALAM_HCHO:			
			memset(g_share_memory->sensor_alarm_val.hcho,0,10);
			break;
		case ADDR_ALAM_CO2:			
			memset(g_share_memory->sensor_alarm_val.co2,0,10);
			break;
		case ADDR_ALAM_O3:			
			memset(g_share_memory->sensor_alarm_val.o3,0,10);
			break;
		case ADDR_ALAM_TVOC:			
			memset(g_share_memory->sensor_alarm_val.tvoc,0,10);
			break;	
		case ADDR_ALAM_TEMP:			
			memset(g_share_memory->sensor_alarm_val.temp,0,10);
			break;
		case ADDR_ALAM_SHIDU:			
			memset(g_share_memory->sensor_alarm_val.shidu,0,10);
			break;
		case ADDR_ALAM_PM25:			
			memset(g_share_memory->sensor_alarm_val.pm25,0,10);
			break;
		case ADDR_ALAM_PM10:			
			memset(g_share_memory->sensor_alarm_val.pm10,0,10);
			break;
		case ADDR_ALAM_NOISE:			
			memset(g_share_memory->sensor_alarm_val.noise,0,10);
			break;
		default:
			break;
	}
}
void handle_alarm_value()
{
	char val[10]={0};
	int result = 0;
	result = read_dgus(ADDR_ALAM_CO,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.co,0,10);
		strcpy(g_share_memory->sensor_alarm_val.co,val);
		printfLog(LCD_PROCESS"co alarm value %s\n", g_share_memory->sensor_alarm_val.co);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_PM25,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.pm25,0,10);
		strcpy(g_share_memory->sensor_alarm_val.pm25,val);
		printfLog(LCD_PROCESS"pm25 alarm value %s\n", g_share_memory->sensor_alarm_val.pm25);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_PM10,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.pm10,0,10);
		strcpy(g_share_memory->sensor_alarm_val.pm10,val);
		printfLog(LCD_PROCESS"pm10 alarm value %s\n", g_share_memory->sensor_alarm_val.pm10);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_HCHO,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.hcho,0,10);
		strcpy(g_share_memory->sensor_alarm_val.hcho,val);
		printfLog(LCD_PROCESS"hcho alarm value %s\n", g_share_memory->sensor_alarm_val.hcho);
	}
	memset(val,0,10);

	
	result = read_dgus(ADDR_ALAM_O3,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.o3,0,10);
		strcpy(g_share_memory->sensor_alarm_val.o3,val);
		printfLog(LCD_PROCESS"o3 alarm value %s\n", g_share_memory->sensor_alarm_val.o3);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_NOISE,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.noise,0,10);
		strcpy(g_share_memory->sensor_alarm_val.noise,val);
		printfLog(LCD_PROCESS"noise alarm value %s\n", g_share_memory->sensor_alarm_val.noise);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_CO2,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.co2,0,10);
		strcpy(g_share_memory->sensor_alarm_val.co2,val);
		printfLog(LCD_PROCESS"co2 alarm value %s\n", g_share_memory->sensor_alarm_val.co2);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_TEMP,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.temp,0,10);
		strcpy(g_share_memory->sensor_alarm_val.temp,val);
		printfLog(LCD_PROCESS"temp alarm value %s\n", g_share_memory->sensor_alarm_val.temp);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_TVOC,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.tvoc,0,10);
		strcpy(g_share_memory->sensor_alarm_val.tvoc,val);
		printfLog(LCD_PROCESS"tvoc alarm value %s\n", g_share_memory->sensor_alarm_val.tvoc);
	}
	memset(val,0,10);
	result = read_dgus(ADDR_ALAM_SHIDU,6,val);
	if (result && strlen(val) !=0)		
	{
		memset(g_share_memory->sensor_alarm_val.shidu,0,10);
		strcpy(g_share_memory->sensor_alarm_val.shidu,val);
		printfLog(LCD_PROCESS"shidu alarm value %s\n", g_share_memory->sensor_alarm_val.shidu);
	}
	set_alarm_val(SENSOR_ALARM_FILE);
}
void show_alarm_value()
{
	clear_buf(ADDR_ALAM_CO,6);clear_buf(ADDR_ALAM_HCHO,6);clear_buf(ADDR_ALAM_SHIDU,6);
	clear_buf(ADDR_ALAM_CO2,6);clear_buf(ADDR_ALAM_O3,6);clear_buf(ADDR_ALAM_TEMP,6);
	clear_buf(ADDR_ALAM_PM25,6);clear_buf(ADDR_ALAM_NOISE,6);clear_buf(ADDR_ALAM_TVOC,6);
	clear_buf(ADDR_ALAM_PM10,6);
	if (strlen(g_share_memory->sensor_alarm_val.co)!=0)
		write_string(ADDR_ALAM_CO,g_share_memory->sensor_alarm_val.co,
			strlen(g_share_memory->sensor_alarm_val.co));	
	if (strlen(g_share_memory->sensor_alarm_val.pm25)!=0)
		write_string(ADDR_ALAM_PM25,g_share_memory->sensor_alarm_val.pm25,
			strlen(g_share_memory->sensor_alarm_val.pm25));	
	if (strlen(g_share_memory->sensor_alarm_val.hcho)!=0)
		write_string(ADDR_ALAM_HCHO,g_share_memory->sensor_alarm_val.hcho,
			strlen(g_share_memory->sensor_alarm_val.hcho));	
	if (strlen(g_share_memory->sensor_alarm_val.pm10)!=0)
		write_string(ADDR_ALAM_PM10,g_share_memory->sensor_alarm_val.pm10,
			strlen(g_share_memory->sensor_alarm_val.pm10));	
	if (strlen(g_share_memory->sensor_alarm_val.o3)!=0)
		write_string(ADDR_ALAM_O3,g_share_memory->sensor_alarm_val.o3,
			strlen(g_share_memory->sensor_alarm_val.o3));	
	if (strlen(g_share_memory->sensor_alarm_val.noise)!=0)
		write_string(ADDR_ALAM_NOISE,g_share_memory->sensor_alarm_val.noise,
			strlen(g_share_memory->sensor_alarm_val.noise));	
	if (strlen(g_share_memory->sensor_alarm_val.co2)!=0)
		write_string(ADDR_ALAM_CO2,g_share_memory->sensor_alarm_val.co2,
			strlen(g_share_memory->sensor_alarm_val.co2));	
	if (strlen(g_share_memory->sensor_alarm_val.temp)!=0)
		write_string(ADDR_ALAM_TEMP,g_share_memory->sensor_alarm_val.temp,
			strlen(g_share_memory->sensor_alarm_val.temp));	
	if (strlen(g_share_memory->sensor_alarm_val.tvoc)!=0)
		write_string(ADDR_ALAM_TVOC,g_share_memory->sensor_alarm_val.tvoc,
			strlen(g_share_memory->sensor_alarm_val.tvoc));	
	if (strlen(g_share_memory->sensor_alarm_val.shidu)!=0)
		write_string(ADDR_ALAM_SHIDU,g_share_memory->sensor_alarm_val.shidu,
			strlen(g_share_memory->sensor_alarm_val.shidu));	
}
unsigned short input_handle(char *input)
{
	int addr=0,data=0;
	static char wifi_select=0;
	static char verify_object=0;
	static char verify_point = 100;
	static int begin_co=0;
	static int begin_co2=0;
	static int begin_hcho=0;
	static int begin_temp=0;
	static int begin_shidu=0;
	static int begin_pm25=0;
	static int begin_noise=0;
	static int begin_press=0;
	static int begin_wind=0;
	static int begin_o3=0;
	static int begin_tvoc=0;
	static int begin_pm10=0;	
	static int curve_co=0;
	static int curve_co2=0;
	static int curve_hcho=0;
	static int curve_temp=0;
	static int curve_shidu=0;
	static int curve_pm25=0;	
	static int curve_noise=0;
	static int curve_press=0;
	static int curve_wind=0;
	static int curve_o3=0;
	static int curve_tvoc=0;
	static int curve_pm10=0;
	static int interface_config_no = 0;
	static int cur_select_interface = TYPE_SENSOR_CO_WEISHEN;
	input[0]=2;
	addr=input[1]<<8|input[2];
	data=input[4]<<8|input[5];
	printfLog(LCD_PROCESS"got press %04x %04x\r\n",addr,data);
#if 1
		if(lcd_state==0)
		{
			if(g_share_memory->black_lcd)
				lcd_on(g_index);
		}
		else
		{
			alarm(0);
		}
		if(g_share_memory->sleep!=0)
			alarm(g_share_memory->sleep*60);
		else
			alarm(5*60);
#endif
	if((addr==TOUCH_STATE_RETURN_2 && (TOUCH_STATE_RETURN_2+0x100)==data)||
		(addr==TOUCH_STATE_RETURN_1 && (TOUCH_STATE_RETURN_1+0x100)==data)||
		(addr==TOUCH_STATE_RETURN_4 && (TOUCH_STATE_RETURN_4+0x100)==data)||
		(addr==TOUCH_STATE_RETURN_3 && (TOUCH_STATE_RETURN_3+0x100)==data)||
		(addr==TOUCH_PRODUCT_INFO_RETURN && (TOUCH_PRODUCT_INFO_RETURN+0x100)==data)||
		(addr==TOUCH_SYSTEM_SETTING_RETURN && (TOUCH_SYSTEM_SETTING_RETURN+0x100)==data)||
		(addr==TOUCH_HCHO_HISTORY_RETURN && (TOUCH_HCHO_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_CO_HISTORY_RETURN && (TOUCH_CO_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_CO2_HISTORY_RETURN && (TOUCH_CO2_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_O3_HISTORY_RETURN && (TOUCH_O3_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_TVOC_HISTORY_RETURN && (TOUCH_TVOC_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_PM10_HISTORY_RETURN && (TOUCH_PM10_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_PM25_HISTORY_RETURN && (TOUCH_PM25_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_TEMP_HISTORY_RETURN && (TOUCH_TEMP_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_SHIDU_HISTORY_RETURN && (TOUCH_SHIDU_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_NOISE_HISTORY_RETURN && (TOUCH_NOISE_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_PRESS_HISTORY_RETURN && (TOUCH_PRESS_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_WIND_HISTORY_RETURN && (TOUCH_WIND_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_NOISE_RETURN && (TOUCH_NOISE_RETURN+0x100)==data)||
		(addr==TOUCH_CO2_RETURN && (TOUCH_CO2_RETURN+0x100)==data)||
		(addr==TOUCH_CO_RETURN && (TOUCH_CO_RETURN+0x100)==data)||
		(addr==TOUCH_HCHO_RETURN && (TOUCH_HCHO_RETURN+0x100)==data)||
		(addr==TOUCH_PRESS_RETURN && (TOUCH_PRESS_RETURN+0x100)==data)||
		(addr==TOUCH_SHIDU_RETURN && (TOUCH_SHIDU_RETURN+0x100)==data)||
		(addr==TOUCH_PM25_RETURN && (TOUCH_PM25_RETURN+0x100)==data)||
		(addr==TOUCH_PM10_RETURN && (TOUCH_PM10_RETURN+0x100)==data)||
		(addr==TOUCH_O3_RETURN && (TOUCH_O3_RETURN+0x100)==data)||
		(addr==TOUCH_WIND_RETURN && (TOUCH_WIND_RETURN+0x100)==data)||
		(addr==TOUCH_TVOC_RETURN && (TOUCH_TVOC_RETURN+0x100)==data)||
		(addr==TOUCH_TEMP_RETURN && (TOUCH_TEMP_RETURN+0x100)==data))
	{
		if(g_share_memory->ppm)
		{
			switch_pic(MAIN_PAGE_PPM);
			g_index=MAIN_PAGE_PPM;
		}
		else
		{
			switch_pic(MAIN_PAGE);
			g_index=MAIN_PAGE;
		}
		if((addr==TOUCH_HCHO_HISTORY_RETURN && (TOUCH_HCHO_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_CO_HISTORY_RETURN && (TOUCH_CO_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_CO2_HISTORY_RETURN && (TOUCH_CO2_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_O3_HISTORY_RETURN && (TOUCH_O3_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_TVOC_HISTORY_RETURN && (TOUCH_TVOC_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_PM10_HISTORY_RETURN && (TOUCH_PM10_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_PM25_HISTORY_RETURN && (TOUCH_PM25_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_TEMP_HISTORY_RETURN && (TOUCH_TEMP_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_SHIDU_HISTORY_RETURN && (TOUCH_SHIDU_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_NOISE_HISTORY_RETURN && (TOUCH_NOISE_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_PRESS_HISTORY_RETURN && (TOUCH_PRESS_HISTORY_RETURN+0x100)==data)||
		(addr==TOUCH_WIND_HISTORY_RETURN && (TOUCH_WIND_HISTORY_RETURN+0x100)==data))
		{
			begin_co=0;begin_co2=0;begin_hcho=0;begin_noise=0;begin_o3=0;
			begin_pm10=0;begin_pm25=0;begin_press=0;begin_shidu=0;begin_temp=0;
			begin_tvoc=0;begin_wind=0;
		}
	}
	else if((addr==TOUCH_TIME_RETURN && (TOUCH_TIME_RETURN+0x100)==data)||
		(addr==TOUCH_GPRS_OK && (TOUCH_GPRS_OK+0x100)==data)||
		(addr==TOUCH_WIFI_OK && (TOUCH_WIFI_OK+0x100)==data)||
		(addr==TOUCH_USER_INFO_RETURN && (TOUCH_USER_INFO_RETURN+0x100)==data)||
		(addr==TOUCH_FACTORY_INFO_RETURN && (TOUCH_FACTORY_INFO_RETURN+0x100)==data)||
		(addr==TOUCH_XFER_RETURN && (TOUCH_XFER_RETURN+0x100)==data)||
		(addr==TOUCH_SETTING_RETURN && (TOUCH_SETTING_RETURN+0x100)==data)||
		(addr==TOUCH_RETURN_ALAM && (TOUCH_RETURN_ALAM+0x100)==data))
	{
		if(addr==TOUCH_SETTING_RETURN && (TOUCH_SETTING_RETURN+0x100)==data)
			ctl_fan(1);
		switch_pic(SYSTEM_SETTING_PAGE);
		g_index=SYSTEM_SETTING_PAGE;
	}
	else if(addr==TOUCH_FACTORY_INFO && (TOUCH_FACTORY_INFO+0x100)==data)
	{
		switch_pic(FACTORY_PAGE);
		g_index=FACTORY_PAGE;
	}
	else if(addr==TOUCH_USER_INFO && (TOUCH_USER_INFO+0x100)==data)
	{
		switch_pic(USER_INFO_PAGE);
		g_index=USER_INFO_PAGE;
	}
	else if((addr==TOUCH_TEST_GPRS_2 && (TOUCH_TEST_GPRS_2+0x100)==data)||
		(addr==TOUCH_TEST_GPRS_1 && (TOUCH_TEST_GPRS_1+0x100)==data)||
		(addr==TOUCH_TEST_GPRS_3 && (TOUCH_TEST_GPRS_3+0x100)==data)||
		(addr==TOUCH_TEST_GPRS_4 && (TOUCH_TEST_GPRS_4+0x100)==data))
	{
		switch_pic(100);
		if(ping_server_by_gprs())
		{
			printfLog("ping server by gprs ok\n");
			switch_pic(g_index);
			gprs_state(1,g_index);
		}
		else
		{
			printfLog("ping server by gprs failed\n");
			switch_pic(g_index);
			gprs_state(0,g_index);
		}
	}	
	else if((addr==TOUCH_SYSTEM_SETTING_1 && (TOUCH_SYSTEM_SETTING_1+0x100)==data)||
		(addr==TOUCH_SYSTEM_SETTING_2 && (TOUCH_SYSTEM_SETTING_2+0x100)==data))
	{
			switch_pic(SYSTEM_SETTING_PAGE);
			g_index=SYSTEM_SETTING_PAGE;
	}
	else if(addr==TOUCH_CONVERSION_1 && (TOUCH_CONVERSION_1+0x100)==data)
	{
			g_share_memory->ppm=1;
			switch_pic(MAIN_PAGE_PPM);
			g_index=MAIN_PAGE_PPM;
	}
	else if(addr==TOUCH_CONVERSION_2 && (TOUCH_CONVERSION_2+0x100)==data)
	{
			g_share_memory->ppm=0;
			switch_pic(MAIN_PAGE);
			g_index=MAIN_PAGE;
	}
	else if((addr==TOUCH_CO_REAL_1 && (TOUCH_CO_REAL_1+0x100)==data)||
		(addr==TOUCH_CO_REAL_2 && (TOUCH_CO_REAL_2+0x100)==data))
	{//show history CO the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_CO);
				show_curve(ID_CAP_CO,&curve_co);
				g_index=CURVE_PAGE_CO;
			}
			else
			{
				switch_pic(LIST_PAGE_CO);
				begin_co=0;
				memset(g_history_log[SENSOR_CO],0,1024);
				g_history_index[SENSOR_CO]=0;
				g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=0;
				(g_history_index[SENSOR_CO])++;
				begin_co=show_history(ID_CAP_CO,begin_co,g_history_index[SENSOR_CO]-1);
				g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=begin_co;		
				g_index=LIST_PAGE_CO;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_CO;
			else
				g_index=LIST_PAGE_CO;
		}*/
	}
	else if((addr==TOUCH_PRESS_REAL_1 && (TOUCH_PRESS_REAL_1+0x100)==data)||
		(addr==TOUCH_PRESS_REAL_2 && (TOUCH_PRESS_REAL_2+0x100)==data))
	{//show history QIYA the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_PRESS);
				show_curve(ID_CAP_QI_YA,&curve_press);				
				g_index=CURVE_PAGE_PRESS;
			}
			else
			{
				switch_pic(LIST_PAGE_PRESS);
				begin_press=0;
				memset(g_history_log[SENSOR_PRESS],0,1024);
				g_history_index[SENSOR_PRESS]=0;
				g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=0;
				(g_history_index[SENSOR_PRESS])++;
				begin_press=show_history(ID_CAP_QI_YA,begin_press,g_history_index[SENSOR_PRESS]-1);
				g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=begin_press;		
				g_index=LIST_PAGE_PRESS;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_PRESS;
			else
				g_index=LIST_PAGE_PRESS;
		}*/
	}
	else if((addr==TOUCH_TVOC_REAL_1 && (TOUCH_TVOC_REAL_1+0x100)==data)||
		(addr==TOUCH_TVOC_REAL_2 && (TOUCH_TVOC_REAL_2+0x100)==data))
	{//show history TVOC the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_TVOC);
				show_curve(ID_CAP_TVOC,&curve_tvoc);
				g_index=CURVE_PAGE_TVOC;
			}
			else
			{
				switch_pic(LIST_PAGE_TVOC);
				begin_tvoc=0;
				memset(g_history_log[SENSOR_TVOC],0,1024);
				g_history_index[SENSOR_TVOC]=0;
				g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=0;
				(g_history_index[SENSOR_TVOC])++;
				begin_tvoc=show_history(ID_CAP_TVOC,begin_tvoc,g_history_index[SENSOR_TVOC]-1);
				g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=begin_tvoc;		
				g_index=LIST_PAGE_TVOC;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_TVOC;
			else
				g_index=LIST_PAGE_TVOC;
		}*/
	}
	else if((addr==TOUCH_PM10_REAL_1 && (TOUCH_PM10_REAL_1+0x100)==data)||
		(addr==TOUCH_PM10_REAL_2 && (TOUCH_PM10_REAL_2+0x100)==data))
	{//show history PM10 the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_PM10);
				show_curve(ID_CAP_PM_10,&curve_pm10);
				g_index=CURVE_PAGE_PM10;
			}
			else
			{
				switch_pic(LIST_PAGE_PM10);
				begin_pm10=0;
				memset(g_history_log[SENSOR_PM10],0,1024);
				g_history_index[SENSOR_PM10]=0;
				g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=0;
				(g_history_index[SENSOR_PM10])++;
				begin_pm10=show_history(ID_CAP_PM_10,begin_pm10,g_history_index[SENSOR_PM10]-1);
				g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=begin_pm10;		
				g_index=LIST_PAGE_PM10;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_PM10;
			else
				g_index=LIST_PAGE_PM10;
		}	*/	
	}
	else if((addr==TOUCH_O3_REAL_1 && (TOUCH_O3_REAL_1+0x100)==data)||
		(addr==TOUCH_O3_REAL_2 && (TOUCH_O3_REAL_2+0x100)==data))
	{//show history O3 the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_O3);
				show_curve(ID_CAP_CHOU_YANG,&curve_o3);
				g_index=CURVE_PAGE_O3;
			}
			else
			{
				switch_pic(LIST_PAGE_O3);
				begin_o3=0;
				memset(g_history_log[SENSOR_O3],0,1024);
				g_history_index[SENSOR_O3]=0;
				g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=0;
				(g_history_index[SENSOR_O3])++;
				begin_o3=show_history(ID_CAP_CHOU_YANG,begin_o3,g_history_index[SENSOR_O3]-1);
				g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=begin_o3;		
				g_index=LIST_PAGE_O3;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_O3;
			else
				g_index=LIST_PAGE_O3;
		}*/		
	}
	else if((addr==TOUCH_WIND_REAL_1 && (TOUCH_WIND_REAL_1+0x100)==data)||
		(addr==TOUCH_WIND_REAL_2 && (TOUCH_WIND_REAL_2+0x100)==data))
	{//show history WIND the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_WIND);
				show_curve(ID_CAP_FENG_SU,&curve_wind);
				g_index=CURVE_PAGE_WIND;
			}
			else
			{
				switch_pic(LIST_PAGE_WIND);
				begin_wind=0;
				memset(g_history_log[SENSOR_WIND],0,1024);
				g_history_index[SENSOR_WIND]=0;
				g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=0;
				(g_history_index[SENSOR_WIND])++;
				begin_wind=show_history(ID_CAP_FENG_SU,begin_wind,g_history_index[SENSOR_WIND]-1);
				g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=begin_wind;		
				g_index=LIST_PAGE_WIND;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_WIND;
			else
				g_index=LIST_PAGE_WIND;
		}	*/	
	}
	else if((addr==TOUCH_NOISE_REAL_1 && (TOUCH_NOISE_REAL_1+0x100)==data)||
		(addr==TOUCH_NOISE_REAL_2 && (TOUCH_NOISE_REAL_2+0x100)==data))
	{//show history NOISE the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_NOISE);
				show_curve(ID_CAP_BUZZY,&curve_noise);
				g_index=CURVE_PAGE_NOISE;
			}
			else
			{
				switch_pic(LIST_PAGE_NOISE);
				begin_noise=0;
				memset(g_history_log[SENSOR_NOISE],0,1024);
				g_history_index[SENSOR_NOISE]=0;
				g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=0;
				(g_history_index[SENSOR_NOISE])++;
				begin_noise=show_history(ID_CAP_BUZZY,begin_noise,g_history_index[SENSOR_NOISE]-1);
				g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=begin_noise;		
				g_index=LIST_PAGE_NOISE;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_NOISE;
			else
				g_index=LIST_PAGE_NOISE;
		}	*/	
	}
	else if((addr==TOUCH_CO2_REAL_1 && (TOUCH_CO2_REAL_1+0x100)==data)||
		(addr==TOUCH_CO2_REAL_2 && (TOUCH_CO2_REAL_2+0x100)==data))
	{//show history CO2 the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_CO2);
				show_curve(ID_CAP_CO2,&curve_co2);
				g_index=CURVE_PAGE_CO2;
			}
			else
			{
				switch_pic(LIST_PAGE_CO2);
				begin_co2=0;
				memset(g_history_log[SENSOR_CO2],0,1024);
				g_history_index[SENSOR_CO2]=0;
				g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=0;
				(g_history_index[SENSOR_CO2])++;
				begin_co2=show_history(ID_CAP_CO2,begin_co2,g_history_index[SENSOR_CO2]-1);
				g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=begin_co2;		
				g_index=LIST_PAGE_CO2;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_CO2;
			else
				g_index=LIST_PAGE_CO2;
		}*/		
	}	
	else if((addr==TOUCH_HCHO_REAL_1 && (TOUCH_HCHO_REAL_1+0x100)==data)||
		(addr==TOUCH_HCHO_REAL_2 && (TOUCH_HCHO_REAL_2+0x100)==data))
	{//show history HCHO the first page	
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_HCHO);
				show_curve(ID_CAP_HCHO,&curve_hcho);
				g_index=CURVE_PAGE_HCHO;
			}
			else
			{
				switch_pic(LIST_PAGE_HCHO);
				begin_hcho=0;
				memset(g_history_log[SENSOR_HCHO],0,1024);
				g_history_index[SENSOR_HCHO]=0;
				g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=0;
				(g_history_index[SENSOR_HCHO])++;
				begin_hcho=show_history(ID_CAP_HCHO,begin_hcho,g_history_index[SENSOR_HCHO]-1);
				g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=begin_hcho;		
				g_index=LIST_PAGE_HCHO;
			}
		}
		/*else
		{		
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_HCHO;
			else
				g_index=LIST_PAGE_HCHO;
		}	*/	
	}	
	else if((addr==TOUCH_SHIDU_REAL_1 && (TOUCH_SHIDU_REAL_1+0x100)==data)||
		(addr==TOUCH_SHIDU_REAL_2 && (TOUCH_SHIDU_REAL_2+0x100)==data))
	{//show history SHIDU the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_SHIDU);
				show_curve(ID_CAP_SHI_DU,&curve_shidu);
				g_index=CURVE_PAGE_SHIDU;
			}
			else
			{
				switch_pic(LIST_PAGE_SHIDU);
				begin_shidu=0;
				memset(g_history_log[SENSOR_SHIDU],0,1024);
				g_history_index[SENSOR_SHIDU]=0;
				g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=0;
				(g_history_index[SENSOR_SHIDU])++;
				begin_shidu=show_history(ID_CAP_SHI_DU,begin_shidu,g_history_index[SENSOR_SHIDU]-1);
				g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=begin_shidu; 
				g_index=LIST_PAGE_SHIDU;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_SHIDU;
			else
				g_index=LIST_PAGE_SHIDU;
		}	*/			
	}	
	else if((addr==TOUCH_TEMP_REAL_1 && (TOUCH_TEMP_REAL_1+0x100)==data)||
		(addr==TOUCH_TEMP_REAL_2 && (TOUCH_TEMP_REAL_2+0x100)==data))
	{//show history TEMPERATURE the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_TEMP);
				show_curve(ID_CAP_TEMPERATURE,&curve_temp);
				g_index=CURVE_PAGE_TEMP;
			}
			else
			{
				switch_pic(LIST_PAGE_TEMP);
				begin_temp=0;
				memset(g_history_log[SENSOR_TEMP],0,1024);
				g_history_index[SENSOR_TEMP]=0;
				g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=0;
				(g_history_index[SENSOR_TEMP])++;
				begin_temp=show_history(ID_CAP_TEMPERATURE,begin_temp,g_history_index[SENSOR_TEMP]-1);
				g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=begin_temp;
				printfLog(LCD_PROCESS"begin_temp in main is %d,page %d\n",g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]],
					g_history_index[SENSOR_TEMP]);
				g_index=LIST_PAGE_TEMP;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_TEMP;
			else
				g_index=LIST_PAGE_TEMP;
		}	*/			
	}	
	else if((addr==TOUCH_PM25_REAL_1&& (TOUCH_PM25_REAL_1+0x100)==data)||
		(addr==TOUCH_PM25_REAL_2&& (TOUCH_PM25_REAL_2+0x100)==data))
	{//show history PM25 the first page
		//if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE_PM25);
				show_curve(ID_CAP_PM_25,&curve_pm25);
				g_index=CURVE_PAGE_PM25;
			}
			else
			{
				switch_pic(LIST_PAGE_PM25);				
				begin_pm25=0;
				memset(g_history_log[SENSOR_PM25],0,1024);
				g_history_index[SENSOR_PM25]=0;
				g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=0;
				(g_history_index[SENSOR_PM25])++;
				begin_pm25=show_history(ID_CAP_PM_25,begin_pm25,g_history_index[SENSOR_PM25]-1);
				g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=begin_pm25;
				
				g_index=LIST_PAGE_PM25;
			}
		}
		/*else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;
			if(g_share_memory->history_done)
				g_index=CURVE_PAGE_PM25;
			else
				g_index=LIST_PAGE_PM25;
		}	*/	
	}	
	else if(addr==TOUCH_CO_UPDATE && (TOUCH_CO_UPDATE+0x100)==data)
	{//show history CO the next page
		begin_co=0;
		memset(g_history_log[SENSOR_CO],0,1024);
		g_history_index[SENSOR_CO]=0;
		g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=0;
		(g_history_index[SENSOR_CO])++;
		begin_co=show_history(ID_CAP_CO,begin_co,g_history_index[SENSOR_CO]-1);
		g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=begin_co;		
	}
	else if(addr==TOUCH_O3_UPDATE && (TOUCH_O3_UPDATE+0x100)==data)
	{//show history O3 the next page
		begin_o3=0;
		memset(g_history_log[SENSOR_O3],0,1024);
		g_history_index[SENSOR_O3]=0;
		g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=0;
		(g_history_index[SENSOR_O3])++;
		begin_o3=show_history(ID_CAP_CHOU_YANG,begin_o3,g_history_index[SENSOR_O3]-1);
		g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=begin_o3;		
	}
	else if(addr==TOUCH_PRESS_UPDATE && (TOUCH_PRESS_UPDATE+0x100)==data)
	{//show history PRESS the next page
		begin_press=0;
		memset(g_history_log[SENSOR_PRESS],0,1024);
		g_history_index[SENSOR_PRESS]=0;
		g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=0;
		(g_history_index[SENSOR_PRESS])++;
		begin_press=show_history(ID_CAP_QI_YA,begin_press,g_history_index[SENSOR_PRESS]-1);
		g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=begin_press; 	
	}
	else if(addr==TOUCH_TVOC_UPDATE && (TOUCH_TVOC_UPDATE+0x100)==data)
	{//show history TVOC the next page
		begin_tvoc=0;
		memset(g_history_log[SENSOR_TVOC],0,1024);
		g_history_index[SENSOR_TVOC]=0;
		g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=0;
		(g_history_index[SENSOR_TVOC])++;
		begin_tvoc=show_history(ID_CAP_TVOC,begin_tvoc,g_history_index[SENSOR_TVOC]-1);
		g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=begin_tvoc;		
	}
	else if(addr==TOUCH_PM10_UPDATE && (TOUCH_PM10_UPDATE+0x100)==data)
	{//show history PM10 the next page
		begin_pm10=0;
		memset(g_history_log[SENSOR_PM10],0,1024);
		g_history_index[SENSOR_PM10]=0;
		g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=0;
		(g_history_index[SENSOR_PM10])++;
		begin_pm10=show_history(ID_CAP_PM_10,begin_pm10,g_history_index[SENSOR_PM10]-1);
		g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=begin_pm10;		
	}
	else if(addr==TOUCH_WIND_UPDATE && (TOUCH_WIND_UPDATE+0x100)==data)
	{//show history WIND the next page
		begin_wind=0;
		memset(g_history_log[SENSOR_WIND],0,1024);
		g_history_index[SENSOR_WIND]=0;
		g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=0;
		(g_history_index[SENSOR_WIND])++;
		begin_wind=show_history(ID_CAP_FENG_SU,begin_wind,g_history_index[SENSOR_WIND]-1);
		g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=begin_wind;		
	}
	else if(addr==TOUCH_NOISE_UPDATE && (TOUCH_NOISE_UPDATE+0x100)==data)
	{//show history NOISE the next page
		begin_noise=0;
		memset(g_history_log[SENSOR_NOISE],0,1024);
		g_history_index[SENSOR_NOISE]=0;
		g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=0;
		(g_history_index[SENSOR_NOISE])++;
		begin_noise=show_history(ID_CAP_BUZZY,begin_noise,g_history_index[SENSOR_NOISE]-1);
		g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=begin_noise; 	
	}
	else if(addr==TOUCH_CO_LAST_PAGE && (TOUCH_CO_LAST_PAGE+0x100)==data)
	{//show history CO the next page
		if(g_history_index[SENSOR_CO]>1)
		{
			g_history_index[SENSOR_CO]-=2;
			begin_co=g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]];
			begin_co+=show_history(ID_CAP_CO,begin_co,g_history_index[SENSOR_CO]);
			(g_history_index[SENSOR_CO])++;
			g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=begin_co;
		}
	}
	else if(addr==TOUCH_NOISE_LAST_PAGE && (TOUCH_NOISE_LAST_PAGE+0x100)==data)
	{//show history NOISE the next page
		if(g_history_index[SENSOR_NOISE]>1)
		{
			g_history_index[SENSOR_NOISE]-=2;
			begin_noise=g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]];
			begin_noise+=show_history(ID_CAP_BUZZY,begin_noise,g_history_index[SENSOR_NOISE]);
			(g_history_index[SENSOR_NOISE])++;
			g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=begin_noise;
		}
	}
	else if(addr==TOUCH_PRESS_LAST_PAGE && (TOUCH_PRESS_LAST_PAGE+0x100)==data)
	{//show history PRESS the next page
		if(g_history_index[SENSOR_PRESS]>1)
		{
			g_history_index[SENSOR_PRESS]-=2;
			begin_press=g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]];
			begin_press+=show_history(ID_CAP_QI_YA,begin_press,g_history_index[SENSOR_PRESS]);
			(g_history_index[SENSOR_PRESS])++;
			g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=begin_press;
		}
	}
	else if(addr==TOUCH_WIND_LAST_PAGE && (TOUCH_WIND_LAST_PAGE+0x100)==data)
	{//show history WIND the next page
		if(g_history_index[SENSOR_WIND]>1)
		{
			g_history_index[SENSOR_WIND]-=2;
			begin_wind=g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]];
			begin_wind+=show_history(ID_CAP_FENG_SU,begin_wind,g_history_index[SENSOR_WIND]);
			(g_history_index[SENSOR_WIND])++;
			g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=begin_wind;
		}
	}
	else if(addr==TOUCH_O3_LAST_PAGE && (TOUCH_O3_LAST_PAGE+0x100)==data)
	{//show history O3 the next page
		if(g_history_index[SENSOR_O3]>1)
		{
			g_history_index[SENSOR_O3]-=2;
			begin_o3=g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]];
			begin_o3+=show_history(ID_CAP_CHOU_YANG,begin_o3,g_history_index[SENSOR_O3]);
			(g_history_index[SENSOR_O3])++;
			g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=begin_o3;
		}
	}
	else if(addr==TOUCH_TVOC_LAST_PAGE && (TOUCH_TVOC_LAST_PAGE+0x100)==data)
	{//show history TVOC the next page
		if(g_history_index[SENSOR_TVOC]>1)
		{
			g_history_index[SENSOR_TVOC]-=2;
			begin_tvoc=g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]];
			begin_tvoc+=show_history(ID_CAP_TVOC,begin_tvoc,g_history_index[SENSOR_TVOC]);
			(g_history_index[SENSOR_TVOC])++;
			g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=begin_tvoc;
		}
	}
	else if(addr==TOUCH_PM10_LAST_PAGE && (TOUCH_PM10_LAST_PAGE+0x100)==data)
	{//show history PM10 the next page
		if(g_history_index[SENSOR_PM10]>1)
		{
			g_history_index[SENSOR_PM10]-=2;
			begin_pm10=g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]];
			begin_pm10+=show_history(ID_CAP_PM_10,begin_pm10,g_history_index[SENSOR_PM10]);
			(g_history_index[SENSOR_PM10])++;
			g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=begin_pm10;
		}
	}
	else if(addr==TOUCH_CO_NEXT_PAGE && (TOUCH_CO_NEXT_PAGE+0x100)==data)
	{//show history CO the next page
		if(begin_co<g_share_memory->cnt[SENSOR_CO])
		{
			begin_co+=show_history(ID_CAP_CO,begin_co,g_history_index[SENSOR_CO]);
			(g_history_index[SENSOR_CO])++;
			g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=begin_co;
		}
	}
	else if(addr==TOUCH_PM10_NEXT_PAGE && (TOUCH_PM10_NEXT_PAGE+0x100)==data)
	{//show history PM10 the next page
		if(begin_pm10<g_share_memory->cnt[SENSOR_PM10])
		{
			begin_pm10+=show_history(ID_CAP_PM_10,begin_pm10,g_history_index[SENSOR_PM10]);
			(g_history_index[SENSOR_PM10])++;
			g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=begin_pm10;
		}
	}
	else if(addr==TOUCH_TVOC_NEXT_PAGE && (TOUCH_TVOC_NEXT_PAGE+0x100)==data)
	{//show history TVOC the next page
		if(begin_tvoc<g_share_memory->cnt[SENSOR_TVOC])
		{
			begin_tvoc+=show_history(ID_CAP_TVOC,begin_tvoc,g_history_index[SENSOR_TVOC]);
			(g_history_index[SENSOR_TVOC])++;
			g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=begin_tvoc;
		}
	}
	else if(addr==TOUCH_O3_NEXT_PAGE && (TOUCH_O3_NEXT_PAGE+0x100)==data)
	{//show history O3 the next page
		if(begin_o3<g_share_memory->cnt[SENSOR_O3])
		{
			begin_o3+=show_history(ID_CAP_CHOU_YANG,begin_o3,g_history_index[SENSOR_O3]);
			(g_history_index[SENSOR_O3])++;
			g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=begin_o3;
		}
	}
	else if(addr==TOUCH_PRESS_NEXT_PAGE && (TOUCH_PRESS_NEXT_PAGE+0x100)==data)
	{//show history PRESS the next page
		if(begin_press<g_share_memory->cnt[SENSOR_PRESS])
		{
			begin_press+=show_history(ID_CAP_QI_YA,begin_press,g_history_index[SENSOR_PRESS]);
			(g_history_index[SENSOR_PRESS])++;
			g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=begin_press;
		}
	}
	else if(addr==TOUCH_WIND_NEXT_PAGE && (TOUCH_WIND_NEXT_PAGE+0x100)==data)
	{//show history WIND the next page
		if(begin_wind<g_share_memory->cnt[SENSOR_WIND])
		{
			begin_wind+=show_history(ID_CAP_FENG_SU,begin_wind,g_history_index[SENSOR_WIND]);
			(g_history_index[SENSOR_WIND])++;
			g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=begin_wind;
		}
	}
	else if(addr==TOUCH_NOISE_NEXT_PAGE && (TOUCH_NOISE_NEXT_PAGE+0x100)==data)
	{//show history NOISE the next page
		if(begin_noise<g_share_memory->cnt[SENSOR_NOISE])
		{
			begin_noise+=show_history(ID_CAP_BUZZY,begin_noise,g_history_index[SENSOR_NOISE]);
			(g_history_index[SENSOR_NOISE])++;
			g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=begin_noise;
		}
	}
	else if(addr==TOUCH_CO2_UPDATE && (TOUCH_CO2_UPDATE+0x100)==data)
	{//show history CO2 the next page
		begin_co2=0;
		memset(g_history_log[SENSOR_CO2],0,1024);
		g_history_index[SENSOR_CO2]=0;
		g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=0;
		(g_history_index[SENSOR_CO2])++;
		begin_co2=show_history(ID_CAP_CO2,begin_co2,g_history_index[SENSOR_CO2]-1);
		g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=begin_co2;		
	}
	else if(addr==TOUCH_CO2_LAST_PAGE && (TOUCH_CO2_LAST_PAGE+0x100)==data)
	{//show history CO2 the next page
		if(g_history_index[SENSOR_CO2]>1)
		{
			g_history_index[SENSOR_CO2]-=2;
			begin_co2=g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]];
			begin_co2+=show_history(ID_CAP_CO2,begin_co2,g_history_index[SENSOR_CO2]);
			(g_history_index[SENSOR_CO2])++;
			g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=begin_co2;
		}
	}
	else if(addr==TOUCH_CO2_NEXT_PAGE && (TOUCH_CO2_NEXT_PAGE+0x100)==data)
	{//show history CO2 the next page
		if(begin_co2<g_share_memory->cnt[SENSOR_CO2])
		{
			begin_co2+=show_history(ID_CAP_CO2,begin_co2,g_history_index[SENSOR_CO2]);
			(g_history_index[SENSOR_CO2])++;
			g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=begin_co2;
		}
	}
	else if(addr==TOUCH_HCHO_UPDATE && (TOUCH_HCHO_UPDATE+0x100)==data)
	{//show history HCHO the next page
		begin_hcho=0;
		memset(g_history_log[SENSOR_HCHO],0,1024);
		g_history_index[SENSOR_HCHO]=0;
		g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=0;
		(g_history_index[SENSOR_HCHO])++;
		begin_hcho=show_history(ID_CAP_HCHO,begin_hcho,g_history_index[SENSOR_HCHO]-1);
		g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=begin_hcho;		
	}
	else if(addr==TOUCH_HCHO_LAST_PAGE && (TOUCH_HCHO_LAST_PAGE+0x100)==data)
	{//show history HCHO the next page
		if(g_history_index[SENSOR_HCHO]>1)
		{
			g_history_index[SENSOR_HCHO]-=2;
			begin_hcho=g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]];
			begin_hcho+=show_history(ID_CAP_HCHO,begin_hcho,g_history_index[SENSOR_HCHO]);
			(g_history_index[SENSOR_HCHO])++;
			g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=begin_hcho;
		}
	}
	else if(addr==TOUCH_HCHO_NEXT_PAGE && (TOUCH_HCHO_NEXT_PAGE+0x100)==data)
	{//show history HCHO the next page
		if(begin_hcho<g_share_memory->cnt[SENSOR_HCHO])
		{
			begin_hcho+=show_history(ID_CAP_HCHO,begin_hcho,g_history_index[SENSOR_HCHO]);
			(g_history_index[SENSOR_HCHO])++;
			g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=begin_hcho;
		}
	}
	else if(addr==TOUCH_TEMP_UPDATE && (TOUCH_TEMP_UPDATE+0x100)==data)
	{//show history TEMPERATURE the next page
		begin_temp=0;
		memset(g_history_log[SENSOR_TEMP],0,1024);
		g_history_index[SENSOR_TEMP]=0;
		g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=0;
		(g_history_index[SENSOR_TEMP])++;
		begin_temp=show_history(ID_CAP_TEMPERATURE,begin_temp,g_history_index[SENSOR_TEMP]-1);
		g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=begin_temp;
		printfLog(LCD_PROCESS"begin_temp in update is %d,page %d\n",g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]],
			g_history_index[SENSOR_TEMP]);
	}
	else if(addr==TOUCH_TEMP_LAST_PAGE && (TOUCH_TEMP_LAST_PAGE+0x100)==data)
	{//show history TEMP the next page
		if(g_history_index[SENSOR_TEMP]>1)
		{
			g_history_index[SENSOR_TEMP]-=2;
			begin_temp=g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]];
			begin_temp+=show_history(ID_CAP_TEMPERATURE,begin_temp,g_history_index[SENSOR_TEMP]);
			(g_history_index[SENSOR_TEMP])++;
			g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=begin_temp;
			printfLog(LCD_PROCESS"begin_temp in last begin is %d page %d\n",g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]
				,g_history_index[SENSOR_TEMP]);
		}
	}
	else if(addr==TOUCH_TEMP_NEXT_PAGE && (TOUCH_TEMP_NEXT_PAGE+0x100)==data)
	{//show history TEMP the next page
		if(begin_temp<g_share_memory->cnt[SENSOR_TEMP])
		{
			begin_temp+=show_history(ID_CAP_TEMPERATURE,begin_temp,g_history_index[SENSOR_TEMP]);
			(g_history_index[SENSOR_TEMP])++;
			g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=begin_temp;
			printfLog(LCD_PROCESS"begin_temp in next begin is %d page %d \n",
				g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]],g_history_index[SENSOR_TEMP]);
		}
	}
	else if(addr==TOUCH_SHIDU_UPDATE&& (TOUCH_SHIDU_UPDATE+0x100)==data)
	{//show history SHIDU the next page
		begin_shidu=0;
		memset(g_history_log[SENSOR_SHIDU],0,1024);
		g_history_index[SENSOR_SHIDU]=0;
		g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=0;
		(g_history_index[SENSOR_SHIDU])++;
		begin_shidu=show_history(ID_CAP_SHI_DU,begin_shidu,g_history_index[SENSOR_SHIDU]-1);
		g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=begin_shidu; 
	}
	else if(addr==TOUCH_SHIDU_LAST_PAGE && (TOUCH_SHIDU_LAST_PAGE+0x100)==data)
	{//show history SHIDU the next page
		if(g_history_index[SENSOR_SHIDU]>1)
		{
			g_history_index[SENSOR_SHIDU]-=2;
			begin_shidu=g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]];
			begin_shidu+=show_history(ID_CAP_SHI_DU,begin_shidu,g_history_index[SENSOR_SHIDU]);
			(g_history_index[SENSOR_SHIDU])++;
			g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=begin_shidu;
		}
	}
	else if(addr==TOUCH_SHIDU_NEXT_PAGE && (TOUCH_SHIDU_NEXT_PAGE+0x100)==data)
	{//show history SHIDU the next page
		if(begin_shidu<g_share_memory->cnt[SENSOR_SHIDU])
		{
			begin_shidu+=show_history(ID_CAP_SHI_DU,begin_shidu,g_history_index[SENSOR_SHIDU]);
			(g_history_index[SENSOR_SHIDU])++;
			g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=begin_shidu;
		}
	}
	else if(addr==TOUCH_PM25_UPDATE && (TOUCH_PM25_UPDATE+0x100)==data)
	{//show history PM25 the next page
		begin_pm25=0;
		memset(g_history_log[SENSOR_PM25],0,1024);
		g_history_index[SENSOR_PM25]=0;
		g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=0;
		(g_history_index[SENSOR_PM25])++;
		begin_pm25=show_history(ID_CAP_PM_25,begin_pm25,g_history_index[SENSOR_PM25]-1);
		g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=begin_pm25;
	}
	else if(addr==TOUCH_PM25_LAST_PAGE && (TOUCH_PM25_LAST_PAGE+0x100)==data)
	{//show history PM25 the next page
		if(g_history_index[SENSOR_PM25]>1)
		{
			g_history_index[SENSOR_PM25]-=2;
			begin_pm25=g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]];
			begin_pm25+=show_history(ID_CAP_PM_25,begin_pm25,g_history_index[SENSOR_PM25]);
			(g_history_index[SENSOR_PM25])++;
			g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=begin_pm25;
		}
	}
	else if(addr==TOUCH_PM25_NEXT_PAGE && (TOUCH_PM25_NEXT_PAGE+0x100)==data)
	{//show history PM25 the next page
		if(begin_pm25<g_share_memory->cnt[SENSOR_PM25])
		{
			begin_pm25+=show_history(ID_CAP_PM_25,begin_pm25,g_history_index[SENSOR_PM25]);
			(g_history_index[SENSOR_PM25])++;
			g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=begin_pm25;
		}
	}
	else if((addr==TOUCH_DEVICE_STATE_1&& (TOUCH_DEVICE_STATE_1+0x100)==data)||
		(addr==TOUCH_DEVICE_STATE_2&& (TOUCH_DEVICE_STATE_2+0x100)==data))
	{//show sensor and network state
		show_sensor_network();
	}
	else if(addr==TOUCH_TIME_SETTING&& (TOUCH_TIME_SETTING+0x100)==data)
	{//set time
		clear_buf(ADDR_YEAR,4);
		clear_buf(ADDR_MON,2);
		clear_buf(ADDR_DAY,2);
		clear_buf(ADDR_HOUR,2);
		clear_buf(ADDR_MIN,2);
		clear_buf(ADDR_SEC,2);
		switch_pic(TIME_SETTING_PAGE);
		g_index=TIME_SETTING_PAGE;
	}
	else if(addr==TOUCH_XFER_OK&& (TOUCH_XFER_OK+0x100)==data)
	{//WiFi Passwd changed
		g_share_memory->send_by_wifi=wifi_select;
		if(wifi_select)
		{
			wifi_handle();
			switch_pic(WIFI_DONE_PAGE);
			g_index=WIFI_DONE_PAGE;
		}
		else
		{
			switch_pic(GPRS_DONE_PAGE);
			g_index=GPRS_DONE_PAGE;
		}
		set_net_interface();		
	}		
	else if(addr==TOUCH_ALAM_SET&& (TOUCH_ALAM_SET+0x100)==data)
	{//setting alarm value
		show_alarm_value();
		switch_pic(ALARM_SETTING_PAGE);
		g_index=ALARM_SETTING_PAGE;
	}
	else if(addr==TOUCH_SET_ALAM&& (TOUCH_SET_ALAM+0x100)==data)
	{//setting alarm value
		handle_alarm_value();
		show_main_alarm();
		switch_pic(SYSTEM_SETTING_PAGE);
		g_index=SYSTEM_SETTING_PAGE;
	}
	else if(addr==TOUCH_CLEAN_ALARM_CO && (TOUCH_CLEAN_ALARM_CO+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_CO);
	}	
	else if(addr==TOUCH_CLEAN_ALARM_CO2 && (TOUCH_CLEAN_ALARM_CO2+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_CO2);
	}
	else if(addr==TOUCH_CLEAN_ALARM_HCHO && (TOUCH_CLEAN_ALARM_HCHO+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_HCHO);
	}
	else if(addr==TOUCH_CLEAN_ALARM_TEMP && (TOUCH_CLEAN_ALARM_TEMP+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_TEMP);
	}
	else if(addr==TOUCH_CLEAN_ALARM_O3 && (TOUCH_CLEAN_ALARM_O3+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_O3);
	}
	else if(addr==TOUCH_CLEAN_ALARM_PM25 && (TOUCH_CLEAN_ALARM_PM25+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_PM25);
	}
	else if(addr==TOUCH_CLEAN_ALARM_PM10 && (TOUCH_CLEAN_ALARM_PM10+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_PM10);
	}
	else if(addr==TOUCH_CLEAN_ALARM_TVOC && (TOUCH_CLEAN_ALARM_TVOC+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_TVOC);
	}
	else if(addr==TOUCH_CLEAN_ALARM_NOISE && (TOUCH_CLEAN_ALARM_NOISE+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_NOISE);
	}
	else if(addr==TOUCH_CLEAN_ALARM_SHIDU && (TOUCH_CLEAN_ALARM_SHIDU+0x100)==data)
	{
		handle_clean_alarm(ADDR_ALAM_SHIDU);
	}
	else if((addr==TOUCH_SEL_WIFI)&& (TOUCH_SEL_WIFI+0x100)==data)
	{//WiFi Passwd changed
		wifi_select=1;
		write_string(ADDR_XFER_SELECT,"WIFI",strlen("WIFI"));
	}
	else if(addr==TOUCH_XFER_RETURN&& (TOUCH_XFER_RETURN+0x100)==data)
	{//use gprs to xfer
		wifi_select=g_share_memory->send_by_wifi;
	}
	else if(addr==TOUCH_SEL_GPRS&& (TOUCH_SEL_GPRS+0x100)==data)
	{//use gprs to xfer
		wifi_select=0;
		write_string(ADDR_XFER_SELECT,"GPRS",strlen("GPRS"));
	}
	else if(addr==TOUCH_SLEEP_SETTING && (TOUCH_SLEEP_SETTING+0x100)==data)
	{
		switch_pic(SLEEPING_SELECT_PAGE);
		g_index=SLEEPING_SELECT_PAGE;
	}
	else if(addr==TOUCH_BLACK_SETTING&& (TOUCH_BLACK_SETTING+0x100)==data)
	{
		g_share_memory->black_lcd=1;
		switch_pic(TIME_PAGE_SETTING);
		g_index=TIME_PAGE_SETTING;
		set_net_interface();
	}	
	else if(addr==TOUCH_PINGBAO_SETTING&& (TOUCH_PINGBAO_SETTING+0x100)==data)
	{
		g_share_memory->black_lcd=0;
		switch_pic(SLEEPING_PINGBAO_SETTING_PAGE);
		g_index=SLEEPING_PINGBAO_SETTING_PAGE;
		set_net_interface();
	}
	else if((addr==TOUCH_ONE_MINS && (TOUCH_ONE_MINS+0x100)==data)
		||(addr==TOUCH_ONE_MINS_1 && (TOUCH_ONE_MINS_1+0x100)==data))
	{
		g_share_memory->sleep=1;
		switch_pic(SYSTEM_SETTING_PAGE);
		g_index=SYSTEM_SETTING_PAGE;
		set_net_interface();
	}
	else if((addr==TOUCH_FIVE_MINS && (TOUCH_FIVE_MINS+0x100)==data)
		||(addr==TOUCH_FIVE_MINS_1 && (TOUCH_FIVE_MINS_1+0x100)==data))
	{
		g_share_memory->sleep=5;
		switch_pic(SYSTEM_SETTING_PAGE);
		g_index=SYSTEM_SETTING_PAGE;
		set_net_interface();
	}
	else if((addr==TOUCH_TEN_MINS && (TOUCH_TEN_MINS+0x100)==data)
		||(addr==TOUCH_TEN_MINS_1 && (TOUCH_TEN_MINS_1+0x100)==data))
	{
		g_share_memory->sleep=10;
		switch_pic(SYSTEM_SETTING_PAGE);
		g_index=SYSTEM_SETTING_PAGE;
		set_net_interface();
	}
	else if(addr==TOUCH_NEVER_MINS_1 && (TOUCH_NEVER_MINS_1+0x100)==data)
	{
		g_share_memory->sleep=0;
		switch_pic(SYSTEM_SETTING_PAGE);
		g_index=SYSTEM_SETTING_PAGE;
		set_net_interface();
	}
	else if(addr==TOUCH_NO_UPLOAD && (TOUCH_NO_UPLOAD+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_UNKNOWN;
		show_cur_select_intr(cur_select_interface);
		if (g_share_memory->sensor_interface_mem[interface_config_no]==
			TYPE_SENSOR_O3_1)
		{
			write_data(ADDR_O3_SHOW_PIC,0x01);
			write_data(ADDR_O3_SHOW_PIC_PPM,0x01);
			g_share_memory->sensor_off[SENSOR_O3] = 1;
			g_share_memory->sensor_has_data[SENSOR_O3] = 0;
		}
		else if (g_share_memory->sensor_interface_mem[interface_config_no]==
			TYPE_SENSOR_TVOC_1)
		{
			write_data(ADDR_TVOC_SHOW_PIC,0x01);
			write_data(ADDR_TVOC_SHOW_PIC_PPM,0x01);
			g_share_memory->sensor_off[SENSOR_TVOC] = 1;
			g_share_memory->sensor_has_data[SENSOR_TVOC] = 0;
		}
		
		if (interface_config_no == 1)
		{
			g_share_memory->sensor_off[SENSOR_PM25] = 1;
			g_share_memory->sensor_off[SENSOR_PM10] = 1;
			g_share_memory->sensor_has_data[SENSOR_PM25] = 0;
			g_share_memory->sensor_has_data[SENSOR_PM10] = 0;
			write_data(ADDR_PM25_SHOW_PIC,0x01);
			write_data(ADDR_PM10_SHOW_PIC,0x01);
			write_data(ADDR_PM25_SHOW_PIC_PPM,0x01);
			write_data(ADDR_PM10_SHOW_PIC_PPM,0x01);
		}
		else if (interface_config_no == 2)
		{
			g_share_memory->sensor_off[SENSOR_CO2] = 1;
			g_share_memory->sensor_has_data[SENSOR_CO2] = 0;
			write_data(ADDR_CO2_SHOW_PIC,0x01);
			write_data(ADDR_CO2_SHOW_PIC_PPM,0x01);
		}
		else if (interface_config_no == 3)
		{
			g_share_memory->sensor_off[SENSOR_HCHO] = 1;
			g_share_memory->sensor_has_data[SENSOR_HCHO] = 0;
			write_data(ADDR_HCHO_SHOW_PIC,0x01);
			write_data(ADDR_HCHO_SHOW_PIC_PPM,0x01);
			printfLog(LCD_PROCESS"close HCHO display\n");
		}
		else if (interface_config_no == 4)
		{
			g_share_memory->sensor_off[SENSOR_CO] = 1;
			g_share_memory->sensor_has_data[SENSOR_CO] = 0;
			write_data(ADDR_CO_SHOW_PIC,0x01);
			write_data(ADDR_CO_SHOW_PIC_PPM,0x01);
		}
		else if (interface_config_no == 5)
		{
			g_share_memory->sensor_off[SENSOR_NOISE] = 1;
			g_share_memory->sensor_has_data[SENSOR_NOISE] = 0;
			write_data(ADDR_NOISE_SHOW_PIC,0x01);
			write_data(ADDR_NOISE_SHOW_PIC_PPM,0x01);
		}
		else if (interface_config_no == 8)
		{
			g_share_memory->sensor_off[SENSOR_TEMP] = 1;
			g_share_memory->sensor_off[SENSOR_SHIDU] = 1;
			g_share_memory->sensor_has_data[SENSOR_TEMP] = 0;
			g_share_memory->sensor_has_data[SENSOR_SHIDU] = 0;
			write_data(ADDR_SHIDU_SHOW_PIC,0x01);
			write_data(ADDR_TEMP_SHOW_PIC,0x01);
			write_data(ADDR_TEMP_SHOW_PIC_PPM,0x01);
			write_data(ADDR_SHIDU_SHOW_PIC_PPM,0x01);
		}
		else if (interface_config_no == 9)
		{
			g_share_memory->sensor_off[SENSOR_WIND] = 1;
			g_share_memory->sensor_has_data[SENSOR_WIND] = 0;
			write_data(ADDR_WIND_SHOW_PIC,0x01);
			write_data(ADDR_WIND_SHOW_PIC_PPM,0x01);
		}
		else if (interface_config_no == 10)
		{
			g_share_memory->sensor_off[SENSOR_PRESS] = 1;
			g_share_memory->sensor_has_data[SENSOR_PRESS] = 0;
			write_data(ADDR_PRESS_SHOW_PIC,0x01);
			write_data(ADDR_PRESS_SHOW_PIC_PPM,0x01);
		}
		printfLog(LCD_PROCESS"no upload key pressed\n");
	}
	else if(addr==TOUCH_XFER_SETTING&&(TOUCH_XFER_SETTING+0x100)==data)
	{//enter wifi passwd setting
		char buf1[]={"WIFI已联网"};
		char buf2[]={"请输入WIFI账号和密码"};
		char cmd[256]={0};
		clear_buf(ADDR_AP_NAME,20);
		clear_buf(ADDR_AP_KEY,20);
		clear_buf(ADDR_WIFI_STATUS,32);
		switch_pic(100);
		if(ping_server())
			sprintf(cmd,"%s",buf1);
		else
			sprintf(cmd,"%s",buf2);
		write_string(ADDR_WIFI_STATUS,cmd,strlen(cmd));		
		switch_pic(XFER_SETTING_PAGE);
		g_index=XFER_SETTING_PAGE;
		wifi_select=g_share_memory->send_by_wifi;
		if(g_share_memory->send_by_wifi)
		{
			write_string(ADDR_XFER_SELECT,"WIFI",strlen("WIFI"));
		}
		else
		{
			write_string(ADDR_XFER_SELECT,"GPRS",strlen("GPRS"));
		}
	}
	else if(addr==TOUCH_TIME_OK && (TOUCH_TIME_OK+0x100)==data)
	{//manul set time
		manul_set_time();
		switch_pic(SYSTEM_SETTING_PAGE);
		g_index=SYSTEM_SETTING_PAGE;
	}
	else if(addr==TOUCH_SYNC_SERVER && (TOUCH_SYNC_SERVER+0x100)==data)
	{//set time from server
		sync_server(0,0);
		display_time(g_share_memory->server_time[5]+2000,g_share_memory->server_time[6],g_share_memory->server_time[7],
			g_share_memory->server_time[8],g_share_memory->server_time[9],g_share_memory->server_time[10]);
	}
	else if(addr==TOUCH_INTERFACE_0 && (TOUCH_INTERFACE_0+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=101;
			g_share_memory->jiaozhun_sensor=atoi(ID_CAP_PM_10);
			jiaozhun(1,verify_object,verify_point);
		}
	}
	else if(addr==TOUCH_INTERFACE_1 && (TOUCH_INTERFACE_1+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=1;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);
		}
		else
		{
			interface_config_no=1;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_2 && (TOUCH_INTERFACE_2+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;		
			verify_object=2;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);
		}
		else
		{
			interface_config_no=2;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_3 && (TOUCH_INTERFACE_3+0x100)==data)
	{
		if(interface_select==0)
		{	
			g_index=VERIFY_PAGE;
			verify_object=3;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=3;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_4 && (TOUCH_INTERFACE_4+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=4;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=4;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_5 && (TOUCH_INTERFACE_5+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=5;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=5;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_6 && (TOUCH_INTERFACE_6+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=6;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=6;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_7 && (TOUCH_INTERFACE_7+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=107;
			g_share_memory->jiaozhun_sensor=atoi(ID_CAP_SHI_DU);
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=7;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_8 && (TOUCH_INTERFACE_8+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=8;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=8;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_9 && (TOUCH_INTERFACE_9+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=9;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));		
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=9;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_10 && (TOUCH_INTERFACE_10+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=10;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));		
			jiaozhun(1,verify_object,verify_point);		
		}
		else
		{
			interface_config_no=10;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_INTERFACE_11 && (TOUCH_INTERFACE_11+0x100)==data)
	{
		if(interface_select==0)
		{
			g_index=VERIFY_PAGE;
			verify_object=11;
			g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
			jiaozhun(1,verify_object,verify_point);
		}
		else
		{
			interface_config_no=11;
			cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
			show_cur_select_intr(cur_select_interface);
			switch_pic(SENSOR_SEL_PAGE);
		}
	}
	else if(addr==TOUCH_VERIFY && (TOUCH_VERIFY+0x100)==data)
	{	//verify sensor display
		g_share_memory->sensor_interface_mem[0]=0x1234;
		switch_pic(100);
		ask_interface();
		show_cur_interface(INTERFACE_PAGE);
		switch_pic(INTERFACE_PAGE);
		g_index=INTERFACE_PAGE;
		interface_select=0;
	}
	else if(addr==TOUCH_EXIT_VERIFY && (TOUCH_EXIT_VERIFY+0x100)==data)
	{	//verify sensor display
		if (g_index!=ALARM_SETTING_PAGE) 
		{
			if(verify_point==0)
				tun_zero(0);
			verify_point=100;
			jiaozhun(0,verify_object,verify_point);
		}
		else
			switch_pic(ALARM_SETTING_PAGE);
	}
	else if(addr==ADDR_XIUZHENG)
	{
		char ch;
		while(read(g_share_memory->fd_lcd,&ch,1)==1);
		send_return(verify_object,verify_point);
		printfLog(LCD_PROCESS"XiuZhen Enter\n");
	}
	else if(addr==TOUCH_TUN_ZERO && (TOUCH_TUN_ZERO+0x100)==data)
	{//verify sensor display
		g_index=TUN_ZERO_PAGE;
		g_share_memory->sensor_interface_mem[0]=0x1234;
		switch_pic(100);
		ask_interface();
		clear_buf(ADDR_TUN_ZERO_HCHO,4);
		clear_buf(ADDR_TUN_ZERO_CO,4);
		switch_pic(TUN_ZERO_PAGE);		
	}
	else if(addr==TOUCH_TUN_ZERO_START && (TOUCH_TUN_ZERO_START+0x100)==data)
	{//tun zero point start
		if(g_share_memory->factory_mode!=TUN_ZERO_MODE)
		{
			g_share_memory->factory_mode=TUN_ZERO_MODE;
			tun_zero(1);
		}
	}
	else if((addr==TOUCH_TUN_ZERO_RETURN && (TOUCH_TUN_ZERO_RETURN+0x100)==data) || 
		(addr==TOUCH_TUN_ZERO_END && (TOUCH_TUN_ZERO_END+0x100)==data))
	{//tun zero point stop
		if(g_share_memory->factory_mode == TUN_ZERO_MODE)
		{
			if(addr==TOUCH_TUN_ZERO_RETURN && (TOUCH_TUN_ZERO_RETURN+0x100)==data)
			tun_zero_return();
			else
			tun_zero(0);
			g_share_memory->factory_mode=NORMAL_MODE;
		}
		switch_pic(SENSOR_SETTING_PAGE);
		g_index=SENSOR_SETTING_PAGE;
	}
	else if(addr==TOUCH_SEL_INTERFACE && (TOUCH_SEL_INTERFACE+0x100)==data)
	{//set sensor interface
		g_share_memory->sensor_interface_mem[0]=0x1234;
		switch_pic(100);
		ask_interface();
		show_cur_interface(INTERFACE_PAGE);
		switch_pic(INTERFACE_PAGE);
		g_index=INTERFACE_PAGE;
		interface_select=1;
	}
	else if(addr==TOUCH_SEL_VP_0 && (TOUCH_SEL_VP_0+0x100)==data)
	{
		if(verify_point!=0)
		{
			verify_point=0;
			tun_zero(1);
			show_point(0,verify_object);
		}
	}
	else if(addr==TOUCH_SEL_VP_1 && (TOUCH_SEL_VP_1+0x100)==data)
	{
		if(verify_point!=1)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=1;
			show_point(1,verify_object);
		}
	}
	else if(addr==TOUCH_SEL_VP_2 && (TOUCH_SEL_VP_2+0x100)==data)
	{
		if(verify_point!=2)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=2;
			show_point(2,verify_object);
		}
	}
	else if(addr==TOUCH_SEL_VP_3 && (TOUCH_SEL_VP_3+0x100)==data)
	{
		if(verify_point!=3)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=3;
			show_point(3,verify_object);
		}
	}
	else if(addr==TOUCH_SEL_VP_4 && (TOUCH_SEL_VP_4+0x100)==data)
	{
		if(verify_point!=4)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=4;
			show_point(4,verify_object);
		}
	}
	else if(addr==TOUCH_SEL_VP_5 && (TOUCH_SEL_VP_5+0x100)==data)
	{
		if(verify_point!=5)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=5;
			show_point(5,verify_object);
		}
	}
	else if(addr==TOUCH_SEL_VP_6 && (TOUCH_SEL_VP_6+0x100)==data)
	{
		if(verify_point!=6)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=6;
			show_point(6,verify_object);
		}
	}
	else if(addr==TOUCH_SEL_VP_7 && (TOUCH_SEL_VP_7+0x100)==data)
	{
		if(verify_point!=7)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=7;
			show_point(7,verify_object);
		}
	}
	else if(addr==TOUCH_INTERFACE_RETURN2 && (TOUCH_INTERFACE_RETURN2+0x100)==data)
	{
		if(interface_config_no!=0)
		{
			if((interface_config_no==1 && cur_select_interface==TYPE_SENSOR_PM25_WEISHEN)
				||(interface_config_no==2 && (cur_select_interface==TYPE_SENSOR_CO2_WEISHEN || cur_select_interface==TYPE_SENSOR_CO2_RUDIAN))
				||(interface_config_no==3 && (cur_select_interface==TYPE_SENSOR_CH2O_AERSHEN|| cur_select_interface==TYPE_SENSOR_CH2O_WEISHEN))
				||(interface_config_no==4 && (cur_select_interface==TYPE_SENSOR_CO_DD|| cur_select_interface==TYPE_SENSOR_CO_WEISHEN))
				||(interface_config_no==5 && cur_select_interface==TYPE_SENSOR_ZHAOSHEN)
				||(interface_config_no==8 && cur_select_interface==TYPE_SENSOR_WENSHI_RUSHI)
				||(interface_config_no==9 && cur_select_interface==TYPE_SENSOR_FENGSU)
				||(interface_config_no==10 && cur_select_interface==TYPE_SENSOR_QIYA_RUSHI)
				||(((interface_config_no>5 && interface_config_no<8) || interface_config_no == 11)
				&& cur_select_interface!=TYPE_SENSOR_QIYA_RUSHI
				&& cur_select_interface!=TYPE_SENSOR_WENSHI_RUSHI
				&& cur_select_interface!=TYPE_SENSOR_PM25_WEISHEN
				&& cur_select_interface!=TYPE_SENSOR_FENGSU
				&& cur_select_interface!=TYPE_SENSOR_CO2_WEISHEN
				&& cur_select_interface!=TYPE_SENSOR_CO2_RUDIAN
				&& cur_select_interface!=TYPE_SENSOR_CH2O_AERSHEN
				&& cur_select_interface!=TYPE_SENSOR_CH2O_WEISHEN
				&& cur_select_interface!=TYPE_SENSOR_CO_DD
				&& cur_select_interface!=TYPE_SENSOR_CO_WEISHEN
				&& cur_select_interface!=TYPE_SENSOR_ZHAOSHEN)
				|| cur_select_interface==TYPE_SENSOR_UNKNOWN)
			g_share_memory->sensor_interface_mem[interface_config_no]=cur_select_interface;
			printfLog(LCD_PROCESS"save sensor_interface_mem[%d]=%02x\n",interface_config_no,cur_select_interface);
			show_cur_interface(INTERFACE_PAGE);
			switch_pic(INTERFACE_PAGE);
			g_index=INTERFACE_PAGE;
			//switch_pic(fd_lcd,SYSTEM_SET_PAGE);
		}
	}
	else if(addr==TOUCH_SENSOR_SETTING && (TOUCH_SENSOR_SETTING+0x100)==data)
	{//show history CO2 the first page
		if(logged)
		{			
			ctl_fan(0);
			switch_pic(SENSOR_SETTING_PAGE);
		}
		else
		{
			clear_buf(ADDR_LOGIN_USER_NAME,16);
			clear_buf(ADDR_LOGIN_USER_KEY,16);
			switch_pic(LOGIN_PAGE);
			last_g_index=g_index;			
		}
		g_index=SENSOR_SETTING_PAGE;
	}	
	else if(addr==TOUCH_INTERFACE_RETURN1 && (TOUCH_INTERFACE_RETURN1+0x100)==data)
	{		
		switch_pic(SENSOR_SETTING_PAGE);
		g_index=SENSOR_SETTING_PAGE;
	}
	else if(addr==TOUCH_INTERFACE_OK && (TOUCH_INTERFACE_OK+0x100)==data)
	{		
		if(interface_select==1)
		set_interface();
		switch_pic(SENSOR_SETTING_PAGE);
		g_index=SENSOR_SETTING_PAGE;
	}
	else if(addr==TOUCH_SEL_HCHO_1 && (TOUCH_SEL_HCHO_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CH2O_WEISHEN;
		g_share_memory->sensor_off[SENSOR_HCHO] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_HCHO_2 && (TOUCH_SEL_HCHO_2+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CH2O_AERSHEN;
		g_share_memory->sensor_off[SENSOR_HCHO] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_WENSHI_1 && (TOUCH_SEL_WENSHI_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_WENSHI_RUSHI;
		g_share_memory->sensor_off[SENSOR_TEMP] = 0;
		g_share_memory->sensor_off[SENSOR_SHIDU] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_PM25_1 && (TOUCH_SEL_PM25_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_PM25_WEISHEN;
		g_share_memory->sensor_off[SENSOR_PM25] = 0;
		g_share_memory->sensor_off[SENSOR_PM10] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_NOISE_1 && (TOUCH_SEL_NOISE_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_ZHAOSHEN;
		g_share_memory->sensor_off[SENSOR_NOISE] = 0;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SEL_CO_1 && (TOUCH_SEL_CO_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO_WEISHEN;
		g_share_memory->sensor_off[SENSOR_CO] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_CO_2 && (TOUCH_SEL_CO_2+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO_DD;
		g_share_memory->sensor_off[SENSOR_CO] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_CO2_1 && (TOUCH_SEL_CO2_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO2_WEISHEN;
		g_share_memory->sensor_off[SENSOR_CO2] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_CO2_2 && (TOUCH_SEL_CO2_2+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO2_RUDIAN;
		g_share_memory->sensor_off[SENSOR_CO2] = 0;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SEL_WIND_1 && (TOUCH_SEL_WIND_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_FENGSU;
		g_share_memory->sensor_off[SENSOR_WIND] = 0;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SEL_PRESS_1 && (TOUCH_SEL_PRESS_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_QIYA_RUSHI;
		g_share_memory->sensor_off[SENSOR_PRESS] = 0;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SEL_O3_1 && (TOUCH_SEL_O3_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_O3_1;
		g_share_memory->sensor_off[SENSOR_O3] = 0;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SEL_TVOC_1&& (TOUCH_SEL_TVOC_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_TVOC_1;
		g_share_memory->sensor_off[SENSOR_TVOC] = 0;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SYSTEM_SETTING_RETURN&& (TOUCH_SYSTEM_SETTING_RETURN+0x100)==data)
	{
		g_index=MAIN_PAGE;
	}
	else if((addr==TOUCH_PRODUCT_INFO_1 && (TOUCH_PRODUCT_INFO_1+0x100)==data)||
		(addr==TOUCH_PRODUCT_INFO_2 && (TOUCH_PRODUCT_INFO_2+0x100)==data))
	{//product info
		clear_buf(ADDR_HW_VER,40);
		clear_buf(ADDR_PRODUCT_MODEL,40);
		clear_buf(ADDR_PRODUCT_ID,40);
		clear_buf(ADDR_SW_VERSION,20);
		write_string(ADDR_PRODUCT_ID,g_share_memory->uuid,strlen(g_share_memory->uuid));
		write_string(ADDR_HW_VER,g_share_memory->hw_ver,strlen(g_share_memory->hw_ver));
		write_string(ADDR_SW_VERSION,VERSION,strlen(VERSION));
		switch_pic(PRODUCT_PAGE);
		g_index=PRODUCT_PAGE;
	}		
	else if((addr==TOUCH_HCHO_HISTORY&& (TOUCH_HCHO_HISTORY+0x100)==data)||
			(addr==TOUCH_CO_HISTORY&& (TOUCH_CO_HISTORY+0x100)==data)||
			(addr==TOUCH_CO2_HISTORY&& (TOUCH_CO2_HISTORY+0x100)==data)||
			(addr==TOUCH_PRESS_HISTORY&& (TOUCH_PRESS_HISTORY+0x100)==data)||
			(addr==TOUCH_PM25_HISTORY&& (TOUCH_PM25_HISTORY+0x100)==data)||
			(addr==TOUCH_PM10_HISTORY&& (TOUCH_PM10_HISTORY+0x100)==data)||
			(addr==TOUCH_WIND_HISTORY&& (TOUCH_WIND_HISTORY+0x100)==data)||
			(addr==TOUCH_TVOC_HISTORY&& (TOUCH_TVOC_HISTORY+0x100)==data)||
			(addr==TOUCH_TEMP_HISTORY&& (TOUCH_TEMP_HISTORY+0x100)==data)||
			(addr==TOUCH_SHIDU_HISTORY&& (TOUCH_SHIDU_HISTORY+0x100)==data)||
			(addr==TOUCH_NOISE_HISTORY&& (TOUCH_NOISE_HISTORY+0x100)==data)||
			(addr==TOUCH_O3_HISTORY&& (TOUCH_O3_HISTORY+0x100)==data))
	{//show detail in list
		switch (g_index)
		{
			case CURVE_PAGE_CO:
			{//co
				switch_pic(LIST_PAGE_CO);
				begin_co=0;
				memset(g_history_log[SENSOR_CO],0,1024);
				g_history_index[SENSOR_CO]=0;
				g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=0;
				(g_history_index[SENSOR_CO])++;
				begin_co=show_history(ID_CAP_CO,begin_co,g_history_index[SENSOR_CO]-1);
				g_history_log[SENSOR_CO][g_history_index[SENSOR_CO]]=begin_co;		
			}
			break;
			case CURVE_PAGE_PRESS:
			{//press
				switch_pic(LIST_PAGE_PRESS);
				begin_press=0;
				memset(g_history_log[SENSOR_PRESS],0,1024);
				g_history_index[SENSOR_PRESS]=0;
				g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=0;
				(g_history_index[SENSOR_PRESS])++;
				begin_press=show_history(ID_CAP_QI_YA,begin_press,g_history_index[SENSOR_PRESS]-1);
				g_history_log[SENSOR_PRESS][g_history_index[SENSOR_PRESS]]=begin_press;		
			}
			break;
			case CURVE_PAGE_PM10:
			{//pm10
				switch_pic(LIST_PAGE_PM10);
				begin_pm10=0;
				memset(g_history_log[SENSOR_PM10],0,1024);
				g_history_index[SENSOR_PM10]=0;
				g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=0;
				(g_history_index[SENSOR_PM10])++;
				begin_pm10=show_history(ID_CAP_PM_10,begin_pm10,g_history_index[SENSOR_PM10]-1);
				g_history_log[SENSOR_PM10][g_history_index[SENSOR_PM10]]=begin_pm10;		
			}
			break;
			case CURVE_PAGE_WIND:
			{//wind
				switch_pic(LIST_PAGE_WIND);
				begin_wind=0;
				memset(g_history_log[SENSOR_WIND],0,1024);
				g_history_index[SENSOR_WIND]=0;
				g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=0;
				(g_history_index[SENSOR_WIND])++;
				begin_wind=show_history(ID_CAP_FENG_SU,begin_wind,g_history_index[SENSOR_WIND]-1);
				g_history_log[SENSOR_WIND][g_history_index[SENSOR_WIND]]=begin_wind;		
			}
			break;
			case CURVE_PAGE_TVOC:
			{//tvoc
				switch_pic(LIST_PAGE_TVOC);
				begin_tvoc=0;
				memset(g_history_log[SENSOR_TVOC],0,1024);
				g_history_index[SENSOR_TVOC]=0;
				g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=0;
				(g_history_index[SENSOR_TVOC])++;
				begin_tvoc=show_history(ID_CAP_TVOC,begin_tvoc,g_history_index[SENSOR_TVOC]-1);
				g_history_log[SENSOR_TVOC][g_history_index[SENSOR_TVOC]]=begin_tvoc;		
			}
			break;
			case CURVE_PAGE_O3:
			{//o3
				switch_pic(LIST_PAGE_O3);
				begin_o3=0;
				memset(g_history_log[SENSOR_O3],0,1024);
				g_history_index[SENSOR_O3]=0;
				g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=0;
				(g_history_index[SENSOR_O3])++;
				begin_o3=show_history(ID_CAP_CHOU_YANG,begin_o3,g_history_index[SENSOR_O3]-1);
				g_history_log[SENSOR_O3][g_history_index[SENSOR_O3]]=begin_o3;		
			}
			break;
			case CURVE_PAGE_NOISE:
			{//noise
				switch_pic(LIST_PAGE_NOISE);
				begin_noise=0;
				memset(g_history_log[SENSOR_NOISE],0,1024);
				g_history_index[SENSOR_NOISE]=0;
				g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=0;
				(g_history_index[SENSOR_NOISE])++;
				begin_noise=show_history(ID_CAP_BUZZY,begin_noise,g_history_index[SENSOR_NOISE]-1);
				g_history_log[SENSOR_NOISE][g_history_index[SENSOR_NOISE]]=begin_noise;		
			}
			break;
			case CURVE_PAGE_CO2:
			{//co2
				switch_pic(LIST_PAGE_CO2);
				begin_co2=0;
				memset(g_history_log[SENSOR_CO2],0,1024);
				g_history_index[SENSOR_CO2]=0;
				g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=0;
				(g_history_index[SENSOR_CO2])++;
				begin_co2=show_history(ID_CAP_CO2,begin_co2,g_history_index[SENSOR_CO2]-1);
				g_history_log[SENSOR_CO2][g_history_index[SENSOR_CO2]]=begin_co2;		
			}
			break;
			case CURVE_PAGE_HCHO:
			{//hcho
				switch_pic(LIST_PAGE_HCHO);
				begin_hcho=0;
				memset(g_history_log[SENSOR_HCHO],0,1024);
				g_history_index[SENSOR_HCHO]=0;
				g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=0;
				(g_history_index[SENSOR_HCHO])++;
				begin_hcho=show_history(ID_CAP_HCHO,begin_hcho,g_history_index[SENSOR_HCHO]-1);
				g_history_log[SENSOR_HCHO][g_history_index[SENSOR_HCHO]]=begin_hcho;		
			}
			break;
			case CURVE_PAGE_SHIDU:
			{//shidu
				switch_pic(LIST_PAGE_SHIDU);				
				begin_shidu=0;
				memset(g_history_log[SENSOR_SHIDU],0,1024);
				g_history_index[SENSOR_SHIDU]=0;
				g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=0;
				(g_history_index[SENSOR_SHIDU])++;
				begin_shidu=show_history(ID_CAP_SHI_DU,begin_shidu,g_history_index[SENSOR_SHIDU]-1);
				g_history_log[SENSOR_SHIDU][g_history_index[SENSOR_SHIDU]]=begin_shidu;				
			}
			break;
			case CURVE_PAGE_TEMP:
			{//temp
				switch_pic(LIST_PAGE_TEMP);
				begin_temp=0;
				memset(g_history_log[SENSOR_TEMP],0,1024);
				g_history_index[SENSOR_TEMP]=0;
				g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=0;
				(g_history_index[SENSOR_TEMP])++;
				begin_temp=show_history(ID_CAP_TEMPERATURE,begin_temp,g_history_index[SENSOR_TEMP]-1);
				g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]]=begin_temp;
				printfLog(LCD_PROCESS"begin_temp in curve is %d,page %d\n",g_history_log[SENSOR_TEMP][g_history_index[SENSOR_TEMP]],
					g_history_index[SENSOR_TEMP]);
			}
			break;
			case CURVE_PAGE_PM25:
			{//pm25
				switch_pic(LIST_PAGE_PM25);
				begin_pm25=0;
				memset(g_history_log[SENSOR_PM25],0,1024);
				g_history_index[SENSOR_PM25]=0;
				g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=0;
				(g_history_index[SENSOR_PM25])++;
				begin_pm25=show_history(ID_CAP_PM_25,begin_pm25,g_history_index[SENSOR_PM25]-1);
				g_history_log[SENSOR_PM25][g_history_index[SENSOR_PM25]]=begin_pm25;
			}
			break;
			default:
				break;
		}
	}
	else if(addr==TOUCH_LOGIN_RETURN&& (TOUCH_LOGIN_RETURN+0x100)==data)
	{
			switch_pic(last_g_index);
			g_index=last_g_index;
	}
	else if(addr==TOUCH_LOGIN_OK&& (TOUCH_LOGIN_OK+0x100)==data)
	{//Login if didn't 
		log_in();
		printfLog(LCD_PROCESS"lcd=>history_done %d\n",g_share_memory->history_done);
		if(logged)
		{
			if(g_share_memory->history_done==0)
			switch (g_index)
			{
			case LIST_PAGE_CO:
			{//co
				switch_pic(LIST_PAGE_CO);
				show_history(ID_CAP_CO,begin_co,0);
				//begin_co=0;
			}
			break;
			case LIST_PAGE_NOISE:
			{//co
				switch_pic(LIST_PAGE_NOISE);
				show_history(ID_CAP_BUZZY,begin_noise,0);
				//begin_co=0;
			}
			break;
			case LIST_PAGE_WIND:
			{//co
				switch_pic(LIST_PAGE_WIND);
				show_history(ID_CAP_FENG_SU,begin_wind,0);
				//begin_co=0;
			}
			break;
			case LIST_PAGE_O3:
			{//co
				switch_pic(LIST_PAGE_O3);
				show_history(ID_CAP_CHOU_YANG,begin_o3,0);
				//begin_co=0;
			}
			break;
			case LIST_PAGE_PM10:
			{//co
				switch_pic(LIST_PAGE_PM10);
				show_history(ID_CAP_PM_10,begin_pm10,0);
				//begin_co=0;
			}
			break;
			case LIST_PAGE_TVOC:
			{//co
				switch_pic(LIST_PAGE_TVOC);
				show_history(ID_CAP_TVOC,begin_tvoc,0);
				//begin_co=0;
			}
			break;
			case LIST_PAGE_PRESS:
			{//co
				switch_pic(LIST_PAGE_PRESS);
				show_history(ID_CAP_QI_YA,begin_press,0);
				//begin_co=0;
			}
			break;
			case LIST_PAGE_CO2:
			{//co2
				switch_pic(LIST_PAGE_CO2);
				show_history(ID_CAP_CO2,begin_co2,0);
				//begin_co2=0;
			}
			break;
			case LIST_PAGE_HCHO:
			{//hcho
				switch_pic(LIST_PAGE_HCHO);
				show_history(ID_CAP_HCHO,begin_hcho,0);
				//begin_hcho=0;
			}
			break;
			case LIST_PAGE_SHIDU:
			{//shidu
				switch_pic(LIST_PAGE_SHIDU);
				show_history(ID_CAP_SHI_DU,begin_shidu,0);
				//begin_shidu=0;
			}
			break;
			case LIST_PAGE_TEMP:
			{//temp
				switch_pic(LIST_PAGE_TEMP);
				show_history(ID_CAP_TEMPERATURE,begin_temp,0);
				//begin_temp=0;
			}
			break;
			case LIST_PAGE_PM25:
			{//pm25
				switch_pic(LIST_PAGE_PM25);
				show_history(ID_CAP_PM_25,begin_pm25,0);
				//begin_pm25=0;
			}
			break;
			default:
				break;
		}
			else
			switch (g_index)
			{
				case CURVE_PAGE_CO:
				{//co
					switch_pic(CURVE_PAGE_CO);
					show_curve(ID_CAP_CO,&curve_co);
				}
				break;
				case CURVE_PAGE_NOISE:
				{//co
					switch_pic(CURVE_PAGE_NOISE);
					show_curve(ID_CAP_BUZZY,&curve_noise);
				}
				break;
				case CURVE_PAGE_WIND:
				{//co
					switch_pic(CURVE_PAGE_WIND);
					show_curve(ID_CAP_FENG_SU,&curve_wind);
				}
				break;
				case CURVE_PAGE_PM10:
				{//co
					switch_pic(CURVE_PAGE_PM10);
					show_curve(ID_CAP_PM_10,&curve_pm10);
				}
				break;
				case CURVE_PAGE_TVOC:
				{//co
					switch_pic(CURVE_PAGE_TVOC);
					show_curve(ID_CAP_TVOC,&curve_tvoc);
				}
				break;
				case CURVE_PAGE_PRESS:
				{//co
					switch_pic(CURVE_PAGE_PRESS);
					show_curve(ID_CAP_QI_YA,&curve_press);
				}
				break;
				case CURVE_PAGE_O3:
				{//co
					switch_pic(CURVE_PAGE_O3);
					show_curve(ID_CAP_CHOU_YANG,&curve_o3);
				}
				break;
				case CURVE_PAGE_CO2:
				{//co2
					switch_pic(CURVE_PAGE_CO2);
					show_curve(ID_CAP_CO2,&curve_co2);
				}
				break;
				case CURVE_PAGE_HCHO:
				{//hcho
					switch_pic(CURVE_PAGE_HCHO);
					show_curve(ID_CAP_HCHO,&curve_hcho);
				}
				break;
				case CURVE_PAGE_SHIDU:
				{//shidu
					switch_pic(CURVE_PAGE_SHIDU);
					show_curve(ID_CAP_SHI_DU,&curve_shidu);
				}
				break;
				case CURVE_PAGE_TEMP:
				{//temp
					switch_pic(CURVE_PAGE_TEMP);
					show_curve(ID_CAP_TEMPERATURE,&curve_temp);
				}
				break;
				case CURVE_PAGE_PM25:
				{//pm25
					switch_pic(CURVE_PAGE_PM25);
					show_curve(ID_CAP_PM_25,&curve_pm25);
				}
				break;
				default:
					break;
			}
			if(g_index==SENSOR_SETTING_PAGE)
				switch_pic(g_index);
		}
	}
	else if((addr==TOUCH_FAN_1&& (TOUCH_FAN_1+0x100)==data) ||
		(addr==TOUCH_FAN_2&& (TOUCH_FAN_2+0x100)==data))
	{
		if(g_share_memory->fan_state==0)
			g_share_memory->fan_state=1;
		else
			g_share_memory->fan_state=0;
		show_fan(g_share_memory->fan_state);
		ctl_fan(g_share_memory->fan_state);
	}
	else if((addr==TOUCH_AUDIO_1&& (TOUCH_AUDIO_1+0x100)==data)||
		(addr==TOUCH_AUDIO_2&& (TOUCH_AUDIO_2+0x100)==data))
	{
		if(g_share_memory->audio_state==0)
			g_share_memory->audio_state=1;
		else
			g_share_memory->audio_state=0;
		show_audio(g_share_memory->audio_state);
		//ctl_audio(g_share_memory->audio_state);
	}
	else if((addr==TOUCH_MANUL_UPLOADING&& (TOUCH_MANUL_UPLOADING+0x100)==data)||
		(addr==TOUCH_DONE_UPLOADING_WRONG&& (TOUCH_DONE_UPLOADING_WRONG+0x100)==data)||
		(addr==TOUCH_RETURN_UPLOADING_WRONG&& (TOUCH_RETURN_UPLOADING_WRONG+0x100)==data))
	{
		//manul uploading data		
		g_index=UPLOADING_SETTING_PAGE;
		/*
		clear_buf(ADDR_BEGIN_YEAR,4);clear_buf(ADDR_BEGIN_MON,2);clear_buf(ADDR_BEGIN_DAY,2);
		clear_buf(ADDR_BEGIN_HOUR,2);clear_buf(ADDR_BEGIN_MIN,2);		
		clear_buf(ADDR_END_YEAR,4);clear_buf(ADDR_END_MON,2);clear_buf(ADDR_END_DAY,2);
		clear_buf(ADDR_END_HOUR,2);clear_buf(ADDR_END_MIN,2);
		*/
		switch_pic(g_index);
	}	
	else if((addr==TOUCH_RETURN_UPLOADING&& (TOUCH_RETURN_UPLOADING+0x100)==data)||
		(addr==TOUCH_DONE_UPLOADING&& (TOUCH_DONE_UPLOADING+0x100)==data))
	{
		//manul uploading data
		g_index=SYSTEM_SETTING_PAGE;
		switch_pic(g_index);
	}
	else if((addr==TOUCH_BEGIN_UPLOADING&& (TOUCH_BEGIN_UPLOADING+0x100)==data))
	{
		//manul uploading data
		g_index=UPLOADING_WRONG_PAGE;
		char b_year[5]	={0};
		char b_mon[3]	={0};
		char b_day[3]	={0};
		char b_hour[3]	={0};
		char b_min[3]	={0};
		char e_year[5]	={0};
		char e_mon[3]	={0};
		char e_day[3]	={0};
		char e_hour[3]	={0};
		char e_min[3]	={0};
		date b,e,c;
		if(	read_dgus(ADDR_BEGIN_YEAR,2,b_year) && read_dgus(ADDR_BEGIN_DAY,1,b_day)
			&& read_dgus(ADDR_BEGIN_MON,1,b_mon)&& read_dgus(ADDR_BEGIN_HOUR,1,b_hour)
			&& read_dgus(ADDR_BEGIN_MIN,1,b_min)&& read_dgus(ADDR_END_YEAR,2,e_year)
			&& read_dgus(ADDR_END_DAY,1,e_day) 	&& read_dgus(ADDR_END_MON,1,e_mon)
			&& read_dgus(ADDR_END_HOUR,1,e_hour)&& read_dgus(ADDR_END_MIN,1,e_min))
			{
				if( atoi(b_year)>0 		&& atoi(b_mon)>0 	&& atoi(b_mon)<=12 && atoi(b_day)>0
					&& atoi(b_day)<=31	&& atoi(b_hour)>=0 	&& atoi(b_hour)<=23&& atoi(b_min)>=0 
					&& atoi(b_min)<=59	&& atoi(e_year)>0 	&& atoi(e_mon)>0   && atoi(e_mon)<=12 
					&& atoi(e_day)>0 	&& atoi(e_day)<=31 	&& atoi(e_hour)>=0 && atoi(e_hour)<=23 
					&& atoi(e_min)>=0 	&& atoi(e_min)<=59)
					{
						b.wYear=atoi(b_year);b.wMonth=	atoi(b_mon);b.wDay=atoi(b_day);
						b.wHour=atoi(b_hour);b.wMin=	atoi(b_min);
						e.wYear=atoi(e_year);e.wMonth=	atoi(e_mon);e.wDay=atoi(e_day);
						e.wHour=atoi(e_hour);e.wMin=	atoi(e_min);
						c.wYear	=g_share_memory->current_time[0]+2000;
						c.wMonth=g_share_memory->current_time[1];
						c.wDay	=g_share_memory->current_time[2];
						c.wHour	=g_share_memory->current_time[3];
						c.wMin	=g_share_memory->current_time[4];
						if(dataD(b,e)<=92*24*60 && dataD(b,e)>0 && dataD(b,c)>0 && dataD(e,c)>0)
						{							
							g_index=UPLOADING_OK_PAGE;
							switch_pic(100);
							manul_reloading(b_year,b_mon,b_day,b_hour,b_min,
								e_year,e_mon,e_day,e_hour,e_min);
						}
						else
						{
							printfLog(LCD_PROCESS"b-e,b-c,e-c error\n");
						}
					}
				else
					{
						printfLog(LCD_PROCESS"some param error\n");
					}
			}
		else
			{
				printfLog(LCD_PROCESS"some input error\n");
			}
		printfLog(LCD_PROCESS"reloading \nb_time %s-%s-%s %s:%s\ne_time %s-%s-%s %s:%s\n"
							 "c_time %d-%d-%d %d:%d\n",
								b_year,b_mon,b_day,b_hour,b_min,
								e_year,e_mon,e_day,e_hour,e_min,
								c.wYear,c.wMonth,c.wDay,
								c.wHour,c.wMin);
		switch_pic(g_index);		
	}	
	return 0;
}
int runnian(int mf)
{
    if (mf % 4 != 0 && (mf % 100 == 0 || mf % 400 != 0))
        return 0;
    else
        return 1;
}

long dataD(date mf, date time)
{
    int sum = 0;
	int i,flag;
    int yaerNum = time.wYear - mf.wYear;
    for (i = mf.wYear + 1; i<time.wYear; i++)
    {
        if (runnian(i))
            sum++;
    }
    sum += yaerNum * 365;
    printfLog(LCD_PROCESS"b %d-%d-%d %d:%d\n",mf.wYear,mf.wMonth,
    		mf.wDay,mf.wHour,mf.wMin);
    printfLog(LCD_PROCESS"e %d-%d-%d %d:%d\n",time.wYear,time.wMonth,
    		time.wDay,time.wHour,time.wMin);
	printfLog(LCD_PROCESS"dist first to end %d days1\n",sum);
	if(time.wMonth != mf.wMonth)
	{
	    flag = (time.wMonth - mf.wMonth)>0;
	    if (flag)
	    for (i = mf.wMonth; i < time.wMonth; i++)
	    {
	        switch (i)
	        {
	        case 1:case 3:case 5:case 7:case 8:case 10:case 12:sum += 31; break;
	        case 2:if (runnian(mf.wYear)) sum++; sum += 28; break;
	        default:sum += 30;
	        }
	    }
	    else
	    for (i = time.wMonth; i < mf.wMonth; i++)
	    {
	        switch (i)
	        {
	        case 1:case 3:case 5:case 7:case 8:case 10:case 12:sum -= 31; break;
	        case 2:if (runnian(time.wYear)) sum--; sum -= 28; break;
	        default:sum -= 30;
	        }
	    }
	}
    sum += (time.wDay - mf.wDay);
	printfLog(LCD_PROCESS"dist first to end %d days\n",sum);
	sum = sum*24*60;
	if(time.wHour != mf.wHour)
	{
		flag = (time.wHour - mf.wHour)>0;
		if(flag)
			for (i = mf.wHour; i< time.wHour; i++)
				sum += 60;
		else
			for (i = time.wHour; i< mf.wHour; i++)
				sum -= 60;
	}
	sum += (time.wMin - mf.wMin);
	printfLog(LCD_PROCESS"dist first to end %d mins\n",sum);
    return sum;
}

void lcd_loop()
{	
	char ch;
	int i=1;
	int get=0;
	int len = 0;
	char ptr[32]={0};	
	switch_pic(MAIN_PAGE);
	g_index=MAIN_PAGE;
	while(1)	
	{	
		if(read(g_share_memory->fd_lcd,&ch,1)==1)
		{
			//printfLog(LCD_PROCESS"$<= %x \r\n",ch);
			switch(get)
			{
				case 0:
					if(ch==0x5a)
					{
						//printf(LCD_PROCESS"0x5a get ,get =1\r\n");
						get=1;
					}
					else
						get=0;
					break;
				case 1:
					if(ch==0xa5)
					{
						//printf(LCD_PROCESS"0xa5 get ,get =2\r\n");
						get=2;

					}
					else
						get=0;
					break;
				case 2:
					//if(ch==0x06 || ch == 0x0a)
					//{
						//printf(LCD_PROCESS"0x06 get,get =3\r\n");
					//	get=3;
					//}
					//else
					//	get=0;
					len = ch;
					get=3;
					break;
				case 3:
					if(ch==0x83)
					{
						//printf(LCD_PROCESS"0x83 get,get =4\r\n");
						get=4;
						i=1;
						break;
					}
					else
						get=0;
				case 4:
					{
						//printf(LCD_PROCESS"%02x get ,get =5\r\n",ch);
						ptr[i++]=ch;
						if(i==len)
						{
							get=0;
							ptr[0]=0x01;
							printfLog(LCD_PROCESS"get %x %x %x %x %x %x\r\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);
							input_handle(ptr);
							printfLog(LCD_PROCESS"enter new loop\n");
							get=0;
						}
					}
					break;	
				default:
					printfLog(LCD_PROCESS"unknown state\r\n");
					get=0;
					break;						
			}			
		}
	}	
}

int lcd_init()
{
	int fpid = 0, i=0;
	fpid=fork();
	if(fpid==0)
	{
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
		g_share_memory	= (struct share_memory *)shmat(shmid_share_memory,	 0, 0);
		g_share_memory->sensor_interface_mem[0] = 0x1234;
		signal(SIGALRM, lcd_off);		
		init_alarm_show();
		if(g_share_memory->sleep!=0)
			alarm(g_share_memory->sleep*60);
		else
			alarm(5*60);
		for(i=0;i<SENSOR_NO;i++)
			memset(g_history_log[i],0,1024);
		while(1)
			lcd_loop();
	}
	else
		printfLog(LCD_PROCESS"[PID]%d lcd process\n",fpid);
	return 0;
}
