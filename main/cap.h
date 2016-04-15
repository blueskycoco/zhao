#ifndef _UART_H
#define _UART_H
#define SENSOR_NO		10
#define SENSOR_CO		0
#define SENSOR_CO2		1
#define SENSOR_HCHO		2
#define SENSOR_TEMP		3
#define SENSOR_SHIDU	4
#define SENSOR_PM25		5
struct nano{
	char time[20];
	char data[10];
};
struct history{
	struct nano co;
	struct nano co2;
	struct nano hcho;
	struct nano temp;
	struct nano shidu;
	struct nano pm25;
};
struct share_memory{
	long 	cnt[SENSOR_NO];			//cap sensor hisotry co/co2/ch2o/pm25/temp/shidu count	
	char 	history_done;			//history load done by history process
	char 	send_by_wifi;			//send by wifi	
	char 	network_state;			//network state,ok or failed
	char 	sensor_state[SENSOR_NO];//co/co2/ch20/pm25/temp/shidu state
	char 	server_time[32];		//current time	
	char 	alarm[SENSOR_NO];		//alarm state
	char 	sent[SENSOR_NO];		//had send state
	char 	times[SENSOR_NO];		//wrong times
	int 	sensor_interface_mem[11];
	char	factory_mode;
	int 	jiaozhun_sensor;	
	int 	cur_ch2o;
	int 	cur_co;	
	float 	p[8];
	float 	x[8];
	char 	y;
};
int cap_init();
#endif
