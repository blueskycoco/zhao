#ifndef _UART_H
#define _UART_H
#define SENSOR_NO	10
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
struct state{
	char network_state;
	char sensor_state[SENSOR_NO];
};
struct misc{	
	struct state misc_state;			//network state and sensor state
	char server_time[32];				//current time
};
struct history{
	struct 	*nano sensor_history[SENSOR_NO];	//cap sensor history data and time
	long 	*cnt[SENSOR_NO];				//cap sensor hisotry co/co2/ch2o/pm25/temp/shidu count
	key_t 	shmid_history[SENSOR_NO];
	key_t 	shmid_cnt[SENSOR_NO];
};
typedef struct _sensor_alarm {
	char alarm[SENSOR_NO];
	char sent[SENSOR_NO];
	char times[SENSOR_NO];
}sensor_alarm;

int cap_init();
#endif
