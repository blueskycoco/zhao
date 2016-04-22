#include "cap.h"
#include "dwin.h"
#include "misc.h"
#include "netlib.h"
#include "xfer.h"
int lcd_state=1;
char logged=0,g_index=0;
extern char g_uuid[256];
extern char ip[20];

#define LCD_PROCESS	"[LCD_PROCESS] "
void write_data(unsigned int Index,int data)
{
	//int i = 0;
	char cmd[]={0x5a,0xa5,0x05,0x82,0x00,0x00,0x00,0x00};
	cmd[4]=(Index&0xff00)>>8;cmd[5]=Index&0x00ff;
	cmd[6]=(data&0xff00)>>8;cmd[7]=data&0x00ff;
	//for(i = 0;i<sizeof(cmd);i++)
	//	printfLog(LCD_PROCESS"%02x ",cmd[i]);
	//printf(LCD_PROCESS"\n");
	write(g_share_memory->fd_lcd,cmd,8);
}

void switch_pic(unsigned char Index)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x04,0x00};
	cmd[5]=Index;
	write(g_share_memory->fd_lcd,cmd,6);
}
void lcd_off(int a)
{
	char cmd[]={0x5a,0xa5,0x03,0x80,0x01,0x00};
	write(g_share_memory->fd_lcd,cmd,6);
	switch_pic(OFF_PAGE);
	lcd_state=0;
	printfLog(LCD_PROCESS"lcd off\n");
}
void lcd_on(int page)
{
	switch_pic(page);
	usleep(100000);
	char cmd[]={0x5a,0xa5,0x03,0x80,0x01,0x40};
	write(g_share_memory->fd_lcd,cmd,6);
	lcd_state=1;
	printfLog(LCD_PROCESS"lcd on\n");
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
	clear_buf(ADDR_VERIFY_VALUE,5);
	clear_buf(ADDR_REAL_VALUE,5);
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
	for(i=0;i<len;i++)
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
	printfLog(LCD_PROCESS"sensor co %d, co2 %d, hcho %d, shidu %d, temp %d, pm25 %d\nnetwork_state %d\n",
		g_share_memory->sensor_state[SENSOR_CO],g_share_memory->sensor_state[SENSOR_CO2],g_share_memory->sensor_state[SENSOR_HCHO],
		g_share_memory->sensor_state[SENSOR_SHIDU],g_share_memory->sensor_state[SENSOR_TEMP],g_share_memory->sensor_state[SENSOR_PM25],
		g_share_memory->network_state);
	
	if((g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25])
		&&(!g_share_memory->network_state))
		pic=STATE_E_E_PAGE;
	else if((g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25])
		&&(g_share_memory->network_state))
		pic=STATE_E_O_PAGE;
	else if(!(g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25])
		&&(g_share_memory->network_state))
		pic=STATE_O_O_PAGE;
	else if(!(g_share_memory->sensor_state[SENSOR_CO] || g_share_memory->sensor_state[SENSOR_CO2] || g_share_memory->sensor_state[SENSOR_HCHO] 
		|| g_share_memory->sensor_state[SENSOR_SHIDU] || g_share_memory->sensor_state[SENSOR_TEMP] || g_share_memory->sensor_state[SENSOR_PM25])
		&&(!g_share_memory->network_state))
		pic=STATE_O_E_PAGE;
	switch_pic(pic);
}

void show_history(char *id,int offset)
{	
	char tmp[4]={0};
	if(strncmp(id,ID_CAP_CO,strlen(id))==0)
	{
//		printfLog(LCD_PROCESS"g_co_cnt %d\n",*g_co_cnt);
		if((g_share_memory->cnt[SENSOR_CO]-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7 + 1);
			write_string(ADDR_CO_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",g_share_memory->cnt[SENSOR_CO]/7);
			write_string(ADDR_CO_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(ADDR_CO_LIST_TIME_0,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].time));
			write_string(ADDR_CO_LIST_DATA_0,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].data,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-1].data));
			write_string(ADDR_CO_LIST_TIME_1,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].time,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].time));
			write_string(ADDR_CO_LIST_DATA_1,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].data,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-2].data));
			write_string(ADDR_CO_LIST_TIME_2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].time,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].time));
			write_string(ADDR_CO_LIST_DATA_2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].data,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-3].data));
			write_string(ADDR_CO_LIST_TIME_3,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].time,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].time));
			write_string(ADDR_CO_LIST_DATA_3,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].data,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-4].data));
			write_string(ADDR_CO_LIST_TIME_4,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].time,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].time));
			write_string(ADDR_CO_LIST_DATA_4,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].data,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-5].data));
			write_string(ADDR_CO_LIST_TIME_5,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].time,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].time));
			write_string(ADDR_CO_LIST_DATA_5,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].data,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-6].data));
			write_string(ADDR_CO_LIST_TIME_6,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].time,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].time));
			write_string(ADDR_CO_LIST_DATA_6,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].data,
				strlen(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
//		printfLog(LCD_PROCESS"g_co2_cnt %d\n",g_share_memory->cnt[SENSOR_CO2]);
		if((g_share_memory->cnt[SENSOR_CO2]-offset-7)>0)
		{			
			sprintf(tmp,"%4d",offset/7 + 1);
			write_string(ADDR_CO2_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",g_share_memory->cnt[SENSOR_CO2]/7);			
			write_string(ADDR_CO2_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(ADDR_CO2_LIST_TIME_0,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].time));
			write_string(ADDR_CO2_LIST_DATA_0,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].data,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-1].data));
			write_string(ADDR_CO2_LIST_TIME_1,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].time,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].time));
			write_string(ADDR_CO2_LIST_DATA_1,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].data,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-2].data));
			write_string(ADDR_CO2_LIST_TIME_2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].time,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].time));
			write_string(ADDR_CO2_LIST_DATA_2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].data,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-3].data));
			write_string(ADDR_CO2_LIST_TIME_3,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].time,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].time));
			write_string(ADDR_CO2_LIST_DATA_3,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].data,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-4].data));
			write_string(ADDR_CO2_LIST_TIME_4,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].time,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].time));
			write_string(ADDR_CO2_LIST_DATA_4,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].data,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-5].data));
			write_string(ADDR_CO2_LIST_TIME_5,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].time,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].time));
			write_string(ADDR_CO2_LIST_DATA_5,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].data,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-6].data));
			write_string(ADDR_CO2_LIST_TIME_6,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].time,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].time));
			write_string(ADDR_CO2_LIST_DATA_6,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].data,
				strlen(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
//		printfLog(LCD_PROCESS"g_co_cnt %d\n",*g_hcho_cnt);
		if((g_share_memory->cnt[SENSOR_HCHO]-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7 + 1);
			write_string(ADDR_HCHO_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",g_share_memory->cnt[SENSOR_HCHO]/7);			
			write_string(ADDR_HCHO_PAGE_TOTAL,tmp,strlen(tmp));		
			write_string(ADDR_HCHO_LIST_TIME_0,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].time));
			write_string(ADDR_HCHO_LIST_DATA_0,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].data,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-1].data));
			write_string(ADDR_HCHO_LIST_TIME_1,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].time,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].time));
			write_string(ADDR_HCHO_LIST_DATA_1,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].data,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-2].data));
			write_string(ADDR_HCHO_LIST_TIME_2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].time,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].time));
			write_string(ADDR_HCHO_LIST_DATA_2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].data,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-3].data));
			write_string(ADDR_HCHO_LIST_TIME_3,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].time,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].time));
			write_string(ADDR_HCHO_LIST_DATA_3,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].data,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-4].data));
			write_string(ADDR_HCHO_LIST_TIME_4,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].time,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].time));
			write_string(ADDR_HCHO_LIST_DATA_4,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].data,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-5].data));
			write_string(ADDR_HCHO_LIST_TIME_5,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].time,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].time));
			write_string(ADDR_HCHO_LIST_DATA_5,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].data,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-6].data));
			write_string(ADDR_HCHO_LIST_TIME_6,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].time,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].time));
			write_string(ADDR_HCHO_LIST_DATA_6,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].data,
				strlen(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
//		printfLog(LCD_PROCESS"g_shidu_cnt %d\n",*g_shidu_cnt);
		if((g_share_memory->cnt[SENSOR_SHIDU]-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7 + 1);
			write_string(ADDR_SHIDU_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",g_share_memory->cnt[SENSOR_SHIDU]/7);			
			write_string(ADDR_SHIDU_PAGE_TOTAL,tmp,strlen(tmp));		
			write_string(ADDR_SHIDU_LIST_TIME_0,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].time));
			write_string(ADDR_SHIDU_LIST_DATA_0,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].data,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-1].data));
			write_string(ADDR_SHIDU_LIST_TIME_1,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].time,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].time));
			write_string(ADDR_SHIDU_LIST_DATA_1,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].data,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-2].data));
			write_string(ADDR_SHIDU_LIST_TIME_2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].time,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].time));
			write_string(ADDR_SHIDU_LIST_DATA_2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].data,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-3].data));
			write_string(ADDR_SHIDU_LIST_TIME_3,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].time,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].time));
			write_string(ADDR_SHIDU_LIST_DATA_3,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].data,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-4].data));
			write_string(ADDR_SHIDU_LIST_TIME_4,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].time,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].time));
			write_string(ADDR_SHIDU_LIST_DATA_4,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].data,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-5].data));
			write_string(ADDR_SHIDU_LIST_TIME_5,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].time,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].time));
			write_string(ADDR_SHIDU_LIST_DATA_5,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].data,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-6].data));
			write_string(ADDR_SHIDU_LIST_TIME_6,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].time,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].time));
			write_string(ADDR_SHIDU_LIST_DATA_6,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].data,
				strlen(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
//		printfLog(LCD_PROCESS"g_temp_cnt %d\n",*g_temp_cnt);
		if((g_share_memory->cnt[SENSOR_TEMP]-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7 + 1);
			write_string(ADDR_TEMP_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",g_share_memory->cnt[SENSOR_TEMP]/7);			
			write_string(ADDR_TEMP_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(ADDR_TEMP_LIST_TIME_0,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].time));
			write_string(ADDR_TEMP_LIST_DATA_0,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].data,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-1].data));
			write_string(ADDR_TEMP_LIST_TIME_1,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].time,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].time));
			write_string(ADDR_TEMP_LIST_DATA_1,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].data,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-2].data));
			write_string(ADDR_TEMP_LIST_TIME_2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].time,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].time));
			write_string(ADDR_TEMP_LIST_DATA_2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].data,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-3].data));
			write_string(ADDR_TEMP_LIST_TIME_3,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].time,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].time));
			write_string(ADDR_TEMP_LIST_DATA_3,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].data,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-4].data));
			write_string(ADDR_TEMP_LIST_TIME_4,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].time,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].time));
			write_string(ADDR_TEMP_LIST_DATA_4,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].data,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-5].data));
			write_string(ADDR_TEMP_LIST_TIME_5,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].time,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].time));
			write_string(ADDR_TEMP_LIST_DATA_5,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].data,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-6].data));
			write_string(ADDR_TEMP_LIST_TIME_6,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].time,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].time));
			write_string(ADDR_TEMP_LIST_DATA_6,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].data,
				strlen(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-offset-7].data));
		}
	}
	if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
//		printfLog(LCD_PROCESS"g_pm25_cnt %d\n",*g_pm25_cnt);
		if((g_share_memory->cnt[SENSOR_PM25]-offset-7)>0)
		{
			sprintf(tmp,"%4d",offset/7 + 1);
			write_string(ADDR_PM25_PAGE_NUM,tmp,strlen(tmp));
			memset(tmp,'\0',4);
			sprintf(tmp,"%4ld",g_share_memory->cnt[SENSOR_PM25]/7);			
			clear_buf(ADDR_PM25_LIST_DATA_0,4);
			clear_buf(ADDR_PM25_LIST_DATA_1,4);
			clear_buf(ADDR_PM25_LIST_DATA_2,4);
			clear_buf(ADDR_PM25_LIST_DATA_3,4);
			clear_buf(ADDR_PM25_LIST_DATA_4,4);
			clear_buf(ADDR_PM25_LIST_DATA_5,4);
			clear_buf(ADDR_PM25_LIST_DATA_6,4);
			write_string(ADDR_PM25_PAGE_TOTAL,tmp,strlen(tmp));
			write_string(ADDR_PM25_LIST_TIME_0,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].time));
			write_string(ADDR_PM25_LIST_DATA_0,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].data,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-1].data));
			write_string(ADDR_PM25_LIST_TIME_1,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].time,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].time));
			write_string(ADDR_PM25_LIST_DATA_1,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].data,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-2].data));
			write_string(ADDR_PM25_LIST_TIME_2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].time,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].time));
			write_string(ADDR_PM25_LIST_DATA_2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].data,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-3].data));
			write_string(ADDR_PM25_LIST_TIME_3,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].time,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].time));
			write_string(ADDR_PM25_LIST_DATA_3,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].data,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-4].data));
			write_string(ADDR_PM25_LIST_TIME_4,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].time,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].time));
			write_string(ADDR_PM25_LIST_DATA_4,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].data,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-5].data));
			write_string(ADDR_PM25_LIST_TIME_5,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].time,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].time));
			write_string(ADDR_PM25_LIST_DATA_5,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].data,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-6].data));
			write_string(ADDR_PM25_LIST_TIME_6,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].time,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].time));
			write_string(ADDR_PM25_LIST_DATA_6,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].data,
				strlen(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-offset-7].data));
		}
	}
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
	if(read_dgus(ADDR_AP_NAME,10,ap_name) && read_dgus(ADDR_AP_PASSWD,10,ap_passwd))
	{
		printfLog(LCD_PROCESS"AP Name %s \nAP Pwd %s\n",ap_name,ap_passwd);
		if(strlen(ap_passwd)<8||strlen(ap_name)==0)
			return ;
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
		printfLog(LCD_PROCESS"exec %s\n",cmd);
		system(cmd);	
		sprintf(cmd,"wpa_cli -ira0 set_network %d psk \\\"%s\\\"",i,ap_passwd);
		printfLog(LCD_PROCESS"exec %s\n",cmd);
		system(cmd);		
		sprintf(cmd,"wpa_cli -ira0 set_network %d ssid \\\"%s\\\"",i,ap_name);
		printfLog(LCD_PROCESS"exec %s\n",cmd);
		system(cmd);
		sprintf(cmd,"wpa_cli -ira0 enable_network %d",i);
		printfLog(LCD_PROCESS"exec %s\n",cmd);
		system(cmd);
		//sprintf(cmd,"wpa_cli -ira0 select_network %d",i);
		//printf("exec %s\n",cmd);
		//system(cmd);
		sprintf(cmd,"wpa_cli -ira0 save_config");
		printfLog(LCD_PROCESS"exec %s\n",cmd);		
		system(cmd);
		sprintf(cmd,"udhcpc -i ra0");
		printfLog(LCD_PROCESS"exec %s\n",cmd);
		//system(cmd);
	}
}

void show_curve(char *id,int* offset)
{
	int buf[24]={0};
	char info[20]={0};
	char temp[10]={0},temp2[10]={0};
	char hour1[3]={0},hour2[3]={0};
	char index[5][17]={{0},{0}};
	char index_time[5][3]={{0},{0}};
	int i=0,j=0,m=0;
	clear_curve();
	if(strncmp(id,ID_CAP_CO,strlen(id))==0)
	{
		//printfLog(LCD_PROCESS"g_co_cnt %d\n",*g_co_cnt);
		if((g_share_memory->cnt[SENSOR_CO]-*offset)>0)
		{
			strcpy(index[0],"10");
			strcpy(index[1],"7.5");
			strcpy(index[2],"5.0");
			strcpy(index[3],"2.5");
			strcpy(index[4],"0");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"CO");
			write_string(ADDR_CURVE_DATE, sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-1].time,10);
			memcpy(temp,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printfLog(LCD_PROCESS"hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-i-1].data+2)*10*5;
								printfLog(LCD_PROCESS"j %d %s==>%d\n",j,
									sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-i-1].time,buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,sensor_history.co[g_share_memory->cnt[SENSOR_CO]-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printfLog(LCD_PROCESS"j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			//*offset+=i;
			if(*offset>=g_share_memory->cnt[SENSOR_CO])
				*offset=0;
		}
		else
			*offset=0;
	}
	if(strncmp(id,ID_CAP_CO2,strlen(id))==0)
	{
		//printfLog(LCD_PROCESS"g_co2_cnt %d\n",*g_co2_cnt);
		if((g_share_memory->cnt[SENSOR_CO2]-*offset)>0)
		{
			strcpy(index[0],"0.2000");
			strcpy(index[1],"0.1500");
			strcpy(index[2],"0.1000");
			strcpy(index[3],"0.0500");
			strcpy(index[4],"0.0000");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"CO2");
			write_string(ADDR_CURVE_DATE, sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-1].time,10);
			memcpy(temp,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printfLog(LCD_PROCESS"hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-i-1].data+2);
								printfLog(LCD_PROCESS"j %d %s==>%d\n",j,
									sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-i-1].time,buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,sensor_history.co2[g_share_memory->cnt[SENSOR_CO2]-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printfLog(LCD_PROCESS"j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			//*offset+=i;
			if(*offset>=g_share_memory->cnt[SENSOR_CO2])
				*offset=0;
		}
		else
			*offset=0;
	}
	if(strncmp(id,ID_CAP_HCHO,strlen(id))==0)
	{
		//printf("g_co_cnt %d\n",*g_hcho_cnt);
		if((g_share_memory->cnt[SENSOR_HCHO]-*offset)>0)
		{
			strcpy(index[0],"1.00");
			strcpy(index[1],"0.75");
			strcpy(index[2],"0.50");
			strcpy(index[3],"0.25");
			strcpy(index[4],"0.0");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"HCHO");
			write_string(ADDR_CURVE_DATE, sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-1].time,10);
			memcpy(temp,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printfLog(LCD_PROCESS"hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=(atoi(sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].data+2)*667*3)/500;
								printfLog(LCD_PROCESS"j %d %s==>%d\n",j,
									sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].time,buf[j-1]);							
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,sensor_history.hcho[g_share_memory->cnt[SENSOR_HCHO]-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printfLog(LCD_PROCESS"j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			//*offset+=i;
			if(*offset>=g_share_memory->cnt[SENSOR_HCHO])
				*offset=0;

		}
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_SHI_DU,strlen(id))==0)
	{
		//printf("g_shidu_cnt %d\n",*g_shidu_cnt);
		if((g_share_memory->cnt[SENSOR_SHIDU]-*offset-7)>0)
		{
			char info_l[]={"湿度"};
			strcpy(index[0],"90");
			strcpy(index[1],"68");
			strcpy(index[2],"45");
			strcpy(index[3],"23");
			strcpy(index[4],"0");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			//strcpy(info,"Humidity");
			sprintf(info,"%s",info_l);
			write_string(ADDR_CURVE_DATE, sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-1].time,10);
			memcpy(temp,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printfLog(LCD_PROCESS"hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-i-1].data)*22;
								printfLog(LCD_PROCESS"j %d %s==>%d\n",j,
									sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-i-1].time,buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,sensor_history.shidu[g_share_memory->cnt[SENSOR_SHIDU]-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printfLog(LCD_PROCESS"j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			//*offset+=i;
			if(*offset>=g_share_memory->cnt[SENSOR_SHIDU])
				*offset=0;

		}
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_TEMPERATURE,strlen(id))==0)
	{
		//printf("g_temp_cnt %d\n",*g_temp_cnt);
		if((g_share_memory->cnt[SENSOR_TEMP]-*offset-7)>0)
		{
			char info_l[]={"温度"};
			strcpy(index[0],"60");
			strcpy(index[1],"40");
			strcpy(index[2],"20");
			strcpy(index[3],"0");
			strcpy(index[4],"-20");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			//strcpy(info,"Temperature");
			sprintf(info,"%s",info_l);
			write_string(ADDR_CURVE_DATE, sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-1].time,10);
			memcpy(temp,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printfLog(LCD_PROCESS"hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=(atoi(sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-i-1].data)+20)*25;
								printfLog(LCD_PROCESS"j %d %s==>%d\n",j,
									sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-i-1].time,buf[j-1]);
							}
							
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,sensor_history.temp[g_share_memory->cnt[SENSOR_TEMP]-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printfLog(LCD_PROCESS"j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			//*offset+=i;
			if(*offset>=g_share_memory->cnt[SENSOR_TEMP])
				*offset=0;
		}
		else
			*offset=0;
	}	
	if(strncmp(id,ID_CAP_PM_25,strlen(id))==0)
	{
		//printf("g_pm25_cnt %d\n",*g_pm25_cnt);
		if((g_share_memory->cnt[SENSOR_PM25]-*offset)>0)
		{
			strcpy(index[0],"500");
			strcpy(index[1],"375");
			strcpy(index[2],"250");
			strcpy(index[3],"125");
			strcpy(index[4],"0");
			strcpy(index_time[0],"24");
			strcpy(index_time[1],"18");
			strcpy(index_time[2],"12");
			strcpy(index_time[3],"06");
			strcpy(index_time[4],"00");	
			strcpy(info,"PM25");
			write_string(ADDR_CURVE_DATE, sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-1].time,10);
			memcpy(temp,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-1].time,10);
			strcpy(hour1,"24");
			memcpy(hour2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-1].time+11,2);
			while(1)
			{
				memcpy(temp2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].time,10);	
				if(strncmp(temp,temp2,10)==0)
				{				
					if(strncmp(hour1,hour2,2)!=0)
					{	
						printfLog(LCD_PROCESS"hour1 %s,hour2 %s\n",hour1,hour2);
						for(m=atoi(hour1);m>atoi(hour2);m--)
						{
							if(j<24)
							{
								buf[j++]=atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data)*2*2;
								printfLog(LCD_PROCESS"j %d %s==>%d %d\n",j,
									sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].time,
									atoi(sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].data),buf[j-1]);
							}
						}					
						memcpy(hour1,hour2,2);
					}
					i++;
					memcpy(hour2,sensor_history.pm25[g_share_memory->cnt[SENSOR_PM25]-*offset-i-1].time+11,2);
				}
				else
					break;
			}
			printfLog(LCD_PROCESS"j is %d\n",j);
			if(j!=24)
			{
				for(m=j;m<24;m++)
					buf[m]=buf[j-1];
				j=24;
			}
			//*offset+=i;
			if(*offset>=g_share_memory->cnt[SENSOR_PM25])
				*offset=0;
		}
		else
			*offset=0;
	}
	//write index
	clear_buf(ADDR_CURVE_TYPE,10);
	clear_buf(ADDR_CURVE_DATA_5,16);
	clear_buf(ADDR_CURVE_DATA_4,16);
	clear_buf(ADDR_CURVE_DATA_3,16);
	clear_buf(ADDR_CURVE_DATA_2,16);
	clear_buf(ADDR_CURVE_DATA_1,16);
	write_string(ADDR_CURVE_TYPE, info,strlen(info));
	write_string(ADDR_CURVE_TIME_5, index_time[0],2);
	write_string(ADDR_CURVE_TIME_4, index_time[1],2);
	write_string(ADDR_CURVE_TIME_3, index_time[2],2);
	write_string(ADDR_CURVE_TIME_2, index_time[3],2);
	write_string(ADDR_CURVE_TIME_1, index_time[4],2);
	write_string(ADDR_CURVE_DATA_5, index[0], strlen(index[0]));
	write_string(ADDR_CURVE_DATA_4, index[1], strlen(index[1]));
	write_string(ADDR_CURVE_DATA_3, index[2], strlen(index[2]));
	write_string(ADDR_CURVE_DATA_2, index[3], strlen(index[3]));
	write_string(ADDR_CURVE_DATA_1, index[4], strlen(index[4]));
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
			printfLog(LCD_PROCESS"==> %x\n",ch);
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
	write_string(ADDR_TIME_YEAR,buf,4);
	memset(buf,0,5);
	sprintf(buf,"%d",day);
	write_string(ADDR_TIME_DAY,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",mon);
	write_string(ADDR_TIME_MONTH,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",hour);
	write_string(ADDR_TIME_HOUR,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",min);
	write_string(ADDR_TIME_MIN,buf,2);
	memset(buf,0,5);
	sprintf(buf,"%d",seconds);
	write_string(ADDR_TIME_SECOND,buf,2);
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
	if(read_dgus(ADDR_USER_NAME_VERIFY,5,user_name) && read_dgus(ADDR_USER_PWD_VERIFY,5,passwd))
	{
		printfLog(LCD_PROCESS"User Name %s \nUser Pwd %s\n",user_name,passwd);
		if(verify_pwd(user_name,passwd))
		{
			if(g_index!=SYSTEM_SET_PAGE)
			{
				if(g_share_memory->history_done)
					switch_pic(CURVE_PAGE);
				else
					switch_pic(MAIN_PAGE);	
			}
			else
				switch_pic(SENSOR_TEST_PAGE);
			return;
		}
	}
	if(g_index!=SYSTEM_SET_PAGE)
		switch_pic(MAIN_PAGE);
	else
		switch_pic(SYSTEM_SET_PAGE);
}
void show_cur_select_intr(int sel)
{
	char name[256]={0};
	clear_buf(ADDR_CUR_SELECT,10);	
	interface_to_string(sel,name);
	write_string(ADDR_CUR_SELECT,name,strlen(name));
}
void show_cur_interface(int page)
{
	char name[256]={0};
	if(page==INTERFACE_SELECT_PAGE)
	{
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
		write_string(ADDR_INTERFACE_1,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[1],name);
		write_string(ADDR_INTERFACE_2,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[2],name);
		write_string(ADDR_INTERFACE_3,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[3],name);
		write_string(ADDR_INTERFACE_4,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[4],name);
		write_string(ADDR_INTERFACE_5,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[5],name);
		write_string(ADDR_INTERFACE_6,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[6],name);
		write_string(ADDR_INTERFACE_7,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[7],name);
		write_string(ADDR_INTERFACE_8,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[8],name);
		write_string(ADDR_INTERFACE_9,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[9],name);
		write_string(ADDR_INTERFACE_10,name,strlen(name));
		interface_to_string(g_share_memory->sensor_interface_mem[10],name);
		write_string(ADDR_INTERFACE_11,name,strlen(name));
	}
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
	}
	
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
	if(read_dgus(ADDR_TIME_YEAR,2,year) && read_dgus(ADDR_TIME_DAY,1,day)
		&& read_dgus(ADDR_TIME_MONTH,1,mon) && read_dgus(ADDR_TIME_HOUR,1,hour)
		&& read_dgus(ADDR_TIME_MIN,1,min) && read_dgus(ADDR_TIME_SECOND,1,second))
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
			write(g_share_memory->fd_com,g_share_memory->server_time,13);
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
		switch_pic(VERIFY_PAGE);
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
			write(g_share_memory->fd_com,cmd_verify,sizeof(cmd_verify));
		}
	}
	else
	{
		
		switch_pic(SENSOR_TEST_PAGE);
		if(g_share_memory->factory_mode==SENSOR_VERIFY_MODE)
		{
			g_share_memory->factory_mode=NORMAL_MODE;
			printfLog(LCD_PROCESS"End to JiaoZhun %d\n",sensor);
			//send_return(fd,sensor,jp);
		}
	}
}
int getxiuzhen()
{
	char data[10]={0};
	int d=0,p_get=0,y=0;
	if(read_dgus(ADDR_VERIFY_VALUE,5,data))
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
	cmd_return[5]=sensor+1;
	cmd_return[6]=jp+1;
	cmd_return[7]=(xz>>8)&0xff;
	cmd_return[8]=xz&0xff;
	int crc=CRC_check((unsigned char *)cmd_return,9);
	cmd_return[9]=(crc&0xff00)>>8;cmd_return[10]=crc&0x00ff;		
	for(i=0;i<sizeof(cmd_return);i++)
		printfLog(LCD_PROCESS"%02X ",cmd_return[i]);
	printfLog(LCD_PROCESS"\n");
	write(g_share_memory->fd_com,cmd_return,sizeof(cmd_return));
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
	return NULL;
}
void show_point(int index,char sensor)
{
	char cmd0[]={0x5a,0xa5,0x15,0x82,0x08,0x31,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x00,0xb7};
	
	char cmd1[]={0x5a,0xa5,0x15,0x82,0x08,0x3a,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x00,0xeb};
	
	char cmd2[]={0x5a,0xa5,0x15,0x82,0x08,0x43,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x01,0x1e};
	
	char cmd3[]={0x5a,0xa5,0x15,0x82,0x08,0x4c,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x01,0x51};
	
	char cmd4[]={0x5a,0xa5,0x15,0x82,0x08,0x55,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x01,0x86};
	
	char cmd5[]={0x5a,0xa5,0x15,0x82,0x08,0x5e,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x01,0xba};
	
	char cmd6[]={0x5a,0xa5,0x15,0x82,0x08,0x67,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x01,0xed};
	
	char cmd7[]={0x5a,0xa5,0x15,0x82,0x08,0x70,0x00,0x06,0x00,0x01,0x00,0x16,
				0x02,0x8e,0x01,0x5a,0x02,0xa1,0x01,0x6c,0x00,0xe7,0x02,0x21};
	
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
	char cmd[64]={0};
	sprintf(cmd,"%f",g_share_memory->x[index]);
	write_string(ADDR_VERIFY_VALUE,cmd,strlen(cmd));
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
		printfLog(LCD_PROCESS"%02x ",cmd[i]);
	printfLog(LCD_PROCESS"\ngoing to set_interface end\n");
	write(g_share_memory->fd_com,cmd,sizeof(cmd));
}

void tun_zero(int on)
{
	char cmd_request_verify[]=	{0x6c,ARM_TO_CAP,0x00,0x07,0x01,0x00,0x00,0x00};	
	int crc = 0;
	int i =0;
	//send cap board start co & hcho
	clear_buf(ADDR_TUN_ZERO_HCHO,4);
	clear_buf(ADDR_TUN_ZERO_CO,4);
	//for(i=0;i<11;i++)
	//	if(sensor_interface_mem[i] == TYPE_SENSOR_CO_WEISHEN ||
	//	sensor_interface_mem[i] == TYPE_SENSOR_CO_DD)
	//	break;
	//printf("CO interface %d %4x\n",i,sensor_interface_mem[i]);
	if(on)
	{		
		printfLog(LCD_PROCESS"Start Co,ch2o tun zero\n");
		cmd_request_verify[5]=0x01;
	}
	else
	{
		if(g_share_memory->factory_mode==TUN_ZERO_MODE)
		{
			return_zero_point(1);
			sleep(1);
			return_zero_point(0);
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
	write(g_share_memory->fd_com,cmd_request_verify,sizeof(cmd_request_verify));
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
	static int curve_co=0;
	static int curve_co2=0;
	static int curve_hcho=0;
	static int curve_temp=0;
	static int curve_shidu=0;
	static int curve_pm25=0;
	static int interface_config_no = 0;
	static int cur_select_interface = TYPE_SENSOR_CO_WEISHEN;
	//char * line = NULL;
	//char date1[32]={0};
	//char date2[32]={0};
	//char date3[32]={0};
	//char data1[32]={0};
	//char data2[32]={0};
	//char data3[32]={0};
	//char time[]={0x32,0x30,0x31 ,0x35 ,0x2d,0x31 ,0x31 ,0x2d ,0x32 ,0x30 ,0x20,0x32 ,0x37 ,0x3A ,0x32 ,0x30 ,0x3A ,0x30 ,0x30};
	//char co[]={0x30,0x2e,0x31,0x32};
	input[0]=2;
	addr=input[1]<<8|input[2];
	data=input[4]<<8|input[5];
	printfLog(LCD_PROCESS"got press %04x %04x\r\n",addr,data);
	if(lcd_state==0)
	{
		if(g_index==TUN_ZERO_PAGE)
			lcd_on(TUN_ZERO_PAGE);
		else if(g_index==VERIFY_SELECT_PAGE)
			lcd_on(VERIFY_SELECT_PAGE);
		else if(g_index==VERIFY_PAGE)
			lcd_on(VERIFY_PAGE);
		else
			lcd_on(MAIN_PAGE);
		alarm(300);
	}
	else
	{
		alarm(0);
		alarm(300);
	}
	if(addr==TOUCH_RUN_TIME_CO && (TOUCH_RUN_TIME_CO+0x100)==data)
	{//show history CO the first page
		if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE);
				show_curve(ID_CAP_CO,&curve_co);
			}
			else
			{
				switch_pic(LIST_CO_PAGE);
				show_history(ID_CAP_CO,begin_co);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(ADDR_USER_NAME_VERIFY,16);
			clear_buf(ADDR_USER_PWD_VERIFY,16);
			switch_pic(LOG_IN_PAGE);
		}
		g_index=LIST_CO_PAGE;
	}
	else if(addr==TOUCH_RUN_TIME_CO2 && (TOUCH_RUN_TIME_CO2+0x100)==data)
	{//show history CO2 the first page
		if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE);
				show_curve(ID_CAP_CO2,&curve_co2);
			}
			else
			{
				switch_pic(LIST_CO2_PAGE);
				show_history(ID_CAP_CO2,begin_co2);
				//begin_co2=0;
			}
		}
		else
		{
			clear_buf(ADDR_USER_NAME_VERIFY,16);
			clear_buf(ADDR_USER_PWD_VERIFY,16);
			switch_pic(LOG_IN_PAGE);	
		}
		g_index=LIST_CO2_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_HCHO && (TOUCH_RUN_TIME_HCHO+0x100)==data)
	{//show history HCHO the first page	
		if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE);
				show_curve(ID_CAP_HCHO,&curve_hcho);
			}
			else
			{
				switch_pic(LIST_HCHO_PAGE);
				show_history(ID_CAP_HCHO,begin_hcho);
				//begin_co=0;
			}
		}
		else
		{		
			clear_buf(ADDR_USER_NAME_VERIFY,16);
			clear_buf(ADDR_USER_PWD_VERIFY,16);
			switch_pic(LOG_IN_PAGE);			
		}
		g_index=LIST_HCHO_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_SHIDU && (TOUCH_RUN_TIME_SHIDU+0x100)==data)
	{//show history SHIDU the first page
		if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE);
				show_curve(ID_CAP_SHI_DU,&curve_shidu);
			}
			else
			{
				switch_pic(LIST_SHIDU_PAGE);
				show_history(ID_CAP_SHI_DU,begin_shidu);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(ADDR_USER_NAME_VERIFY,16);
			clear_buf(ADDR_USER_PWD_VERIFY,16);
			switch_pic(LOG_IN_PAGE);
		}		
		g_index=LIST_SHIDU_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_TEMP && (TOUCH_RUN_TIME_TEMP+0x100)==data)
	{//show history TEMPERATURE the first page
		if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE);
				show_curve(ID_CAP_TEMPERATURE,&curve_temp);
			}
			else
			{
				switch_pic(LIST_TEMP_PAGE);
				show_history(ID_CAP_TEMPERATURE,begin_temp);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(ADDR_USER_NAME_VERIFY,16);
			clear_buf(ADDR_USER_PWD_VERIFY,16);
			switch_pic(LOG_IN_PAGE);
		}		
		g_index=LIST_TEMP_PAGE;
	}	
	else if(addr==TOUCH_RUN_TIME_PM25&& (TOUCH_RUN_TIME_PM25+0x100)==data)
	{//show history PM25 the first page
		if(logged)
		{
			if(g_share_memory->history_done)
			{
				switch_pic(CURVE_PAGE);
				show_curve(ID_CAP_PM_25,&curve_pm25);
			}
			else
			{
				switch_pic(LIST_PM25_PAGE);
				show_history(ID_CAP_PM_25,begin_pm25);
				//begin_co=0;
			}
		}
		else
		{
			clear_buf(ADDR_USER_NAME_VERIFY,16);
			clear_buf(ADDR_USER_PWD_VERIFY,16);
			switch_pic(LOG_IN_PAGE);
		}
		g_index=LIST_PM25_PAGE;
	}	
	else if(addr==TOUCH_CO_REFRESH_PAGE && (TOUCH_CO_REFRESH_PAGE+0x100)==data)
	{//show history CO the next page
		begin_co=0;
		show_history(ID_CAP_CO,begin_co);
	}
	else if(addr==TOUCH_CO_LAST_PAGE && (TOUCH_CO_LAST_PAGE+0x100)==data)
	{//show history CO the next page
		if(begin_co>=7)
		{
			begin_co-=7;
			show_history(ID_CAP_CO,begin_co);
		}
	}
	else if(addr==TOUCH_CO_NEXT_PAGE && (TOUCH_CO_NEXT_PAGE+0x100)==data)
	{//show history CO the next page
		if(begin_co+7<g_share_memory->cnt[SENSOR_CO])
		{
			begin_co+=7;
			show_history(ID_CAP_CO,begin_co);
		}
	}
	else if(addr==TOUCH_CO2_REFRESH_PAGE && (TOUCH_CO2_REFRESH_PAGE+0x100)==data)
	{//show history CO2 the next page
		begin_co2=0;
		show_history(ID_CAP_CO2,begin_co2);
	}
	else if(addr==TOUCH_CO2_LAST_PAGE && (TOUCH_CO2_LAST_PAGE+0x100)==data)
	{//show history CO2 the next page
		if(begin_co2>=7)
		{
			begin_co2-=7;
			show_history(ID_CAP_CO2,begin_co2);
		}
	}
	else if(addr==TOUCH_CO2_NEXT_PAGE && (TOUCH_CO2_NEXT_PAGE+0x100)==data)
	{//show history CO2 the next page
		if(begin_co2+7<g_share_memory->cnt[SENSOR_CO2])
		{
			begin_co2+=7;
			show_history(ID_CAP_CO2,begin_co2);
		}
	}
	else if(addr==TOUCH_HCHO_REFRESH_PAGE && (TOUCH_HCHO_REFRESH_PAGE+0x100)==data)
	{//show history HCHO the next page
		begin_hcho=0;
		show_history(ID_CAP_HCHO,begin_hcho);
	}
	else if(addr==TOUCH_HCHO_LAST_PAGE && (TOUCH_HCHO_LAST_PAGE+0x100)==data)
	{//show history HCHO the next page
		if(begin_hcho>=7)
		{
			begin_hcho-=7;
			show_history(ID_CAP_HCHO,begin_hcho);
		}
	}
	else if(addr==TOUCH_HCHO_NEXT_PAGE && (TOUCH_HCHO_NEXT_PAGE+0x100)==data)
	{//show history HCHO the next page
		if(begin_hcho+7<g_share_memory->cnt[SENSOR_HCHO])
		{
			begin_hcho+=7;
			show_history(ID_CAP_HCHO,begin_hcho);
		}
	}
	else if(addr==TOUCH_TEMP_REFRESH_PAGE && (TOUCH_TEMP_REFRESH_PAGE+0x100)==data)
	{//show history TEMPERATURE the next page
		begin_temp=0;
		show_history(ID_CAP_TEMPERATURE,begin_temp);
	}
	else if(addr==TOUCH_TEMP_LAST_PAGE && (TOUCH_TEMP_LAST_PAGE+0x100)==data)
	{//show history TEMP the next page
		if(begin_temp>=7)
		{
			begin_temp-=7;
			show_history(ID_CAP_TEMPERATURE,begin_temp);
		}
	}
	else if(addr==TOUCH_TEMP_NEXT_PAGE && (TOUCH_TEMP_NEXT_PAGE+0x100)==data)
	{//show history TEMP the next page
		if(begin_temp+7<g_share_memory->cnt[SENSOR_TEMP])
		{
			begin_temp+=7;
			show_history(ID_CAP_TEMPERATURE,begin_temp);
		}
	}
	else if(addr==TOUCH_SHIDU_REFRESH_PAGE&& (TOUCH_SHIDU_REFRESH_PAGE+0x100)==data)
	{//show history SHIDU the next page
		begin_shidu=0;
		show_history(ID_CAP_SHI_DU,begin_shidu);
	}
	else if(addr==TOUCH_SHIDU_LAST_PAGE && (TOUCH_SHIDU_LAST_PAGE+0x100)==data)
	{//show history SHIDU the next page
		if(begin_shidu>=7)
		{
			begin_shidu-=7;
			show_history(ID_CAP_SHI_DU,begin_shidu);
		}
	}
	else if(addr==TOUCH_SHIDU_NEXT_PAGE && (TOUCH_SHIDU_NEXT_PAGE+0x100)==data)
	{//show history SHIDU the next page
		if(begin_shidu+7<g_share_memory->cnt[SENSOR_SHIDU])
		{
			begin_shidu+=7;
			show_history(ID_CAP_SHI_DU,begin_shidu);
		}
	}
	else if(addr==TOUCH_PM25_REFRESH_PAGE && (TOUCH_PM25_REFRESH_PAGE+0x100)==data)
	{//show history PM25 the next page
		begin_pm25=0;
		show_history(ID_CAP_PM_25,begin_pm25);
	}
	else if(addr==TOUCH_PM25_LAST_PAGE && (TOUCH_PM25_LAST_PAGE+0x100)==data)
	{//show history PM25 the next page
		if(begin_pm25>=7)
		{
			begin_pm25-=7;
			show_history(ID_CAP_PM_25,begin_pm25);
		}
	}
	else if(addr==TOUCH_PM25_NEXT_PAGE && (TOUCH_PM25_NEXT_PAGE+0x100)==data)
	{//show history PM25 the next page
		if(begin_pm25+7<g_share_memory->cnt[SENSOR_PM25])
		{
			begin_pm25+=7;
			show_history(ID_CAP_PM_25,begin_pm25);
		}
	}	
	else if(addr==TOUCH_DEVICE_STATE&& (TOUCH_DEVICE_STATE+0x100)==data)
	{//show sensor and network state
		show_sensor_network();
	}
	else if(addr==TOUCH_TIME_SET&& (TOUCH_TIME_SET+0x100)==data)
	{//set time
		clear_buf(ADDR_TIME_YEAR,4);
		clear_buf(ADDR_TIME_MONTH,2);
		clear_buf(ADDR_TIME_DAY,2);
		clear_buf(ADDR_TIME_HOUR,2);
		clear_buf(ADDR_TIME_MIN,2);
		clear_buf(ADDR_TIME_SECOND,2);
		//switch_pic(fd_lcd, TIME_SETTING_PAGE);
	}
	else if(addr==TOUCH_SELECT_OK&& (TOUCH_SELECT_OK+0x100)==data)
	{//WiFi Passwd changed
		g_share_memory->send_by_wifi=wifi_select;
		if(wifi_select)
		{
			wifi_handle();
			switch_pic(WIFI_PAGE);
		}
		else
			switch_pic(GPRS_PAGE);
		set_net_interface();		
	}
	else if((addr==TOUCH_SELECT_WIFI)&& (TOUCH_SELECT_WIFI+0x100)==data)
	{//WiFi Passwd changed
		//wifi_handle(fd_lcd);
		wifi_select=1;
		//set_net_interface();
		write_string(ADDR_XFER_MODE,"WIFI",strlen("WIFI"));
	}
	else if(addr==TOUCH_SELECT_RETURN&& (TOUCH_SELECT_RETURN+0x100)==data)
	{//use gprs to xfer
		wifi_select=g_share_memory->send_by_wifi;
	}
	else if(addr==TOUCH_SELECT_GPRS&& (TOUCH_SELECT_GPRS+0x100)==data)
	{//use gprs to xfer
		wifi_select=0;
		//set_net_interface();
		write_string(ADDR_XFER_MODE,"GPRS",strlen("GPRS"));
	}
	else if(addr==TOUCH_XFER_SET&&(TOUCH_XFER_SET+0x100)==data)
	{//enter wifi passwd setting
		clear_buf(ADDR_AP_NAME,20);
		clear_buf(ADDR_AP_PASSWD,20);
		if(g_share_memory->send_by_wifi)
		{
			write_string(ADDR_XFER_MODE,"WIFI",strlen("WIFI"));
		}
		else
		{
			write_string(ADDR_XFER_MODE,"GPRS",strlen("GPRS"));
		}
	}
	else if((addr==TOUCH_TIME_CHANGE_OK && (TOUCH_TIME_CHANGE_OK+0x100)==data)
		|| ((addr==TOUCH_TIME_CHANGE_MANUL) && (TOUCH_TIME_CHANGE_MANUL+0x100)==data))
	{//manul set time
		manul_set_time();
		if(addr==TOUCH_TIME_CHANGE_OK)
		switch_pic(SYSTEM_SET_PAGE);
	}
	else if(addr==TOUCH_TIME_CHANGE_SERVER && (TOUCH_TIME_CHANGE_SERVER+0x100)==data)
	{//set time from server
		sync_server(0,0);
		display_time(g_share_memory->server_time[5]+2000,g_share_memory->server_time[6],g_share_memory->server_time[7],
			g_share_memory->server_time[8],g_share_memory->server_time[9],g_share_memory->server_time[10]);
		//sleep(3);
		//switch_pic(fd_lcd,18);
	}
	else if(addr==TOUCH_VERIFY_HCHO && (TOUCH_VERIFY_HCHO+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=0;
		g_share_memory->jiaozhun_sensor=atoi(ID_CAP_HCHO);
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_PM25 && (TOUCH_VERIFY_PM25+0x100)==data)
	{
		g_index=VERIFY_PAGE;		
		verify_object=1;
		g_share_memory->jiaozhun_sensor=atoi(ID_CAP_PM_25);
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_INT3 && (TOUCH_VERIFY_INT3+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=2;
		g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_INT4 && (TOUCH_VERIFY_INT4+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=3;
		g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_INT5 && (TOUCH_VERIFY_INT5+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=4;
		g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_INT6 && (TOUCH_VERIFY_INT6+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=5;
		g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_INT7 && (TOUCH_VERIFY_INT7+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=6;
		g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_INT8 && (TOUCH_VERIFY_INT8+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=7;
		g_share_memory->jiaozhun_sensor=atoi(Get_Type(verify_object));
		jiaozhun(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_VERIFY_WENSHI && (TOUCH_VERIFY_WENSHI+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=8;
		jiaozhun(1,verify_object,verify_point);
		g_share_memory->jiaozhun_sensor=atoi(ID_CAP_TEMPERATURE);
	}
	else if(addr==TOUCH_VERIFY_FENGSU && (TOUCH_VERIFY_FENGSU+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=9;
		jiaozhun(1,verify_object,verify_point);
		g_share_memory->jiaozhun_sensor=atoi(ID_CAP_FENG_SU);
	}
	else if(addr==TOUCH_VERIFY_QIYA && (TOUCH_VERIFY_QIYA+0x100)==data)
	{
		g_index=VERIFY_PAGE;
		verify_object=10;
		jiaozhun(1,verify_object,verify_point);
		g_share_memory->jiaozhun_sensor=atoi(ID_CAP_QI_YA);
	}
	else if(addr==TOUCH_VERIFY && (TOUCH_VERIFY+0x100)==data)
	{	//verify sensor display
		g_share_memory->sensor_interface_mem[0]=0x1234;
		ask_interface();
		show_cur_interface(VERIFY_SELECT_PAGE);
		g_index=VERIFY_SELECT_PAGE;
	}
	else if(addr==TOUCH_VERIFY_EXIT && (TOUCH_VERIFY_EXIT+0x100)==data)
	{	//verify sensor display
		if(verify_point==0)
			tun_zero(0);
		verify_point=100;
		jiaozhun(0,verify_object,verify_point);
	}
	else if(addr==ADDR_VERIFY_VALUE)
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
		ask_interface();
		clear_buf(ADDR_TUN_ZERO_HCHO,4);
		clear_buf(ADDR_TUN_ZERO_CO,4);
	}
	else if(addr==TOUCH_TUN_ZERO_BEGIN && (TOUCH_TUN_ZERO_BEGIN+0x100)==data)
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
			tun_zero(0);
			g_share_memory->factory_mode=NORMAL_MODE;
		}
	}
	else if(addr==TOUCH_INTERFACE_SET && (TOUCH_INTERFACE_SET+0x100)==data)
	{//set sensor interface
		g_share_memory->sensor_interface_mem[0]=0x1234;
		ask_interface();
		show_cur_interface(INTERFACE_SELECT_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_1 && (TOUCH_INTERFACE_1+0x100)==data)
	{
		interface_config_no=0;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		//switch_pic(fd_lcd,INTERFACE_SELECT_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_2 && (TOUCH_INTERFACE_2+0x100)==data)
	{
		interface_config_no=1;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_3 && (TOUCH_INTERFACE_3+0x100)==data)
	{
		interface_config_no=2;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_4 && (TOUCH_INTERFACE_4+0x100)==data)
	{
		interface_config_no=3;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_5 && (TOUCH_INTERFACE_5+0x100)==data)
	{
		interface_config_no=4;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}		
	else if(addr==TOUCH_INTERFACE_6 && (TOUCH_INTERFACE_6+0x100)==data)
	{
		interface_config_no=5;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_7 && (TOUCH_INTERFACE_7+0x100)==data)
	{
		interface_config_no=6;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_8 && (TOUCH_INTERFACE_8+0x100)==data)
	{
		interface_config_no=7;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_9 && (TOUCH_INTERFACE_9+0x100)==data)
	{
		interface_config_no=8;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_10 && (TOUCH_INTERFACE_10+0x100)==data)
	{
		interface_config_no=9;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_11 && (TOUCH_INTERFACE_11+0x100)==data)
	{
		interface_config_no=10;
		cur_select_interface=g_share_memory->sensor_interface_mem[interface_config_no];
		show_cur_select_intr(cur_select_interface);
		switch_pic(INTERFACE_ALL_PAGE);
	}
	else if(addr==TOUCH_JIAOZHUN_P0 && (TOUCH_JIAOZHUN_P0+0x100)==data)
	{
		if(verify_point!=0)
		{
			verify_point=0;
			tun_zero(1);
			show_point(0,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_P1 && (TOUCH_JIAOZHUN_P1+0x100)==data)
	{
		if(verify_point!=1)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=1;
			show_point(1,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_P2 && (TOUCH_JIAOZHUN_P2+0x100)==data)
	{
		if(verify_point!=2)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=2;
			show_point(2,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_P3 && (TOUCH_JIAOZHUN_P3+0x100)==data)
	{
		if(verify_point!=3)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=3;
			show_point(3,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_P4 && (TOUCH_JIAOZHUN_P4+0x100)==data)
	{
		if(verify_point!=4)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=4;
			show_point(4,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_P5 && (TOUCH_JIAOZHUN_P5+0x100)==data)
	{
		if(verify_point!=5)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=5;
			show_point(5,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_P6 && (TOUCH_JIAOZHUN_P6+0x100)==data)
	{
		if(verify_point!=6)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=6;
			show_point(6,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_P7 && (TOUCH_JIAOZHUN_P7+0x100)==data)
	{
		if(verify_point!=7)
		{
			if(verify_point==0)
				tun_zero(0);	
			verify_point=7;
			show_point(7,verify_object);
		}
	}
	else if(addr==TOUCH_JIAOZHUN_UP && (TOUCH_JIAOZHUN_UP+0x100)==data)
	{
		//handle_xiuzhen(1,verify_object,verify_point);
	}
	else if(addr==TOUCH_JIAOZHUN_UP && (TOUCH_JIAOZHUN_UP+0x100)==data)
	{
		//handle_xiuzhen(0,verify_object,verify_point);
	}
	else if(addr==TOUCH_SET_RETURN && (TOUCH_SET_RETURN+0x100)==data)
	{
		if(interface_config_no!=0)
		{
			if((interface_config_no==1 && cur_select_interface==TYPE_SENSOR_PM25_WEISHEN)
				||(interface_config_no==8 && cur_select_interface==TYPE_SENSOR_WENSHI_RUSHI)
				||(interface_config_no==9 && cur_select_interface==TYPE_SENSOR_FENGSU)
				||(interface_config_no==10 && cur_select_interface==TYPE_SENSOR_QIYA_RUSHI)
				||(interface_config_no>1 && interface_config_no<8 
				&& cur_select_interface!=TYPE_SENSOR_QIYA_RUSHI
				&& cur_select_interface!=TYPE_SENSOR_WENSHI_RUSHI
				&& cur_select_interface!=TYPE_SENSOR_PM25_WEISHEN
				&& cur_select_interface!=TYPE_SENSOR_FENGSU))
			g_share_memory->sensor_interface_mem[interface_config_no]=cur_select_interface;
			printfLog(LCD_PROCESS"save sensor_interface_mem[%d]=%02x\n",interface_config_no,cur_select_interface);
			show_cur_interface(INTERFACE_SELECT_PAGE);
			//switch_pic(fd_lcd,SYSTEM_SET_PAGE);
		}
	}
	else if(addr==TOUCH_SENSOR_SET && (TOUCH_SENSOR_SET+0x100)==data)
	{//show history CO2 the first page
		if(logged)
		{			
			switch_pic(SENSOR_TEST_PAGE);
		}
		else
		{
			clear_buf(ADDR_USER_NAME_VERIFY,16);
			clear_buf(ADDR_USER_PWD_VERIFY,16);
			switch_pic(LOG_IN_PAGE);	
		}
		g_index=SYSTEM_SET_PAGE;
	}	
	else if(addr==TOUCH_INTERFACE_RETURN && (TOUCH_INTERFACE_RETURN+0x100)==data)
	{		
		//switch_pic(fd_lcd,SYSTEM_SET_PAGE);
	}
	else if(addr==TOUCH_INTERFACE_OK && (TOUCH_INTERFACE_OK+0x100)==data)
	{		
		set_interface();
		//switch_pic(fd_lcd,SYSTEM_SET_PAGE);
	}
	else if(addr==TOUCH_SET_HCHO_1 && (TOUCH_SET_HCHO_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CH2O_WEISHEN;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SET_HCHO_2 && (TOUCH_SET_HCHO_2+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CH2O_AERSHEN;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SET_WENSHI_1 && (TOUCH_SET_WENSHI_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_WENSHI_RUSHI;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SET_PM25_1 && (TOUCH_SET_PM25_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_PM25_WEISHEN;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SET_ZAOSHEN && (TOUCH_SET_ZAOSHEN+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_ZHAOSHEN;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SET_CO_1 && (TOUCH_SET_CO_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO_WEISHEN;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SET_CO_2 && (TOUCH_SET_CO_2+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO_DD;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SET_CO2_1 && (TOUCH_SET_CO2_1+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO2_WEISHEN;
		show_cur_select_intr(cur_select_interface);
	}	
	else if(addr==TOUCH_SET_CO2_2 && (TOUCH_SET_CO2_2+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_CO2_RUDIAN;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SET_FENGSU && (TOUCH_SET_FENGSU+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_FENGSU;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SET_QIYA && (TOUCH_SET_QIYA+0x100)==data)
	{
		cur_select_interface=TYPE_SENSOR_QIYA_RUSHI;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_UNKNOWN&& (TOUCH_UNKNOWN+0x100)==data)
	{
		cur_select_interface=0x00;
		show_cur_select_intr(cur_select_interface);
	}
	else if(addr==TOUCH_SYS_SET_RETURN&& (TOUCH_SYS_SET_RETURN+0x100)==data)
	{
		g_index=MAIN_PAGE;
	}
	else if(addr==TOUCH_PRODUCT_INFO && (TOUCH_PRODUCT_INFO+0x100)==data)
	{//product info
		clear_buf(ADDR_PRODUCT_NAME,40);
		clear_buf(ADDR_PRODUCT_MODE,40);
		clear_buf(ADDR_PRODUCT_ID,40);
		write_string(ADDR_PRODUCT_ID,g_share_memory->uuid,strlen(g_share_memory->uuid));
	}		
	else if(addr==TOUCH_CURVE_LIST&& (TOUCH_CURVE_LIST+0x100)==data)
	{//show detail in list
		switch (g_index)
		{
			case LIST_CO_PAGE:
			{//co
				switch_pic(LIST_CO_PAGE);
				show_history(ID_CAP_CO,begin_co);
				//begin_co=0;
			}
			break;
			case LIST_CO2_PAGE:
			{//co2
				switch_pic(LIST_CO2_PAGE);
				show_history(ID_CAP_CO2,begin_co2);
				//begin_co2=0;
			}
			break;
			case LIST_HCHO_PAGE:
			{//hcho
				switch_pic(LIST_HCHO_PAGE);
				show_history(ID_CAP_HCHO,begin_hcho);
				//begin_hcho=0;
			}
			break;
			case LIST_SHIDU_PAGE:
			{//shidu
				switch_pic(LIST_SHIDU_PAGE);
				show_history(ID_CAP_SHI_DU,begin_shidu);
				//begin_shidu=0;
			}
			break;
			case LIST_TEMP_PAGE:
			{//temp
				switch_pic(LIST_TEMP_PAGE);
				show_history(ID_CAP_TEMPERATURE,begin_temp);
				//begin_temp=0;
			}
			break;
			case LIST_PM25_PAGE:
			{//pm25
				switch_pic(LIST_PM25_PAGE);
				show_history(ID_CAP_PM_25,begin_pm25);
				//begin_pm25=0;
			}
			break;
			default:
				break;
		}
	}
	else if(addr==TOUCH_USER_RETURN_VERIFY&& (TOUCH_USER_RETURN_VERIFY+0x100)==data)
	{
		if(g_index!=SYSTEM_SET_PAGE)
			switch_pic(MAIN_PAGE);
		else
			switch_pic(SYSTEM_SET_PAGE);
	}
	else if(addr==TOUCH_USER_OK_VERIFY&& (TOUCH_USER_OK_VERIFY+0x100)==data)
	{//Login if didn't 
		log_in();
		printfLog(LCD_PROCESS"lcd=>history_done %d\n",g_share_memory->history_done);
		if(logged)
		{
			if(g_share_memory->history_done==0)
			switch (g_index)
			{
			case LIST_CO_PAGE:
			{//co
				switch_pic(LIST_CO_PAGE);
				show_history(ID_CAP_CO,begin_co);
				//begin_co=0;
			}
			break;
			case LIST_CO2_PAGE:
			{//co2
				switch_pic(LIST_CO2_PAGE);
				show_history(ID_CAP_CO2,begin_co2);
				//begin_co2=0;
			}
			break;
			case LIST_HCHO_PAGE:
			{//hcho
				switch_pic(LIST_HCHO_PAGE);
				show_history(ID_CAP_HCHO,begin_hcho);
				//begin_hcho=0;
			}
			break;
			case LIST_SHIDU_PAGE:
			{//shidu
				switch_pic(LIST_SHIDU_PAGE);
				show_history(ID_CAP_SHI_DU,begin_shidu);
				//begin_shidu=0;
			}
			break;
			case LIST_TEMP_PAGE:
			{//temp
				switch_pic(LIST_TEMP_PAGE);
				show_history(ID_CAP_TEMPERATURE,begin_temp);
				//begin_temp=0;
			}
			break;
			case LIST_PM25_PAGE:
			{//pm25
				switch_pic(LIST_PM25_PAGE);
				show_history(ID_CAP_PM_25,begin_pm25);
				//begin_pm25=0;
			}
			break;
			default:
				break;
			}
			else
			switch (g_index)
			{
				case LIST_CO_PAGE:
				{//co
					show_curve(ID_CAP_CO,&curve_co);
				}
				break;
				case LIST_CO2_PAGE:
				{//co2
					show_curve(ID_CAP_CO2,&curve_co2);
				}
				break;
				case LIST_HCHO_PAGE:
				{//hcho
					show_curve(ID_CAP_HCHO,&curve_hcho);
				}
				break;
				case LIST_SHIDU_PAGE:
				{//shidu
					show_curve(ID_CAP_SHI_DU,&curve_shidu);
				}
				break;
				case LIST_TEMP_PAGE:
				{//temp
					show_curve(ID_CAP_TEMPERATURE,&curve_temp);
				}
				break;
				case LIST_PM25_PAGE:
				{//pm25
					show_curve(ID_CAP_PM_25,&curve_pm25);
				}
				break;
				default:
					break;
			}
		}
	}
	return STATE_MAIN;
}

void lcd_loop()
{	
	char ch;
	int i=1;
	int get=0;
	char ptr[32]={0};	
	switch_pic(MAIN_PAGE);
	while(1)	
	{	
		if(read(g_share_memory->fd_lcd,&ch,1)==1)
		{
			//printf("<=%x \r\n",ch);
			switch(get)
			{
				case 0:
					if(ch==0x5a)
					{
						//printf(LCD_PROCESS"0x5a get ,get =1\r\n");
						get=1;
					}
					break;
				case 1:
					if(ch==0xa5)
					{
						//printf(LCD_PROCESS"0xa5 get ,get =2\r\n");
						get=2;

					}
					break;
				case 2:
					if(ch==0x06)
					{
						//printf(LCD_PROCESS"0x06 get,get =3\r\n");
						get=3;
						break;
					}
				case 3:
					if(ch==0x83)
					{
						//printf(LCD_PROCESS"0x83 get,get =4\r\n");
						get=4;
						i=1;
						break;
					}
				case 4:
					{
						//printf(LCD_PROCESS"%02x get ,get =5\r\n",ch);
						ptr[i++]=ch;
						if(i==6)
						{
							get=0;
							ptr[0]=0x01;
							printfLog(LCD_PROCESS"get %x %x %x %x %x %x\r\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);
							input_handle(ptr);
							printfLog(LCD_PROCESS"enter new loop\n");
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
	int fpid = 0;
	fpid=fork();
	if(fpid==0)
	{
		sensor_history.co	= (struct nano *)shmat(shmid_history_co,	 0, 0);
		sensor_history.co2	= (struct nano *)shmat(shmid_history_co2,	 0, 0);
		sensor_history.hcho = (struct nano *)shmat(shmid_history_hcho,	 0, 0);
		sensor_history.temp = (struct nano *)shmat(shmid_history_temp,	 0, 0);
		sensor_history.shidu= (struct nano *)shmat(shmid_history_shidu,  0, 0);
		sensor_history.pm25 = (struct nano *)shmat(shmid_history_pm25,	 0, 0);
		g_share_memory	= (struct share_memory *)shmat(shmid_share_memory,	 0, 0);
		g_share_memory->sensor_interface_mem[0] = 0x1234;
		signal(SIGALRM, lcd_off);
		alarm(300);
		while(1)
			lcd_loop();
	}
	else
		printfLog(LCD_PROCESS"[PID]%d lcd process\n",fpid);
	return 0;

}
