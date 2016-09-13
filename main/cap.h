#ifndef _UART_H
#define _UART_H
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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <iconv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <linux/rtc.h>
#include <sys/time.h>
//#include "cJSON.h"
//#include "misc.h"
//#include "history.h"
//#include "netlib.h"
//#include "xfer.h"
//#include "dwin.h"
#include "log.h"
#define VERSION	"V0.03-2016-09-13"
#define SENSOR_NO		12
#define SENSOR_CO		0
#define SENSOR_CO2		1
#define SENSOR_HCHO		2
#define SENSOR_TEMP		3
#define SENSOR_SHIDU	4
#define SENSOR_PM25		5
#define	SENSOR_PM10		6
#define	SENSOR_WIND		7
#define	SENSOR_NOISE	8
#define	SENSOR_PRESS	9
#define	SENSOR_TVOC		10
#define	SENSOR_O3		11
#define HISTORY_PATH 	"/home/user/history"
struct nano{
	char time[20];
	char data[10];
};
struct history{
	struct nano *co;
	struct nano *co2;
	struct nano *hcho;
	struct nano *temp;
	struct nano *shidu;
	struct nano *pm25;
	struct nano *pm10;
	struct nano *wind;
	struct nano *noise;
	struct nano *press;
	struct nano *tvoc;
	struct nano *o3;
};
struct share_memory{
	long 	cnt[SENSOR_NO];			//cap sensor hisotry co/co2/ch2o/pm25/temp/shidu count	
	char 	history_done;			//history load done by history process
	char 	send_by_wifi;			//send by wifi	
	char 	sleep;					//0 no sleep , 1 one mins, 5 5 mins, 10 10 mins
	char 	network_state;			//network state,ok or failed
	char 	sensor_state[SENSOR_NO];//co/co2/ch20/pm25/temp/shidu state
	char 	server_time[32];		//current time	
	char 	alarm[SENSOR_NO];		//alarm state
	char 	sent[SENSOR_NO];		//had send state
	char 	times[SENSOR_NO];		//wrong times
	char 	sensor_has_data[SENSOR_NO];//sensor[] is exist in slot ?
	int 	sensor_interface_mem[12];
	char	factory_mode;
	int 	jiaozhun_sensor;	
	int 	cur_ch2o;
	int 	cur_co;
	int 	cur_o3;
	int 	cur_no;	
	int 	cur_nh3;
	int 	cur_eto;
	int 	cur_no2;
	int 	cur_tvoc;	
	float 	p[8];
	float 	x[8];
	char 	y;
	int 	fd_lcd;
	int 	fd_com;
	int 	fd_gprs;	
	char 	ip[20];	
	char 	uuid[256];
	char 	ppm;
	float 	pj[SENSOR_NO][200];
	int		pj_cnt[SENSOR_NO];
	char	cap_board_ack;
	char	fan_state;
	char	audio_state;
	pthread_mutex_t mutex;
};
extern struct history sensor_history;
extern struct share_memory *g_share_memory;

extern key_t  shmid_share_memory;
extern key_t  shmid_history_co;
extern key_t  shmid_history_co2;
extern key_t  shmid_history_hcho;
extern key_t  shmid_history_temp;
extern key_t  shmid_history_shidu;
extern key_t  shmid_history_pm25;
extern key_t  shmid_history_wind;
extern key_t  shmid_history_noise;
extern key_t  shmid_history_o3;
extern key_t  shmid_history_press;
extern key_t  shmid_history_tvoc;
extern key_t  shmid_history_pm10;

#define MAIN_PAGE 				1
#define MAIN_PAGE_PPM			2
#define	CURVE_PAGE_HCHO			3
#define	CURVE_PAGE_PM10			4
#define	CURVE_PAGE_PM25			5
#define	CURVE_PAGE_CO			6
#define	CURVE_PAGE_CO2			7
#define	CURVE_PAGE_TEMP			8
#define	CURVE_PAGE_SHIDU		9
#define	CURVE_PAGE_WIND			10
#define	CURVE_PAGE_NOISE		11
#define	CURVE_PAGE_PRESS		12
#define	CURVE_PAGE_TVOC			13
#define	CURVE_PAGE_O3			15
#define LIST_PAGE_PM25			16
#define LIST_PAGE_CO			17
#define LIST_PAGE_CO2			18
#define LIST_PAGE_TEMP			19
#define LIST_PAGE_SHIDU			20
#define LIST_PAGE_WIND			21
#define LIST_PAGE_NOISE			22
#define LIST_PAGE_PRESS			23
#define LIST_PAGE_TVOC			24
#define LIST_PAGE_O3			25
#define LIST_PAGE_HCHO			26
#define LIST_PAGE_PM10			27
#define FACTORY_PAGE			28
#define PRODUCT_PAGE			29
#define SENSOR_SETTING_PAGE		30
#define GPRS_DONE_PAGE			31
#define WIFI_DONE_PAGE			32
#define STATE_PAGE_O_O			33
#define STATE_PAGE_I_O			34
#define STATE_PAGE_I_I			35
#define STATE_PAGE_O_I			36
#define TIME_PAGE_SETTING		39
#define VERIFY_PAGE				40
#define XFER_SETTING_PAGE		41
#define TIME_SETTING_PAGE		42
#define TUN_ZERO_PAGE			43
#define LOGIN_PAGE				44
#define SYSTEM_SETTING_PAGE		45
#define INTERFACE_PAGE			46
#define SENSOR_SEL_PAGE			47
#define USER_INFO_PAGE			49
#define OFF_PAGE				99
#define	ADDR_HCHO_REAL_1	0x0000
#define	ADDR_PM10_REAL_1	0x0001
#define	ADDR_PM25_REAL_1	0x0002
#define	ADDR_CO_REAL_1	0x0003
#define	ADDR_CO2_REAL_1	0x0004
#define	ADDR_TEMP_REAL_1	0x0005
#define	ADDR_SHIDU_REAL_1	0x0006
#define	ADDR_WIND_REAL_1	0x0007
#define	ADDR_NOISE_REAL_1	0x0008
#define	ADDR_PRESS_REAL_1	0x0009
#define	ADDR_TVOC_REAL_1	0x000a
#define	ADDR_O3_REAL_1	0x000b
#define	TOUCH_HCHO_REAL_1	0x000c
#define	TOUCH_PM10_REAL_1	0x000d
#define	TOUCH_PM25_REAL_1	0x000e
#define	TOUCH_CO_REAL_1	0x000f
#define	TOUCH_CO2_REAL_1	0x0010
#define	TOUCH_TEMP_REAL_1	0x0011
#define	TOUCH_SHIDU_REAL_1	0x0012
#define	TOUCH_WIND_REAL_1	0x0013
#define	TOUCH_NOISE_REAL_1	0x0014
#define	TOUCH_PRESS_REAL_1	0x0015
#define	TOUCH_TVOC_REAL_1	0x0016
#define	TOUCH_O3_REAL_1	0x0017
#define	TOUCH_DEVICE_STATE_1	0x0018
#define	TOUCH_PRODUCT_INFO_1	0x0019
#define	TOUCH_SYSTEM_SETTING_1	0x001a
#define	TOUCH_CONVERSION_1	0x001b
#define 	ADDR_CUT_VP_CO_1	0x0ba1
		
#define	ADDR_HCHO_REAL_2	0x001c
#define	ADDR_PM10_REAL_2	0x001d
#define	ADDR_PM25_REAL_2	0x001e
#define	ADDR_CO_REAL_2	0x001f
#define	ADDR_CO2_REAL_2	0x0020
#define	ADDR_TEMP_REAL_2	0x0021
#define	ADDR_SHIDU_REAL_2	0x0022
#define	ADDR_WIND_REAL_2	0x0023
#define	ADDR_NOISE_REAL_2	0x0024
#define	ADDR_PRESS_REAL_2	0x0025
#define	ADDR_TVOC_REAL_2	0x0026
#define	ADDR_O3_REAL_2	0x0027
#define	TOUCH_HCHO_REAL_2	0x0028
#define	TOUCH_PM10_REAL_2	0x0029
#define	TOUCH_PM25_REAL_2	0x002a
#define	TOUCH_CO_REAL_2	0x002b
#define	TOUCH_CO2_REAL_2	0x002c
#define	TOUCH_TEMP_REAL_2	0x002d
#define	TOUCH_SHIDU_REAL_2	0x002e
#define	TOUCH_WIND_REAL_2	0x002f
#define	TOUCH_NOISE_REAL_2	0x0030
#define	TOUCH_PRESS_REAL_2	0x0031
#define	TOUCH_TVOC_REAL_2	0x0032
#define	TOUCH_O3_REAL_2	0x0033
#define	TOUCH_DEVICE_STATE_2	0x0034
#define	TOUCH_PRODUCT_INFO_2	0x0035
#define	TOUCH_SYSTEM_SETTING_2	0x0036
#define	TOUCH_CONVERSION_2	0x0037
#define 	ADDR_CUT_VP_CO_2	0x0baa
		
#define	TOUCH_HCHO_HISTORY	0x0038
#define	TOUCH_HCHO_RETURN	0x0039
#define	ADDR_CURVE_HCHO_YEAR	0x0bfb
#define	ADDR_CURVE_HCHO_MON	0x0bfc
#define	ADDR_CURVE_HCHO_DAY	0x0bfd
		
#define	TOUCH_PM10_HISTORY	0x003a
#define	TOUCH_PM10_RETURN	0x003b
#define	ADDR_CURVE_PM10_YEAR	0x0d36
#define	ADDR_CURVE_PM10_MON	0x0d37
#define	ADDR_CURVE_PM10_DAY	0x0d38
		
#define	TOUCH_PM25_HISTORY	0x003c
#define	TOUCH_PM25_RETURN	0x003d
#define	ADDR_CURVE_PM25_YEAR	0x0d39
#define	ADDR_CURVE_PM25_MON	0x0d3a
#define	ADDR_CURVE_PM25_DAY	0x0d3b
		
#define	TOUCH_CO_HISTORY	0x003e
#define	TOUCH_CO_RETURN	0x003f
#define	ADDR_CURVE_CO_YEAR	0x0d3c
#define	ADDR_CURVE_CO_MON	0x0d3d
#define	ADDR_CURVE_CO_DAY	0x0d3e
		
#define	TOUCH_CO2_HISTORY	0x0040
#define	TOUCH_CO2_RETURN	0x0041
#define	ADDR_CURVE_CO2_YEAR	0x0d3f
#define	ADDR_CURVE_CO2_MON	0x0d40
#define	ADDR_CURVE_CO2_DAY	0x0d41
		
#define	TOUCH_TEMP_HISTORY	0x0042
#define	TOUCH_TEMP_RETURN	0x0043
#define	ADDR_CURVE_TEMP_YEAR	0x0d42
#define	ADDR_CURVE_TEMP_MON	0x0d43
#define	ADDR_CURVE_TEMP_DAY	0x0d44
		
#define	TOUCH_SHIDU_HISTORY	0x0044
#define	TOUCH_SHIDU_RETURN	0x0045
#define	ADDR_CURVE_SHIDU_YEAR	0x0d45
#define	ADDR_CURVE_SHIDU_MON	0x0d46
#define	ADDR_CURVE_SHIDU_DAY	0x0d47
		
#define	TOUCH_WIND_HISTORY	0x0046
#define	TOUCH_WIND_RETURN	0x0047
#define	ADDR_CURVE_WIND_YEAR	0x0d48
#define	ADDR_CURVE_WIND_MON	0x0d49
#define	ADDR_CURVE_WIND_DAY	0x0d4a
		
#define	TOUCH_NOISE_HISTORY	0x0048
#define	TOUCH_NOISE_RETURN	0x0049
#define	ADDR_CURVE_NOISE_YEAR	0x0d4b
#define	ADDR_CURVE_NOISE_MON	0x0d4c
#define	ADDR_CURVE_NOISE_DAY	0x0d4d
		
#define	TOUCH_PRESS_HISTORY	0x004a
#define	TOUCH_PRESS_RETURN	0x004b
#define	ADDR_CURVE_PRESS_YEAR	0x0d4e
#define	ADDR_CURVE_PRESS_MON	0x0d4f
#define	ADDR_CURVE_PRESS_DAY	0x0d50
		
#define	TOUCH_TVOC_HISTORY	0x004c
#define	TOUCH_TVOC_RETURN	0x004d
#define	ADDR_CURVE_TVOC_YEAR	0x0d51
#define	ADDR_CURVE_TVOC_MON	0x0d52
#define	ADDR_CURVE_TVOC_DAY	0x0d53
		
#define	TOUCH_O3_HISTORY	0x004e
#define	TOUCH_O3_RETURN	0x004f
#define	ADDR_CURVE_O3_YEAR	0x0d54
#define	ADDR_CURVE_O3_MON	0x0d55
#define	ADDR_CURVE_O3_DAY	0x0d56

		
#define	ADDR_PM25_TIME_0	0x0d05
#define	ADDR_PM25_DATA_0	0x0d13
#define	ADDR_PM25_TIME_1	0x0d06
#define	ADDR_PM25_DATA_1	0x0d14
#define	ADDR_PM25_TIME_2	0x0d07
#define	ADDR_PM25_DATA_2	0x0d15
#define	ADDR_PM25_TIME_3	0x0d08
#define	ADDR_PM25_DATA_3	0x0d16
#define	ADDR_PM25_TIME_4	0x0d09
#define	ADDR_PM25_DATA_4	0x0d17
#define	ADDR_PM25_TIME_5	0x0d0a
#define	ADDR_PM25_DATA_5	0x0d18
#define	ADDR_PM25_TIME_6	0x0d0b
#define	ADDR_PM25_DATA_6	0x0d19
#define	TOUCH_PM25_LAST_PAGE	0x0050
#define	TOUCH_PM25_NEXT_PAGE	0x0051
#define	TOUCH_PM25_UPDATE	0x0052
#define	TOUCH_PM25_HISTORY_RETURN	0x0053
#define	ADDR_PM25_PAGE_N	0x0d1a
#define 	ADDR_PM25_PAGE_ALL	0x0d1b
#define	ADDR_LIST_PM25_YEAR	0x0d02
#define	ADDR_LIST_PM25_MON	0x0d03
#define	ADDR_LIST_PM25_DAY	0x0d04
#define	ADDR_PM25_TIME_0_1	0x0d0c
#define	ADDR_PM25_TIME_1_1	0x0d0d
#define	ADDR_PM25_TIME_2_1	0x0d0e
#define	ADDR_PM25_TIME_3_1	0x0d0f
#define	ADDR_PM25_TIME_4_1	0x0d10
#define	ADDR_PM25_TIME_5_1	0x0d11
#define	ADDR_PM25_TIME_6_1	0x0d12
		
#define	ADDR_CO_TIME_0	0x0ceb
#define	ADDR_CO_DATA_0	0x0cf9
#define	ADDR_CO_TIME_1	0x0cec
#define	ADDR_CO_DATA_1	0x0cfa
#define	ADDR_CO_TIME_2	0x0ced
#define	ADDR_CO_DATA_2	0x0cfb
#define	ADDR_CO_TIME_3	0x0cee
#define	ADDR_CO_DATA_3	0x0cfc
#define	ADDR_CO_TIME_4	0x0cef
#define	ADDR_CO_DATA_4	0x0cfd
#define	ADDR_CO_TIME_5	0x0cf0
#define	ADDR_CO_DATA_5	0x0cfe
#define	ADDR_CO_TIME_6	0x0cf1
#define	ADDR_CO_DATA_6	0x0cff
#define	TOUCH_CO_LAST_PAGE	0x0054
#define	TOUCH_CO_NEXT_PAGE	0x0055
#define	TOUCH_CO_UPDATE	0x0056
#define	TOUCH_CO_HISTORY_RETURN	0x0057
#define	ADDR_CO_PAGE_N	0x0d00
#define 	ADDR_CO_PAGE_ALL	0x0d01
#define	ADDR_LIST_CO_YEAR	0x0ce8
#define	ADDR_LIST_CO_MON	0x0ce9
#define	ADDR_LIST_CO_DAY	0x0cea
#define	ADDR_CO_TIME_0_1	0x0cf2
#define	ADDR_CO_TIME_1_1	0x0cf3
#define	ADDR_CO_TIME_2_1	0x0cf4
#define	ADDR_CO_TIME_3_1	0x0cf5
#define	ADDR_CO_TIME_4_1	0x0cf6
#define	ADDR_CO_TIME_5_1	0x0cf7
#define	ADDR_CO_TIME_6_1	0x0cf8
		
#define	ADDR_CO2_TIME_0	0x0cd1
#define	ADDR_CO2_DATA_0	0x0cdf
#define	ADDR_CO2_TIME_1	0x0cd2
#define	ADDR_CO2_DATA_1	0x0ce0
#define	ADDR_CO2_TIME_2	0x0cd3
#define	ADDR_CO2_DATA_2	0x0ce1
#define	ADDR_CO2_TIME_3	0x0cd4
#define	ADDR_CO2_DATA_3	0x0ce2
#define	ADDR_CO2_TIME_4	0x0cd5
#define	ADDR_CO2_DATA_4	0x0ce3
#define	ADDR_CO2_TIME_5	0x0cd6
#define	ADDR_CO2_DATA_5	0x0ce4
#define	ADDR_CO2_TIME_6	0x0cd7
#define	ADDR_CO2_DATA_6	0x0ce5
#define	TOUCH_CO2_LAST_PAGE	0x0058
#define	TOUCH_CO2_NEXT_PAGE	0x0059
#define	TOUCH_CO2_UPDATE	0x005a
#define	TOUCH_CO2_HISTORY_RETURN	0x005b
#define	ADDR_CO2_PAGE_N	0x0ce6
#define 	ADDR_CO2_PAGE_ALL	0x0ce7
#define	ADDR_LIST_CO2_YEAR	0x0cce
#define	ADDR_LIST_CO2_MON	0x0ccf
#define	ADDR_LIST_CO2_DAY	0x0cd0
#define	ADDR_CO2_TIME_0_1	0x0cd8
#define	ADDR_CO2_TIME_1_1	0x0cd9
#define	ADDR_CO2_TIME_2_1	0x0cda
#define	ADDR_CO2_TIME_3_1	0x0cdb
#define	ADDR_CO2_TIME_4_1	0x0cdc
#define	ADDR_CO2_TIME_5_1	0x0cdd
#define	ADDR_CO2_TIME_6_1	0x0cde
		
#define	ADDR_TEMP_TIME_0	0x0cb7
#define	ADDR_TEMP_DATA_0	0x0cc5
#define	ADDR_TEMP_TIME_1	0x0cb8
#define	ADDR_TEMP_DATA_1	0x0cc6
#define	ADDR_TEMP_TIME_2	0x0cb9
#define	ADDR_TEMP_DATA_2	0x0cc7
#define	ADDR_TEMP_TIME_3	0x0cba
#define	ADDR_TEMP_DATA_3	0x0cc8
#define	ADDR_TEMP_TIME_4	0x0cbb
#define	ADDR_TEMP_DATA_4	0x0cc9
#define	ADDR_TEMP_TIME_5	0x0cbc
#define	ADDR_TEMP_DATA_5	0x0cca
#define	ADDR_TEMP_TIME_6	0x0cbd
#define	ADDR_TEMP_DATA_6	0x0ccb
#define	TOUCH_TEMP_LAST_PAGE	0x005c
#define	TOUCH_TEMP_NEXT_PAGE	0x005d
#define	TOUCH_TEMP_UPDATE	0x005e
#define	TOUCH_TEMP_HISTORY_RETURN	0x005f
#define	ADDR_TEMP_PAGE_N	0x0ccc
#define 	ADDR_TEMP_PAGE_ALL	0x0ccd
#define	ADDR_LIST_TEMP_YEAR	0x0cb4
#define	ADDR_LIST_TEMP_MON	0x0cb5
#define	ADDR_LIST_TEMP_DAY	0x0cb6
#define	ADDR_TEMP_TIME_0_1	0x0cbe
#define	ADDR_TEMP_TIME_1_1	0x0cbf
#define	ADDR_TEMP_TIME_2_1	0x0cc0
#define	ADDR_TEMP_TIME_3_1	0x0cc1
#define	ADDR_TEMP_TIME_4_1	0x0cc2
#define	ADDR_TEMP_TIME_5_1	0x0cc3
#define	ADDR_TEMP_TIME_6_1	0x0cc4
		
#define	ADDR_SHIDU_TIME_0	0x0c9d
#define	ADDR_SHIDU_DATA_0	0x0cab
#define	ADDR_SHIDU_TIME_1	0x0c9e
#define	ADDR_SHIDU_DATA_1	0x0cac
#define	ADDR_SHIDU_TIME_2	0x0c9f
#define	ADDR_SHIDU_DATA_2	0x0cad
#define	ADDR_SHIDU_TIME_3	0x0ca0
#define	ADDR_SHIDU_DATA_3	0x0cae
#define	ADDR_SHIDU_TIME_4	0x0ca1
#define	ADDR_SHIDU_DATA_4	0x0caf
#define	ADDR_SHIDU_TIME_5	0x0ca2
#define	ADDR_SHIDU_DATA_5	0x0cb0
#define	ADDR_SHIDU_TIME_6	0x0ca3
#define	ADDR_SHIDU_DATA_6	0x0cb1
#define	TOUCH_SHIDU_LAST_PAGE	0x0060
#define	TOUCH_SHIDU_NEXT_PAGE	0x0061
#define	TOUCH_SHIDU_UPDATE	0x0062
#define	TOUCH_SHIDU_HISTORY_RETURN	0x0063
#define	ADDR_SHIDU_PAGE_N	0x0cb2
#define 	ADDR_SHIDU_PAGE_ALL	0x0cb3
#define	ADDR_LIST_SHIDU_YEAR	0x0c9a
#define	ADDR_LIST_SHIDU_MON	0x0c9b
#define	ADDR_LIST_SHIDU_DAY	0x0c9c
#define	ADDR_SHIDU_TIME_0_1	0x0ca4
#define	ADDR_SHIDU_TIME_1_1	0x0ca5
#define	ADDR_SHIDU_TIME_2_1	0x0ca6
#define	ADDR_SHIDU_TIME_3_1	0x0ca7
#define	ADDR_SHIDU_TIME_4_1	0x0ca8
#define	ADDR_SHIDU_TIME_5_1	0x0ca9
#define	ADDR_SHIDU_TIME_6_1	0x0caa
		
#define	ADDR_WIND_TIME_0	0x0c83
#define	ADDR_WIND_DATA_0	0x0c91
#define	ADDR_WIND_TIME_1	0x0c84
#define	ADDR_WIND_DATA_1	0x0c92
#define	ADDR_WIND_TIME_2	0x0c85
#define	ADDR_WIND_DATA_2	0x0c93
#define	ADDR_WIND_TIME_3	0x0c86
#define	ADDR_WIND_DATA_3	0x0c94
#define	ADDR_WIND_TIME_4	0x0c87
#define	ADDR_WIND_DATA_4	0x0c95
#define	ADDR_WIND_TIME_5	0x0c88
#define	ADDR_WIND_DATA_5	0x0c96
#define	ADDR_WIND_TIME_6	0x0c89
#define	ADDR_WIND_DATA_6	0x0c97
#define	TOUCH_WIND_LAST_PAGE	0x0064
#define	TOUCH_WIND_NEXT_PAGE	0x0065
#define	TOUCH_WIND_UPDATE	0x0066
#define	TOUCH_WIND_HISTORY_RETURN	0x0067
#define	ADDR_WIND_PAGE_N	0x0c98
#define 	ADDR_WIND_PAGE_ALL	0x0c99
#define	ADDR_LIST_WIND_YEAR	0x0c80
#define	ADDR_LIST_WIND_MON	0x0c81
#define	ADDR_LIST_WIND_DAY	0x0c82
#define	ADDR_WIND_TIME_0_1	0x0c8a
#define	ADDR_WIND_TIME_1_1	0x0c8b
#define	ADDR_WIND_TIME_2_1	0x0c8c
#define	ADDR_WIND_TIME_3_1	0x0c8d
#define	ADDR_WIND_TIME_4_1	0x0c8e
#define	ADDR_WIND_TIME_5_1	0x0c8f
#define	ADDR_WIND_TIME_6_1	0x0c90
		
#define	ADDR_NOISE_TIME_0	0x0c69
#define	ADDR_NOISE_DATA_0	0x0c77
#define	ADDR_NOISE_TIME_1	0x0c6a
#define	ADDR_NOISE_DATA_1	0x0c78
#define	ADDR_NOISE_TIME_2	0x0c6b
#define	ADDR_NOISE_DATA_2	0x0c79
#define	ADDR_NOISE_TIME_3	0x0c6c
#define	ADDR_NOISE_DATA_3	0x0c7a
#define	ADDR_NOISE_TIME_4	0x0c6d
#define	ADDR_NOISE_DATA_4	0x0c7b
#define	ADDR_NOISE_TIME_5	0x0c6e
#define	ADDR_NOISE_DATA_5	0x0c7c
#define	ADDR_NOISE_TIME_6	0x0c6f
#define	ADDR_NOISE_DATA_6	0x0c7d
#define	TOUCH_NOISE_LAST_PAGE	0x0068
#define	TOUCH_NOISE_NEXT_PAGE	0x0069
#define	TOUCH_NOISE_UPDATE	0x006a
#define	TOUCH_NOISE_HISTORY_RETURN	0x006b
#define	ADDR_NOISE_PAGE_N	0x0c7e
#define 	ADDR_NOISE_PAGE_ALL	0x0c7f
#define	ADDR_LIST_NOISE_YEAR	0x0c66
#define	ADDR_LIST_NOISE_MON	0x0c67
#define	ADDR_LIST_NOISE_DAY	0x0c68
#define	ADDR_NOISE_TIME_0_1	0x0c70
#define	ADDR_NOISE_TIME_1_1	0x0c71
#define	ADDR_NOISE_TIME_2_1	0x0c72
#define	ADDR_NOISE_TIME_3_1	0x0c73
#define	ADDR_NOISE_TIME_4_1	0x0c74
#define	ADDR_NOISE_TIME_5_1	0x0c75
#define	ADDR_NOISE_TIME_6_1	0x0c76
		
#define	ADDR_PRESS_TIME_0	0x0c4f
#define	ADDR_PRESS_DATA_0	0x0c5d
#define	ADDR_PRESS_TIME_1	0x0c50
#define	ADDR_PRESS_DATA_1	0x0c5e
#define	ADDR_PRESS_TIME_2	0x0c51
#define	ADDR_PRESS_DATA_2	0x0c5f
#define	ADDR_PRESS_TIME_3	0x0c52
#define	ADDR_PRESS_DATA_3	0x0c60
#define	ADDR_PRESS_TIME_4	0x0c53
#define	ADDR_PRESS_DATA_4	0x0c61
#define	ADDR_PRESS_TIME_5	0x0c54
#define	ADDR_PRESS_DATA_5	0x0c62
#define	ADDR_PRESS_TIME_6	0x0c55
#define	ADDR_PRESS_DATA_6	0x0c63
#define	TOUCH_PRESS_LAST_PAGE	0x006c
#define	TOUCH_PRESS_NEXT_PAGE	0x006d
#define	TOUCH_PRESS_UPDATE	0x006e
#define	TOUCH_PRESS_HISTORY_RETURN	0x006f
#define	ADDR_PRESS_PAGE_N	0x0c64
#define 	ADDR_PRESS_PAGE_ALL	0x0c65
#define	ADDR_LIST_PRESS_YEAR	0x0c4c
#define	ADDR_LIST_PRESS_MON	0x0c4d
#define	ADDR_LIST_PRESS_DAY	0x0c4e
#define	ADDR_PRESS_TIME_0_1	0x0c56
#define	ADDR_PRESS_TIME_1_1	0x0c57
#define	ADDR_PRESS_TIME_2_1	0x0c58
#define	ADDR_PRESS_TIME_3_1	0x0c59
#define	ADDR_PRESS_TIME_4_1	0x0c5a
#define	ADDR_PRESS_TIME_5_1	0x0c5b
#define	ADDR_PRESS_TIME_6_1	0x0c5c
		
#define	ADDR_TVOC_TIME_0	0x0c35
#define	ADDR_TVOC_DATA_0	0x0c43
#define	ADDR_TVOC_TIME_1	0x0c36
#define	ADDR_TVOC_DATA_1	0x0c44
#define	ADDR_TVOC_TIME_2	0x0c37
#define	ADDR_TVOC_DATA_2	0x0c45
#define	ADDR_TVOC_TIME_3	0x0c38
#define	ADDR_TVOC_DATA_3	0x0c46
#define	ADDR_TVOC_TIME_4	0x0c39
#define	ADDR_TVOC_DATA_4	0x0c47
#define	ADDR_TVOC_TIME_5	0x0c3a
#define	ADDR_TVOC_DATA_5	0x0c48
#define	ADDR_TVOC_TIME_6	0x0c3b
#define	ADDR_TVOC_DATA_6	0x0c49
#define	TOUCH_TVOC_LAST_PAGE	0x0070
#define	TOUCH_TVOC_NEXT_PAGE	0x0071
#define	TOUCH_TVOC_UPDATE	0x0072
#define	TOUCH_TVOC_HISTORY_RETURN	0x0073
#define	ADDR_TVOC_PAGE_N	0x0c4a
#define 	ADDR_TVOC_PAGE_ALL	0x0c4b
#define	ADDR_LIST_TVOC_YEAR	0x0c32
#define	ADDR_LIST_TVOC_MON	0x0c33
#define	ADDR_LIST_TVOC_DAY	0x0c34
#define	ADDR_TVOC_TIME_0_1	0x0c3c
#define	ADDR_TVOC_TIME_1_1	0x0c3d
#define	ADDR_TVOC_TIME_2_1	0x0c3e
#define	ADDR_TVOC_TIME_3_1	0x0c3f
#define	ADDR_TVOC_TIME_4_1	0x0c40
#define	ADDR_TVOC_TIME_5_1	0x0c41
#define	ADDR_TVOC_TIME_6_1	0x0c42
		
#define	ADDR_O3_TIME_0	0x0c1b
#define	ADDR_O3_DATA_0	0x0c29
#define	ADDR_O3_TIME_1	0x0c1c
#define	ADDR_O3_DATA_1	0x0c2a
#define	ADDR_O3_TIME_2	0x0c1d
#define	ADDR_O3_DATA_2	0x0c2b
#define	ADDR_O3_TIME_3	0x0c1e
#define	ADDR_O3_DATA_3	0x0c2c
#define	ADDR_O3_TIME_4	0x0c1f
#define	ADDR_O3_DATA_4	0x0c2d
#define	ADDR_O3_TIME_5	0x0c20
#define	ADDR_O3_DATA_5	0x0c2e
#define	ADDR_O3_TIME_6	0x0c21
#define	ADDR_O3_DATA_6	0x0c2f
#define	TOUCH_O3_LAST_PAGE	0x0074
#define	TOUCH_O3_NEXT_PAGE	0x0075
#define	TOUCH_O3_UPDATE	0x0076
#define	TOUCH_O3_HISTORY_RETURN	0x0077
#define	ADDR_O3_PAGE_N	0x0c30
#define 	ADDR_O3_PAGE_ALL	0x0c31
#define	ADDR_LIST_O3_YEAR	0x0c18
#define	ADDR_LIST_O3_MON	0x0c19
#define	ADDR_LIST_O3_DAY	0x0c1a
#define	ADDR_O3_TIME_0_1	0x0c22
#define	ADDR_O3_TIME_1_1	0x0c23
#define	ADDR_O3_TIME_2_1	0x0c24
#define	ADDR_O3_TIME_3_1	0x0c25
#define	ADDR_O3_TIME_4_1	0x0c26
#define	ADDR_O3_TIME_5_1	0x0c27
#define	ADDR_O3_TIME_6_1	0x0c28
		
#define	ADDR_HCHO_TIME_0	0x0c01
#define	ADDR_HCHO_DATA_0	0x0c02
#define	ADDR_HCHO_TIME_1	0x0c03
#define	ADDR_HCHO_DATA_1	0x0c04
#define	ADDR_HCHO_TIME_2	0x0c05
#define	ADDR_HCHO_DATA_2	0x0c06
#define	ADDR_HCHO_TIME_3	0x0c07
#define	ADDR_HCHO_DATA_3	0x0c08
#define	ADDR_HCHO_TIME_4	0x0c09
#define	ADDR_HCHO_DATA_4	0x0c0a
#define	ADDR_HCHO_TIME_5	0x0c0b
#define	ADDR_HCHO_DATA_5	0x0c0c
#define	ADDR_HCHO_TIME_6	0x0c0d
#define	ADDR_HCHO_DATA_6	0x0c0e
#define	TOUCH_HCHO_LAST_PAGE	0x0078
#define	TOUCH_HCHO_NEXT_PAGE	0x0079
#define	TOUCH_HCHO_UPDATE	0x007a
#define	TOUCH_HCHO_HISTORY_RETURN	0x007b
#define	ADDR_HCHO_PAGE_N	0x0c0f
#define 	ADDR_HCHO_PAGE_ALL	0x0c10
#define	ADDR_LIST_HCHO_YEAR	0x0bfe
#define	ADDR_LIST_HCHO_MON	0x0bff
#define	ADDR_LIST_HCHO_DAY	0x0c00
#define	ADDR_HCHO_TIME_0_1	0x0c11
#define	ADDR_HCHO_TIME_1_1	0x0c12
#define	ADDR_HCHO_TIME_2_1	0x0c13
#define	ADDR_HCHO_TIME_3_1	0x0c14
#define	ADDR_HCHO_TIME_4_1	0x0c15
#define	ADDR_HCHO_TIME_5_1	0x0c16
#define	ADDR_HCHO_TIME_6_1	0x0c17
		
#define	ADDR_PM10_TIME_0	0x0d1f
#define	ADDR_PM10_DATA_0	0x0d2d
#define	ADDR_PM10_TIME_1	0x0d20
#define	ADDR_PM10_DATA_1	0x0d2e
#define	ADDR_PM10_TIME_2	0x0d21
#define	ADDR_PM10_DATA_2	0x0d2f
#define	ADDR_PM10_TIME_3	0x0d22
#define	ADDR_PM10_DATA_3	0x0d30
#define	ADDR_PM10_TIME_4	0x0d23
#define	ADDR_PM10_DATA_4	0x0d31
#define	ADDR_PM10_TIME_5	0x0d24
#define	ADDR_PM10_DATA_5	0x0d32
#define	ADDR_PM10_TIME_6	0x0d25
#define	ADDR_PM10_DATA_6	0x0d33
#define	TOUCH_PM10_LAST_PAGE	0x007c
#define	TOUCH_PM10_NEXT_PAGE	0x007d
#define	TOUCH_PM10_UPDATE	0x007e
#define	TOUCH_PM10_HISTORY_RETURN	0x007f
#define	ADDR_PM10_PAGE_N	0x0d34
#define 	ADDR_PM10_PAGE_ALL	0x0d35
#define	ADDR_LIST_PM10_YEAR	0x0d1c
#define	ADDR_LIST_PM10_MON	0x0d1d
#define	ADDR_LIST_PM10_DAY	0x0d1e
#define	ADDR_PM10_TIME_0_1	0x0d26
#define	ADDR_PM10_TIME_1_1	0x0d27
#define	ADDR_PM10_TIME_2_1	0x0d28
#define	ADDR_PM10_TIME_3_1	0x0d29
#define	ADDR_PM10_TIME_4_1	0x0d2a
#define	ADDR_PM10_TIME_5_1	0x0d2b
#define	ADDR_PM10_TIME_6_1	0x0d2c
		
#define	TOUCH_FACTORY_INFO_RETURN	0x0080
		
#define	TOUCH_PRODUCT_INFO_RETURN	0x0081
#define	ADDR_PRODUCT_NAME	0x086b
#define	ADDR_PRODUCT_MODEL	0x088b
#define	ADDR_PRODUCT_ID	0x08ab
		
#define	TOUCH_TUN_ZERO	0x0082
#define	TOUCH_VERIFY	0x0083
#define	TOUCH_SEL_INTERFACE	0x0084
#define	TOUCH_SETTING_RETURN	0x0085
		
#define	TOUCH_GPRS_OK	0x0086
		
#define	TOUCH_WIFI_OK	0x0087
		
#define	TOUCH_TEST_GPRS_1	0x0088
#define	TOUCH_STATE_RETURN_1	0x0089
#define 	ADDR_CUT_VP_GPRS_1_1	0x0bb3
#define 	ADDR_CUT_VP_GPRS_1_2	0x0bbc
		
#define	TOUCH_TEST_GPRS_2	0x008a
#define	TOUCH_STATE_RETURN_2	0x008b
#define 	ADDR_CUT_VP_GPRS_2_1	0x0bc5
#define 	ADDR_CUT_VP_GPRS_2_2	0x0bce
		
#define	TOUCH_TEST_GPRS_3	0x008c
#define	TOUCH_STATE_RETURN_3	0x008d
#define 	ADDR_CUT_VP_GPRS_3_1	0x0bd7
#define 	ADDR_CUT_VP_GPRS_3_2	0x0be0
		
#define	TOUCH_TEST_GPRS_4	0x008e
#define	TOUCH_STATE_RETURN_4	0x008f
#define 	ADDR_CUT_VP_GPRS_4_1	0x0be9
#define 	ADDR_CUT_VP_GPRS_4_2	0x0bf2
		
#define	TOUCH_ONE_MINS	0x0090
#define	TOUCH_FIVE_MINS	0x0091
#define	TOUCH_TEN_MINS	0x0092
#define	TOUCH_NEVER_MINS	0x0093
		
#define	TOUCH_EXIT_VERIFY	0x0094
#define	TOUCH_SEL_VP_0	0x0095
#define	TOUCH_SEL_VP_1	0x0096
#define	TOUCH_SEL_VP_2	0x0097
#define	TOUCH_SEL_VP_3	0x0098
#define	TOUCH_SEL_VP_4	0x0099
#define	TOUCH_SEL_VP_5	0x009a
#define	TOUCH_SEL_VP_6	0x009b
#define	TOUCH_SEL_VP_7	0x009c
#define	ADDR_VP_0	0x08b5
#define	ADDR_VP_1	0x08bf
#define	ADDR_VP_2	0x08c9
#define	ADDR_VP_3	0x08d3
#define	ADDR_VP_4	0x08dd
#define	ADDR_VP_5	0x08e7
#define	ADDR_VP_6	0x08f1
#define	ADDR_VP_7	0x08fb
#define	ADDR_XIUZHENG	0x0905
#define	ADDR_JIAOZHUN_REAL	0x090f
#define 	ADDR_CUT_VP_0	0x0b59
#define 	ADDR_CUT_VP_1	0x0b62
#define 	ADDR_CUT_VP_2	0x0b6b
#define 	ADDR_CUT_VP_3	0x0b74
#define 	ADDR_CUT_VP_4	0x0b7d
#define 	ADDR_CUT_VP_5	0x0b86
#define 	ADDR_CUT_VP_6	0x0b8f
#define 	ADDR_CUT_VP_7	0x0b98
		
#define	TOUCH_SEL_GPRS	0x009d
#define	TOUCH_SEL_WIFI	0x009e
#define	TOUCH_XFER_OK	0x009f
#define	TOUCH_XFER_RETURN	0x00a0
#define	ADDR_XFER_SELECT	0x0919
#define	ADDR_WIFI_STATUS	0x0921
#define	ADDR_AP_NAME	0x0941
#define	ADDR_AP_KEY	0x0961
		
#define	ADDR_YEAR	0x0981
#define	ADDR_MON	0x0985
#define	ADDR_DAY	0x0987
#define	ADDR_HOUR	0x0989
#define	ADDR_MIN	0x098b
#define	ADDR_SEC	0x098d
#define	TOUCH_SYNC_SERVER	0x00a7
#define	TOUCH_TIME_OK	0x00a8
#define	TOUCH_TIME_RETURN	0x00a9
		
#define	TOUCH_TUN_ZERO_START	0x00aa
#define	TOUCH_TUN_ZERO_END	0x00ab
#define	TOUCH_TUN_ZERO_RETURN	0x00ac
#define	ADDR_TUN_ZERO_CO	0x098f
#define	ADDR_TUN_ZERO_HCHO	0x0995
#define	ADDR_TUN_ZERO_O3	0x0d57
#define	ADDR_TUN_ZERO_NO	0x0d5d
#define	ADDR_TUN_ZERO_NH3	0x0d63
#define	ADDR_TUN_ZERO_ETO	0x0d69
#define	ADDR_TUN_ZERO_NO2	0x0d6f
#define	ADDR_TUN_ZERO_TVOC	0x0d75
		
#define	TOUCH_LOGIN_OK	0x00ad
#define	TOUCH_LOGIN_RETURN	0x00ae
#define	ADDR_LOGIN_USER_NAME	0x099b
#define	ADDR_LOGIN_USER_KEY	0x09bb
		
#define	TOUCH_SENSOR_SETTING	0x00af
#define	TOUCH_XFER_SETTING	0x00b0
#define	TOUCH_TIME_SETTING	0x00b1
#define	TOUCH_USER_INFO	0x00b2
#define	TOUCH_FACTORY_INFO	0x00b3
#define	TOUCH_SLEEP_SETTING	0x00b4
#define	TOUCH_SYSTEM_SETTING_RETURN	0x00b5
		
#define	TOUCH_INTERFACE_0	0x00b6
#define	TOUCH_INTERFACE_1	0x00b7
#define	TOUCH_INTERFACE_2	0x00b8
#define	TOUCH_INTERFACE_3	0x00b9
#define	TOUCH_INTERFACE_4	0x00ba
#define	TOUCH_INTERFACE_5	0x00bb
#define	TOUCH_INTERFACE_6	0x00bc
#define	TOUCH_INTERFACE_7	0x00bd
#define	TOUCH_INTERFACE_8	0x00be
#define	TOUCH_INTERFACE_9	0x00bf
#define	TOUCH_INTERFACE_10	0x00c0
#define	TOUCH_INTERFACE_11	0x00c1
#define	TOUCH_INTERFACE_OK	0x00c2
#define	TOUCH_INTERFACE_RETURN1	0x00c3
#define	ADDR_INTERFACE_0	0x09db
#define	ADDR_INTERFACE_1	0x09eb
#define	ADDR_INTERFACE_2	0x09fb
#define	ADDR_INTERFACE_3	0x0a0b
#define	ADDR_INTERFACE_4	0x0a1b
#define	ADDR_INTERFACE_5	0x0a2b
#define	ADDR_INTERFACE_6	0x0a3b
#define	ADDR_INTERFACE_7	0x0a4b
#define	ADDR_INTERFACE_8	0x0a5b
#define	ADDR_INTERFACE_9	0x0a6b
#define	ADDR_INTERFACE_10	0x0a7b
#define	ADDR_INTERFACE_11	0x0a8b
		
#define	TOUCH_SEL_HCHO_1	0x00c4
#define	TOUCH_SEL_HCHO_2	0x00c5
#define	TOUCH_SEL_CO_1	0x00c6
#define	TOUCH_SEL_CO_2	0x00c7
#define	TOUCH_SEL_CO2_1	0x00c8
#define	TOUCH_SEL_CO2_2	0x00c9
#define	TOUCH_SEL_WENSHI_1	0x00ca
#define	TOUCH_SEL_PRESS_1	0x00cb
#define	TOUCH_SEL_PM25_1	0x00cc
#define	TOUCH_SEL_NOISE_1	0x00cd
#define	TOUCH_SEL_WIND_1	0x00ce
#define	TOUCH_SEL_UNKNOWN_1	0x00cf
#define	TOUCH_INTERFACE_RETURN2	0x00d0
#define	ADDR_CURRENT_SELECT	0x0a9b
		
#define	TOUCH_USER_INFO_RETURN	0x00d1
#define	ADDR_INFO_USER_NAME	0x0aab
#define	ADDR_INFO_INSTALL_PLACE	0x0ad5
#define	ADDR_INFO_HANGYE	0x0aff
#define	ADDR_INFO_ADDR	0x0b0f
#define	ADDR_INFO_PHONE	0x0b39
#define	ADDR_INFO_CONTACTER	0x0b49
		
#define	TOUCH_WAKE_UP	0x00d2
#define ADDR_SW_VERSION	0x0d7d
#define ADDR_O3_SHOW_PIC			0x0d98
#define ADDR_TVOC_SHOW_PIC			0x0d97
#define ADDR_PRESS_SHOW_PIC			0x0d96
#define ADDR_NOISE_SHOW_PIC			0x0d95
#define ADDR_WIND_SHOW_PIC			0x0d94
#define ADDR_SHIDU_SHOW_PIC			0x0d93
#define ADDR_TEMP_SHOW_PIC			0x0d92
#define ADDR_CO2_SHOW_PIC			0x0d91
#define ADDR_CO_SHOW_PIC			0x0d90
#define ADDR_PM25_SHOW_PIC			0x0d8f
#define ADDR_PM10_SHOW_PIC			0x0d8e
#define ADDR_HCHO_SHOW_PIC			0x0d8d
#define ADDR_O3_SHOW_PIC_PPM		0x0da4
#define ADDR_TVOC_SHOW_PIC_PPM		0x0da3
#define ADDR_PRESS_SHOW_PIC_PPM		0x0da2
#define ADDR_NOISE_SHOW_PIC_PPM		0x0da1
#define ADDR_WIND_SHOW_PIC_PPM		0x0da0
#define ADDR_SHIDU_SHOW_PIC_PPM		0x0d9f
#define ADDR_TEMP_SHOW_PIC_PPM		0x0d9e
#define ADDR_CO2_SHOW_PIC_PPM		0x0d9d
#define ADDR_CO_SHOW_PIC_PPM		0x0d9c
#define ADDR_PM25_SHOW_PIC_PPM		0x0d9b
#define ADDR_PM10_SHOW_PIC_PPM		0x0d9a
#define ADDR_HCHO_SHOW_PIC_PPM		0x0d99

#define ADDR_GRAPHIC_FAN_1			0x0da9
#define ADDR_GRAPHIC_AUDIO_1		0x0db2
#define ADDR_GRAPHIC_FAN_2			0x0dbb
#define ADDR_GRAPHIC_AUDIO_2		0x0dc4
#define TOUCH_FAN_1					0x0da5
#define TOUCH_AUDIO_1				0x0da6
#define TOUCH_FAN_2					0x0da7
#define TOUCH_AUDIO_2				0x0da8

#define ADDR_HISTORY_TEMP_1_1		0x0dcd
#define ADDR_HISTORY_TEMP_2_1		0x0dce
#define ADDR_HISTORY_TEMP_3_1		0x0dcf
#define ADDR_HISTORY_TEMP_4_1		0x0dd0
#define ADDR_HISTORY_TEMP_5_1		0x0dd1
#define ADDR_HISTORY_TEMP_6_1		0x0dd2
#define ADDR_HISTORY_TEMP_1_2		0x0dd3
#define ADDR_HISTORY_TEMP_2_2		0x0dd4
#define ADDR_HISTORY_TEMP_3_2		0x0dd5
#define ADDR_HISTORY_TEMP_4_2		0x0dd6
#define ADDR_HISTORY_TEMP_5_2		0x0dd7
#define ADDR_HISTORY_TEMP_6_2		0x0dd8

#define ADDR_HISTORY_SHIDU_1_1		0x0dfd
#define ADDR_HISTORY_SHIDU_2_1		0x0dfe
#define ADDR_HISTORY_SHIDU_3_1		0x0dff
#define ADDR_HISTORY_SHIDU_4_1		0x0e00
#define ADDR_HISTORY_SHIDU_5_1		0x0e01
#define ADDR_HISTORY_SHIDU_6_1		0x0e02
#define ADDR_HISTORY_SHIDU_1_2		0x0e03
#define ADDR_HISTORY_SHIDU_2_2		0x0e04
#define ADDR_HISTORY_SHIDU_3_2		0x0e05
#define ADDR_HISTORY_SHIDU_4_2		0x0e06
#define ADDR_HISTORY_SHIDU_5_2		0x0e07
#define ADDR_HISTORY_SHIDU_6_2		0x0e08

#define ADDR_HISTORY_CO_1_1		0x0de5
#define ADDR_HISTORY_CO_2_1		0x0de6
#define ADDR_HISTORY_CO_3_1		0x0de7
#define ADDR_HISTORY_CO_4_1		0x0de8
#define ADDR_HISTORY_CO_5_1		0x0de9
#define ADDR_HISTORY_CO_6_1		0x0dea
#define ADDR_HISTORY_CO_1_2		0x0deb
#define ADDR_HISTORY_CO_2_2		0x0dec
#define ADDR_HISTORY_CO_3_2		0x0ded
#define ADDR_HISTORY_CO_4_2		0x0dee
#define ADDR_HISTORY_CO_5_2		0x0def
#define ADDR_HISTORY_CO_6_2		0x0df0

#define ADDR_HISTORY_CO2_1_1		0x0df1
#define ADDR_HISTORY_CO2_2_1		0x0df2
#define ADDR_HISTORY_CO2_3_1		0x0df3
#define ADDR_HISTORY_CO2_4_1		0x0df4
#define ADDR_HISTORY_CO2_5_1		0x0df5
#define ADDR_HISTORY_CO2_6_1		0x0df6
#define ADDR_HISTORY_CO2_1_2		0x0df7
#define ADDR_HISTORY_CO2_2_2		0x0df8
#define ADDR_HISTORY_CO2_3_2		0x0df9
#define ADDR_HISTORY_CO2_4_2		0x0dfa
#define ADDR_HISTORY_CO2_5_2		0x0dfb
#define ADDR_HISTORY_CO2_6_2		0x0dfc

#define ADDR_HISTORY_HCHO_1_1		0x0e45
#define ADDR_HISTORY_HCHO_2_1		0x0e46
#define ADDR_HISTORY_HCHO_3_1		0x0e47
#define ADDR_HISTORY_HCHO_4_1		0x0e48
#define ADDR_HISTORY_HCHO_5_1		0x0e49
#define ADDR_HISTORY_HCHO_6_1		0x0e4a
#define ADDR_HISTORY_HCHO_1_2		0x0e4b
#define ADDR_HISTORY_HCHO_2_2		0x0e4c
#define ADDR_HISTORY_HCHO_3_2		0x0e4d
#define ADDR_HISTORY_HCHO_4_2		0x0e4e
#define ADDR_HISTORY_HCHO_5_2		0x0e4f
#define ADDR_HISTORY_HCHO_6_2		0x0e50

#define ADDR_HISTORY_PM10_1_1		0x0e51
#define ADDR_HISTORY_PM10_2_1		0x0e52
#define ADDR_HISTORY_PM10_3_1		0x0e53
#define ADDR_HISTORY_PM10_4_1		0x0e54
#define ADDR_HISTORY_PM10_5_1		0x0e55
#define ADDR_HISTORY_PM10_6_1		0x0e56
#define ADDR_HISTORY_PM10_1_2		0x0e57
#define ADDR_HISTORY_PM10_2_2		0x0e58
#define ADDR_HISTORY_PM10_3_2		0x0e59
#define ADDR_HISTORY_PM10_4_2		0x0e5a
#define ADDR_HISTORY_PM10_5_2		0x0e5b
#define ADDR_HISTORY_PM10_6_2		0x0e5c

#define ADDR_HISTORY_PM25_1_1		0x0dd9
#define ADDR_HISTORY_PM25_2_1		0x0dda
#define ADDR_HISTORY_PM25_3_1		0x0ddb
#define ADDR_HISTORY_PM25_4_1		0x0ddc
#define ADDR_HISTORY_PM25_5_1		0x0ddd
#define ADDR_HISTORY_PM25_6_1		0x0dde
#define ADDR_HISTORY_PM25_1_2		0x0ddf
#define ADDR_HISTORY_PM25_2_2		0x0de0
#define ADDR_HISTORY_PM25_3_2		0x0de1
#define ADDR_HISTORY_PM25_4_2		0x0de2
#define ADDR_HISTORY_PM25_5_2		0x0de3
#define ADDR_HISTORY_PM25_6_2		0x0de4

#define ADDR_HISTORY_O3_1_1		0x0e39
#define ADDR_HISTORY_O3_2_1		0x0e3a
#define ADDR_HISTORY_O3_3_1		0x0e3b
#define ADDR_HISTORY_O3_4_1		0x0e3c
#define ADDR_HISTORY_O3_5_1		0x0e3d
#define ADDR_HISTORY_O3_6_1		0x0e3e
#define ADDR_HISTORY_O3_1_2		0x0e3f
#define ADDR_HISTORY_O3_2_2		0x0e40
#define ADDR_HISTORY_O3_3_2		0x0e41
#define ADDR_HISTORY_O3_4_2		0x0e42
#define ADDR_HISTORY_O3_5_2		0x0e43
#define ADDR_HISTORY_O3_6_2		0x0e44

#define ADDR_HISTORY_TVOC_1_1		0x0e2d
#define ADDR_HISTORY_TVOC_2_1		0x0e2e
#define ADDR_HISTORY_TVOC_3_1		0x0e2f
#define ADDR_HISTORY_TVOC_4_1		0x0e30
#define ADDR_HISTORY_TVOC_5_1		0x0e31
#define ADDR_HISTORY_TVOC_6_1		0x0e32
#define ADDR_HISTORY_TVOC_1_2		0x0e33
#define ADDR_HISTORY_TVOC_2_2		0x0e34
#define ADDR_HISTORY_TVOC_3_2		0x0e35
#define ADDR_HISTORY_TVOC_4_2		0x0e36
#define ADDR_HISTORY_TVOC_5_2		0x0e37
#define ADDR_HISTORY_TVOC_6_2		0x0e38

#define ADDR_HISTORY_WIND_1_1		0x0e09
#define ADDR_HISTORY_WIND_2_1		0x0e0a
#define ADDR_HISTORY_WIND_3_1		0x0e0b
#define ADDR_HISTORY_WIND_4_1		0x0e0c
#define ADDR_HISTORY_WIND_5_1		0x0e0d
#define ADDR_HISTORY_WIND_6_1		0x0e0e
#define ADDR_HISTORY_WIND_1_2		0x0e0f
#define ADDR_HISTORY_WIND_2_2		0x0e10
#define ADDR_HISTORY_WIND_3_2		0x0e11
#define ADDR_HISTORY_WIND_4_2		0x0e12
#define ADDR_HISTORY_WIND_5_2		0x0e13
#define ADDR_HISTORY_WIND_6_2		0x0e14

#define ADDR_HISTORY_NOISE_1_1		0x0e15
#define ADDR_HISTORY_NOISE_2_1		0x0e16
#define ADDR_HISTORY_NOISE_3_1		0x0e17
#define ADDR_HISTORY_NOISE_4_1		0x0e18
#define ADDR_HISTORY_NOISE_5_1		0x0e19
#define ADDR_HISTORY_NOISE_6_1		0x0e1a
#define ADDR_HISTORY_NOISE_1_2		0x0e1b
#define ADDR_HISTORY_NOISE_2_2		0x0e1c
#define ADDR_HISTORY_NOISE_3_2		0x0e1d
#define ADDR_HISTORY_NOISE_4_2		0x0e1e
#define ADDR_HISTORY_NOISE_5_2		0x0e1f
#define ADDR_HISTORY_NOISE_6_2		0x0e20

#define ADDR_HISTORY_PRESS_1_1		0x0e21
#define ADDR_HISTORY_PRESS_2_1		0x0e22
#define ADDR_HISTORY_PRESS_3_1		0x0e23
#define ADDR_HISTORY_PRESS_4_1		0x0e24
#define ADDR_HISTORY_PRESS_5_1		0x0e25
#define ADDR_HISTORY_PRESS_6_1		0x0e26
#define ADDR_HISTORY_PRESS_1_2		0x0e27
#define ADDR_HISTORY_PRESS_2_2		0x0e28
#define ADDR_HISTORY_PRESS_3_2		0x0e29
#define ADDR_HISTORY_PRESS_4_2		0x0e2a
#define ADDR_HISTORY_PRESS_5_2		0x0e2b
#define ADDR_HISTORY_PRESS_6_2		0x0e2c

#if 0
#define OFF_PAGE 				20
#define LIST_PM25_PAGE 			8
#define LIST_SHIDU_PAGE 		7
#define LIST_TEMP_PAGE 			6
#define LIST_HCHO_PAGE 			5
#define LIST_CO2_PAGE 			4
#define LIST_CO_PAGE 			2
#define LOG_IN_PAGE 			23
#define CURVE_PAGE				3
#define TIME_SETTING_PAGE 		25
#define STATE_E_E_PAGE			22
#define STATE_E_O_PAGE 			21
#define STATE_O_O_PAGE 			27
#define STATE_O_E_PAGE 			9
#define INTERFACE_SELECT_PAGE 	12
#define INTERFACE_ALL_PAGE 		14
#define SYSTEM_SET_PAGE 		24
#define SENSOR_TEST_PAGE		13
#define TUN_ZERO_PAGE			28
#define VERIFY_SELECT_PAGE		29
#define VERIFY_PAGE				19
#define WIFI_PAGE				16
#define GPRS_PAGE				15
//page 1
#define ADDR_RUN_TIME_CO		0x0000
#define ADDR_RUN_TIME_CO2		0x0001
#define ADDR_RUN_TIME_HCHO		0x0002
#define ADDR_RUN_TIME_TEMP		0x0003
#define ADDR_RUN_TIME_SHIDU		0x0004
#define ADDR_RUN_TIME_PM25		0x0005
#define TOUCH_RUN_TIME_CO		0x0006
#define TOUCH_RUN_TIME_CO2		0x0007
#define TOUCH_RUN_TIME_HCHO		0x0008
#define TOUCH_RUN_TIME_TEMP		0x0009
#define TOUCH_RUN_TIME_SHIDU	0x000a
#define TOUCH_RUN_TIME_PM25		0x000b
#define TOUCH_DEVICE_STATE		0x000c
#define TOUCH_PRODUCT_INFO		0x000d
#define TOUCH_SYSTEM_SETTING	0x000e
#define GRAPHIC_CO_FLASH_LED	0x06f3
//page 2	
#define TOUCH_CO_LAST_PAGE		0x000f
#define TOUCH_CO_NEXT_PAGE		0x0010
#define TOUCH_CO_REFRESH_PAGE	0x0011
#define TOUCH_CO_LIST_RETURN	0x0012
#define ADDR_CO_LIST_TIME_0		0x0013
#define ADDR_CO_LIST_DATA_0		0x0023
#define ADDR_CO_LIST_TIME_1		0x0794
#define ADDR_CO_LIST_DATA_1		0x0037
#define ADDR_CO_LIST_TIME_2		0x002b
#define ADDR_CO_LIST_DATA_2		0x004b
#define ADDR_CO_LIST_TIME_3		0x004f
#define ADDR_CO_LIST_DATA_3		0x005f
#define ADDR_CO_LIST_TIME_4		0x0063
#define ADDR_CO_LIST_DATA_4		0x0073
#define ADDR_CO_LIST_TIME_5		0x0077
#define ADDR_CO_LIST_DATA_5		0x0087
#define ADDR_CO_LIST_TIME_6		0x06b3
#define ADDR_CO_LIST_DATA_6		0x008f
#define ADDR_CO_PAGE_NUM		0x06c3
#define ADDR_CO_PAGE_TOTAL		0x06c7
//page 3	
#define ADDR_CURVE_DATA_5		0x0738
#define ADDR_CURVE_DATA_4		0x0748
#define ADDR_CURVE_DATA_3		0x0758
#define ADDR_CURVE_DATA_2		0x0768
#define ADDR_CURVE_DATA_1		0x0778
#define ADDR_CURVE_DATE			0x00a7
#define ADDR_CURVE_TIME_5		0x00b1
#define ADDR_CURVE_TIME_4		0x00b6
#define ADDR_CURVE_TIME_3		0x00bb
#define ADDR_CURVE_TIME_2		0x00c0
#define ADDR_CURVE_TIME_1		0x00c5
#define ADDR_CURVE_TYPE			0x00ca
#define TOUCH_CURVE_LIST		0x00cb
#define TOUCH_CURVE_RETURN		0x00cc
//page 4	
#define TOUCH_CO2_LAST_PAGE		0x0159
#define TOUCH_CO2_NEXT_PAGE		0x015a
#define TOUCH_CO2_REFRESH_PAGE	0x015b
#define TOUCH_CO2_LIST_RETURN	0x015c
#define ADDR_CO2_LIST_TIME_0	0x00cd
#define ADDR_CO2_LIST_DATA_0	0x00dd
#define ADDR_CO2_LIST_TIME_1	0x00e3
#define ADDR_CO2_LIST_DATA_1	0x00f3
#define ADDR_CO2_LIST_TIME_2	0x00f9
#define ADDR_CO2_LIST_DATA_2	0x0109
#define ADDR_CO2_LIST_TIME_3	0x010f
#define ADDR_CO2_LIST_DATA_3	0x011f
#define ADDR_CO2_LIST_TIME_4	0x0125
#define ADDR_CO2_LIST_DATA_4	0x0135
#define ADDR_CO2_LIST_TIME_5	0x013b
#define ADDR_CO2_LIST_DATA_5	0x014b
#define ADDR_CO2_LIST_TIME_6	0x06f0
#define ADDR_CO2_LIST_DATA_6	0x0700
#define ADDR_CO2_PAGE_NUM		0x06cb
#define ADDR_CO2_PAGE_TOTAL		0x06cf
//page 5	
#define TOUCH_HCHO_LAST_PAGE	0x01E9
#define TOUCH_HCHO_NEXT_PAGE	0x01EA
#define TOUCH_HCHO_REFRESH_PAGE	0x01EB
#define TOUCH_HCHO_LIST_RETURN	0x01EC
#define ADDR_HCHO_LIST_TIME_0	0x015D
#define ADDR_HCHO_LIST_DATA_0	0x016D
#define ADDR_HCHO_LIST_TIME_1	0x0171
#define ADDR_HCHO_LIST_DATA_1	0x0181
#define ADDR_HCHO_LIST_TIME_2	0x0185
#define ADDR_HCHO_LIST_DATA_2	0x0195
#define ADDR_HCHO_LIST_TIME_3	0x0199
#define ADDR_HCHO_LIST_DATA_3	0x01A9
#define ADDR_HCHO_LIST_TIME_4	0x01AD
#define ADDR_HCHO_LIST_DATA_4	0x01BD
#define ADDR_HCHO_LIST_TIME_5	0x01C1
#define ADDR_HCHO_LIST_DATA_5	0x01D1
#define ADDR_HCHO_LIST_TIME_6	0x01D5
#define ADDR_HCHO_LIST_DATA_6	0x01E5
#define ADDR_HCHO_PAGE_NUM		0x06d3
#define ADDR_HCHO_PAGE_TOTAL	0x06d7
//page 6	
#define TOUCH_TEMP_LAST_PAGE	0x0279
#define TOUCH_TEMP_NEXT_PAGE	0x027A
#define TOUCH_TEMP_REFRESH_PAGE	0x027B
#define TOUCH_TEMP_LIST_RETURN	0x027C
#define ADDR_TEMP_LIST_TIME_0	0x01ED
#define ADDR_TEMP_LIST_DATA_0	0x01FD
#define ADDR_TEMP_LIST_TIME_1	0x0201
#define ADDR_TEMP_LIST_DATA_1	0x0211
#define ADDR_TEMP_LIST_TIME_2	0x0215
#define ADDR_TEMP_LIST_DATA_2	0x0225
#define ADDR_TEMP_LIST_TIME_3	0x0229
#define ADDR_TEMP_LIST_DATA_3	0x0239
#define ADDR_TEMP_LIST_TIME_4	0x023D
#define ADDR_TEMP_LIST_DATA_4	0x024D
#define ADDR_TEMP_LIST_TIME_5	0x0251
#define ADDR_TEMP_LIST_DATA_5	0x0261
#define ADDR_TEMP_LIST_TIME_6	0x0265
#define ADDR_TEMP_LIST_DATA_6	0x0275
#define ADDR_TEMP_PAGE_NUM		0x06db
#define ADDR_TEMP_PAGE_TOTAL	0x06df
//page 7	
#define TOUCH_SHIDU_LAST_PAGE	0x0309
#define TOUCH_SHIDU_NEXT_PAGE	0x030A
#define TOUCH_SHIDU_REFRESH_PAGE 0x030B
#define TOUCH_SHIDU_LIST_RETURN	0x030C
#define ADDR_SHIDU_LIST_TIME_0	0x027D
#define ADDR_SHIDU_LIST_DATA_0	0x028D
#define ADDR_SHIDU_LIST_TIME_1	0x0291
#define ADDR_SHIDU_LIST_DATA_1	0x02A1
#define ADDR_SHIDU_LIST_TIME_2	0x02A5
#define ADDR_SHIDU_LIST_DATA_2	0x02B5
#define ADDR_SHIDU_LIST_TIME_3	0x02B9
#define ADDR_SHIDU_LIST_DATA_3	0x02C9
#define ADDR_SHIDU_LIST_TIME_4	0x02CD
#define ADDR_SHIDU_LIST_DATA_4	0x02DD
#define ADDR_SHIDU_LIST_TIME_5	0x02E1
#define ADDR_SHIDU_LIST_DATA_5	0x02F1
#define ADDR_SHIDU_LIST_TIME_6	0x02F5
#define ADDR_SHIDU_LIST_DATA_6	0x0305
#define ADDR_SHIDU_PAGE_NUM		0x06e3
#define ADDR_SHIDU_PAGE_TOTAL	0x06e7
//page 8	
#define TOUCH_PM25_LAST_PAGE	0x0499
#define TOUCH_PM25_NEXT_PAGE	0x049A
#define TOUCH_PM25_REFRESH_PAGE	0x049B
#define TOUCH_PM25_LIST_RETURN	0x049C
#define ADDR_PM25_LIST_TIME_0	0x040D
#define ADDR_PM25_LIST_DATA_0	0x041D
#define ADDR_PM25_LIST_TIME_1	0x0421
#define ADDR_PM25_LIST_DATA_1	0x0431
#define ADDR_PM25_LIST_TIME_2	0x0435
#define ADDR_PM25_LIST_DATA_2	0x0445
#define ADDR_PM25_LIST_TIME_3	0x0449
#define ADDR_PM25_LIST_DATA_3	0x0459
#define ADDR_PM25_LIST_TIME_4	0x045D
#define ADDR_PM25_LIST_DATA_4	0x046D
#define ADDR_PM25_LIST_TIME_5	0x0471
#define ADDR_PM25_LIST_DATA_5	0x0481
#define ADDR_PM25_LIST_TIME_6	0x0485
#define ADDR_PM25_LIST_DATA_6	0x0495
#define ADDR_PM25_PAGE_NUM		0x06eb
#define ADDR_PM25_PAGE_TOTAL	0x06ef
//page 9	
#define TOUCH_STATE_RETURN_1	0x049d
//page 10	
#define ADDR_PRODUCT_NAME		0x049e
#define ADDR_PRODUCT_MODE		0x04c6
#define ADDR_PRODUCT_ID			0x04EE
#define TOUCH_PRODUCT_RETURN	0x0740
//page 11	
#define TOUCH_COMPANY_RETURN	0x0516
//page 12	
#define ADDR_INTERFACE_1		0x0517
#define ADDR_INTERFACE_2		0x0521
#define ADDR_INTERFACE_3		0x052b
#define ADDR_INTERFACE_4		0x0535
#define ADDR_INTERFACE_5		0x053f
#define ADDR_INTERFACE_6		0x0549
#define ADDR_INTERFACE_7		0x073e
#define ADDR_INTERFACE_8		0x078a
#define ADDR_INTERFACE_9		0x0752
#define ADDR_INTERFACE_10		0x075c
#define ADDR_INTERFACE_11		0x088a
#define TOUCH_INTERFACE_OK		0x0553
#define TOUCH_INTERFACE_RETURN	0x0554
#define TOUCH_INTERFACE_1		0x0555
#define TOUCH_INTERFACE_2		0x0556
#define TOUCH_INTERFACE_3		0x0557
#define TOUCH_INTERFACE_4		0x0558
#define TOUCH_INTERFACE_5		0x0559
#define TOUCH_INTERFACE_6		0x055a
#define TOUCH_INTERFACE_7		0x073a
#define TOUCH_INTERFACE_8		0x073b
#define TOUCH_INTERFACE_9		0x073c
#define TOUCH_INTERFACE_10		0x073d
#define TOUCH_INTERFACE_11		0x0894
//page 13	
#define TOUCH_TUN_ZERO			0x055b
#define TOUCH_VERIFY			0x055c
#define TOUCH_INTERFACE_SET		0x055d
#define TOUCH_SETTING_RETURN	0x055e
//page 14	
#define TOUCH_SET_HCHO_1		0x055f
#define TOUCH_SET_CO_1			0x0560
#define TOUCH_SET_CO2_1			0x0561
#define TOUCH_SET_WENSHI_1		0x0562
#define TOUCH_SET_PM25_1		0x0563
#define TOUCH_SET_HCHO_2		0x0564
#define TOUCH_SET_CO_2			0x0565
#define TOUCH_SET_ZAOSHEN		0x0567
#define TOUCH_SET_FENGSU		0x0568
#define TOUCH_SET_CO2_2			0x0566
#define TOUCH_SET_RETURN		0x057B
#define TOUCH_SET_QIYA			0x074b
#define TOUCH_UNKNOWN			0x07a4
#define ADDR_CUR_SELECT			0x0741
//page 16	
#define TOUCH_WIFI_SET_RETURN	0x057c
//page 15
#define TOUCH_GPRS_SET_RETURN	0x055F
//page 17	
#define ADDR_XFER_MODE			0x057d
#define ADDR_AP_NAME			0x0581
#define ADDR_AP_PASSWD			0x0595
#define TOUCH_SELECT_GPRS		0x05a9
#define TOUCH_SELECT_WIFI		0x05aa
#define TOUCH_SELECT_OK			0x05ab
#define TOUCH_SELECT_RETURN		0x073f
//page 18	
#define ADDR_USER_NAME			0x05ac
#define ADDR_INSTALL_PLACE		0x05bc
#define ADDR_USER_INDUSTRY		0x05e6
#define ADDR_USER_ADDR			0x0610
#define ADDR_USER_PHONE			0x0630
#define ADDR_USER_CONTACTER		0x063c
#define TOUCH_USER_RETURN		0x064c
//page 20	
#define TOUCH_SLEEP_WAKE		0x064d
//page 21	
#define TOUCH_STATE_RETURN_2	0x064e
//page 22	
#define TOUCH_STATE_RETURN_3	0x064f
//page 23	
#define ADDR_USER_NAME_VERIFY	0x0650
#define ADDR_USER_PWD_VERIFY	0x0660
#define TOUCH_USER_NAME_VERIFY	0x0650
#define TOUCH_USER_PWD_VERIFY	0x0660
#define TOUCH_USER_OK_VERIFY	0x0670
#define TOUCH_USER_RETURN_VERIFY 0x0671
//page 24	
#define TOUCH_SENSOR_SET		0x0672
#define TOUCH_XFER_SET			0x0673
#define TOUCH_TIME_SET			0x0674
#define TOUCH_FACTORY_INFO		0x0677
#define TOUCH_USER_INFO			0x0675
#define TOUCH_SYS_SET_RETURN	0x0676
//page 25	
#define ADDR_TIME_YEAR			0x06a1
#define ADDR_TIME_MONTH			0x06a5
#define ADDR_TIME_DAY			0x06a7
#define ADDR_TIME_HOUR			0x06a9
#define ADDR_TIME_MIN			0x06ab
#define ADDR_TIME_SECOND		0x06ad
#define TOUCH_TIME_CHANGE_MANUL	0x06af
#define TOUCH_TIME_CHANGE_SERVER 0x06b0
#define TOUCH_TIME_CHANGE_RETURN 0x06b2
#define TOUCH_TIME_CHANGE_OK	0x06b1
//page 27	
#define TOUCH_STATE_RETURN_4	0x06b2
//page 28	
#define TOUCH_TUN_ZERO_BEGIN	0x0706
#define TOUCH_TUN_ZERO_END		0x0707
#define TOUCH_TUN_ZERO_RETURN	0x0708
#define ADDR_TUN_ZERO_CO		0x0701
#define ADDR_TUN_ZERO_HCHO		0x0705
//page 19	
#define ADDR_VERIFY_P_0			0x0709
#define ADDR_VERIFY_P_1			0x070E
#define ADDR_VERIFY_P_2			0x0713
#define ADDR_VERIFY_P_3			0x0718
#define ADDR_VERIFY_P_4			0x071d
#define ADDR_VERIFY_P_5			0x0722
#define ADDR_VERIFY_P_6			0x0727
#define ADDR_VERIFY_P_7			0x072c
#define ADDR_VERIFY_VALUE		0x0880
#define ADDR_REAL_VALUE			0x0736
#define TOUCH_VERIFY_EXIT		0x0737
#define TOUCH_JIAJIA			0x0738
#define TOUCH_JIANJIAN			0x0739
#define TOUCH_JIAOZHUN_P0		0x081e
#define TOUCH_JIAOZHUN_P1		0x081F
#define TOUCH_JIAOZHUN_P2		0x0820
#define TOUCH_JIAOZHUN_P3		0x0821
#define TOUCH_JIAOZHUN_P4		0x0822
#define TOUCH_JIAOZHUN_P5		0x0823
#define TOUCH_JIAOZHUN_P6		0x0824
#define TOUCH_JIAOZHUN_P7		0x0825
#define TOUCH_JIAOZHUN_UP		0x082e
#define TOUCH_JIAOZHUN_DOWN		0x082f

//page 29
#define TOUCH_VERIFY_HCHO		0x07a5
#define TOUCH_VERIFY_PM25		0x07a6
#define TOUCH_VERIFY_INT3		0x07a7
#define TOUCH_VERIFY_INT4		0x07a8
#define TOUCH_VERIFY_INT5		0x07a9
#define TOUCH_VERIFY_INT6		0x07aa
#define TOUCH_VERIFY_INT7		0x07ab
#define TOUCH_VERIFY_INT8		0x07ac
#define TOUCH_VERIFY_WENSHI		0x07ad
#define TOUCH_VERIFY_FENGSU		0x07ae
#define TOUCH_VERIFY_QIYA		0x07af
#define ADDR_VERIFY_HCHO		0x07b0
#define ADDR_VERIFY_PM25		0x07bA
#define ADDR_VERIFY_INT3		0x07c4
#define ADDR_VERIFY_INT4		0x07CE
#define ADDR_VERIFY_INT5		0x07D8
#define ADDR_VERIFY_INT6		0x07E2
#define ADDR_VERIFY_INT7		0x07EC
#define ADDR_VERIFY_INT8		0x07F6
#define ADDR_VERIFY_WENSHI		0x0800
#define ADDR_VERIFY_FENGSU		0x080A
#define ADDR_VERIFY_QIYA		0x0814
#endif
#define TYPE_DGRAM_REGISTER				"1"
#define TYPE_DGRAM_DATA					"2"
#define TYPE_DGRAM_RE_DATA				"3"
#define TYPE_DGRAM_WARNING				"4"
#define TYPE_DGRAM_SYNC					"5"
#define TYPE_DGRAM_ASK_RE_DATA			"6"
#define TYPE_DGRAM_VERIFY_USER			"7"
#define TYPE_DGRAM_CLEAR_WARNING		"8"
#define TYPE_DEVICE_TRANS_WIFI			"2"
#define TYPE_DEVICE_TRANS_GPRS			"1"
#define TYPE_DEVICE_TRANS_3G			"3"
#define TYPE_DEVICE_TRANS_4G			"4"

#define ID_DGRAM_TYPE					"0"
#define ID_DEVICE_UID					"30"
#define ID_DEVICE_TYPE					"31"
#define ID_DEVICE_SUB_TYPE				"32"
#define ID_DEVICE_SW_VERSION			"33"
#define ID_DEVICE_TRANS_TYPE			"34"
#define ID_DEVICE_IP_ADDR				"35"
#define ID_DEVICE_PORT					"36"
#define ID_RE_DGRAM_TYPE				"38"
#define ID_USER_NAME_TYPE				"39"
#define ID_USER_PWD_TYPE				"40"
#define ID_ALARM_SENSOR					"41"
#define ID_ALARM_TYPE					"42"

#define ID_DEVICE_CAP_TIME				"103"
#define ID_CAP_CO						"60"
#define ID_CAP_CO2						"61"
#define ID_CAP_HCHO						"62"
#define ID_CAP_TEMPERATURE				"63"
#define ID_CAP_SHI_DU					"64"
#define ID_CAP_PM_25					"65"
#define ID_CAP_PM_10					"66"
#define ID_CAP_BUZZY					"67"
#define ID_CAP_FENG_SU					"68"
#define ID_CAP_QI_YA					"69"
#define ID_CAP_CHOU_YANG				"70"
#define ID_CAP_SO2						"71"
#define ID_CAP_DONG_QI					"72"
#define ID_CAP_ZI_WAI					"73"
#define ID_CAP_TVOC						"74"
#define ID_CAP_BEN						"75"
#define ID_CAP_JIA_BEN					"76"
#define ID_CAP_2_JIA_BEN				"77"
#define ID_CAP_AN_QI1					"78"
#define ID_CAP_HS						"79"
#define ID_CAP_NO						"160"
#define ID_CAP_NO2						"161"
#define ID_CAP_AN_QI					"162"
#define ID_CAP_ZHAO_DU					"163"
#define ID_CAP_WEI_S_WU					"164"

#define ID_CAP_CO_EXT					"260"
#define ID_CAP_CO2_EXT					"261"
#define ID_CAP_HCHO_EXT					"262"
#define ID_CAP_CHOU_YANG_EXT			"270"
#define ID_CAP_SO2_EXT					"271"
#define ID_CAP_TVOC_EXT					"274"
#define ID_CAP_BEN_EXT					"275"
#define ID_CAP_JIA_BEN_EXT				"276"
#define ID_CAP_2_JIA_BEN_EXT			"277"
#define ID_CAP_NO_EXT					"360"
#define ID_CAP_NO2_EXT					"361"
#define ID_CAP_AN_QI_EXT				"362"

#define ID_ALERT_OFF					"90"
#define ID_ALERT_SWITCH_CHANNEL			"91"
#define ID_ALERT_CAP_FAILED				"92"
#define ID_ALERT_NO_ACK					"93"
#define ID_ALERT_POWER_OFF				"94"
#define ID_ALERT_UP						"95"
#define ID_ALERT_BELOW					"96"
#define ID_ALERT_UNINSERT				"97"
#define ID_RE_START_TIME				"101"
#define ID_RE_STOP_TIME					"102"
#define ID_CAP_TIME						"103"
#define ID_SERVER_TIME					"104"
#define ID_USER_LOCAL_PLACE				"200"
#define ID_USER_NAME					"201"
#define ID_USER_PHONE					"202"
#define ID_USER_ID						"203"
#define ID_DEVICE_SETUP_TIME			"210"
#define ID_DEVICE_SETUP_PLACE			"211"
#define ID_DEVICE_INFO					"212"
#define ID_DEVICE_STATUS				"213"

#define MAX_CO		200
#define MIN_CO		0
#define MAX_CO2		10000
#define MIN_CO2		300
#define MAX_HCHO	3
#define MIN_HCHO	0
#define MAX_SHIDU	90
#define MIN_SHIDU	0
#define MAX_TEMP	50
#define MIN_TEMP	(-20)
#define MAX_PM25	1000
#define MIN_PM25	0
#define MIN_NOISE	0
#define MAX_NOISE	300
#define	MIN_PRESS	0
#define MAX_PRESS	10000
#define MIN_TVOC	0
#define MAX_TVOC	100
#define MIN_WIND	0
#define MAX_WIND	100
#define MIN_O3		0
#define MAX_O3		100
#define MIN_PM10	0
#define MAX_PM10	500
#define MAX_COUNT_TIMES 12
#if 0
#define STATE_LOGO			0
#define STATE_MAIN			1
#define STATE_DETAIL_CO		2
#define STATE_DETAIL_CO2	3
#define STATE_DETAIL_HCHO	4
#define STATE_DETAIL_TMP	5
#define STATE_DETAIL_SHIDU	6
#define STATE_DETAIL_PM25	7

#define VAR_DATE_TIME_1	 0x0000
#define VAR_DATE_TIME_2  0x0001
#define VAR_DATE_TIME_3	 0x0002
#define VAR_DATE_TIME_4	 0x0003
#define VAR_ALARM_TYPE_1 0x0004
#define VAR_ALARM_TYPE_2 0x0005
#define VAR_CO_TIME1	 0x0148
#define VAR_CO_DATA1	 (VAR_CO_TIME1+16)
#define VAR_CO_TIME2	 (VAR_CO_DATA1+4)
#define VAR_CO_DATA2	 (VAR_CO_TIME2+16)
#define VAR_CO_TIME3	 (VAR_CO_DATA2+4)
#define VAR_CO_DATA3	 (VAR_CO_TIME3+16)
#define VAR_CO_TIME4	 (VAR_CO_DATA3+4)
#define VAR_CO_DATA4	 (VAR_CO_TIME4+16)
#define VAR_CO_TIME5	 (VAR_CO_DATA4+4)
#define VAR_CO_DATA5	 (VAR_CO_TIME5+16)
#define VAR_CO_TIME6	 (VAR_CO_DATA5+4)
#define VAR_CO_DATA6	 (VAR_CO_TIME6+16)
#define VAR_CO_TIME7	 (VAR_CO_DATA6+4)
#define VAR_CO_DATA7	 (VAR_CO_TIME7+16)

#define VAR_CO2_TIME1	 (VAR_CO_DATA7+4)
#define VAR_CO2_DATA1	 (VAR_CO2_TIME1+16)
#define VAR_CO2_TIME2	 (VAR_CO2_DATA1+4)
#define VAR_CO2_DATA2	 (VAR_CO2_TIME2+16)
#define VAR_CO2_TIME3	 (VAR_CO2_DATA2+4)
#define VAR_CO2_DATA3	 (VAR_CO2_TIME3+16)
#define VAR_CO2_TIME4	 (VAR_CO2_DATA3+4)
#define VAR_CO2_DATA4	 (VAR_CO2_TIME4+16)
#define VAR_CO2_TIME5	 (VAR_CO2_DATA4+4)
#define VAR_CO2_DATA5	 (VAR_CO2_TIME5+16)
#define VAR_CO2_TIME6	 (VAR_CO2_DATA5+4)
#define VAR_CO2_DATA6	 (VAR_CO2_TIME6+16)
#define VAR_CO2_TIME7	 (VAR_CO2_DATA6+4)
#define VAR_CO2_DATA7	 (VAR_CO2_TIME7+16)

#define VAR_HCHO_TIME1	 (VAR_CO2_DATA7+4)
#define VAR_HCHO_DATA1	 (VAR_HCHO_TIME1+16)
#define VAR_HCHO_TIME2	 (VAR_HCHO_DATA1+4)
#define VAR_HCHO_DATA2	 (VAR_HCHO_TIME2+16)
#define VAR_HCHO_TIME3	 (VAR_HCHO_DATA2+4)
#define VAR_HCHO_DATA3	 (VAR_HCHO_TIME3+16)
#define VAR_HCHO_TIME4	 (VAR_HCHO_DATA3+4)
#define VAR_HCHO_DATA4	 (VAR_HCHO_TIME4+16)
#define VAR_HCHO_TIME5	 (VAR_HCHO_DATA4+4)
#define VAR_HCHO_DATA5	 (VAR_HCHO_TIME5+16)
#define VAR_HCHO_TIME6	 (VAR_HCHO_DATA5+4)
#define VAR_HCHO_DATA6	 (VAR_HCHO_TIME6+16)
#define VAR_HCHO_TIME7	 (VAR_HCHO_DATA6+4)
#define VAR_HCHO_DATA7	 (VAR_HCHO_TIME7+16)

#define VAR_TEMP_TIME1	 (VAR_HCHO_DATA7+4)
#define VAR_TEMP_DATA1	 (VAR_TEMP_TIME1+16)
#define VAR_TEMP_TIME2	 (VAR_TEMP_DATA1+4)
#define VAR_TEMP_DATA2	 (VAR_TEMP_TIME2+16)
#define VAR_TEMP_TIME3	 (VAR_TEMP_DATA2+4)
#define VAR_TEMP_DATA3	 (VAR_TEMP_TIME3+16)
#define VAR_TEMP_TIME4	 (VAR_TEMP_DATA3+4)
#define VAR_TEMP_DATA4	 (VAR_TEMP_TIME4+16)
#define VAR_TEMP_TIME5	 (VAR_TEMP_DATA4+4)
#define VAR_TEMP_DATA5	 (VAR_TEMP_TIME5+16)
#define VAR_TEMP_TIME6	 (VAR_TEMP_DATA5+4)
#define VAR_TEMP_DATA6	 (VAR_TEMP_TIME6+16)
#define VAR_TEMP_TIME7	 (VAR_TEMP_DATA6+4)
#define VAR_TEMP_DATA7	 (VAR_TEMP_TIME7+16)

#define VAR_SHIDU_TIME1	 (VAR_TEMP_DATA7+4)
#define VAR_SHIDU_DATA1	 (VAR_SHIDU_TIME1+16)
#define VAR_SHIDU_TIME2	 (VAR_SHIDU_DATA1+4)
#define VAR_SHIDU_DATA2	 (VAR_SHIDU_TIME2+16)
#define VAR_SHIDU_TIME3	 (VAR_SHIDU_DATA2+4)
#define VAR_SHIDU_DATA3	 (VAR_SHIDU_TIME3+16)
#define VAR_SHIDU_TIME4	 (VAR_SHIDU_DATA3+4)
#define VAR_SHIDU_DATA4	 (VAR_SHIDU_TIME4+16)
#define VAR_SHIDU_TIME5	 (VAR_SHIDU_DATA4+4)
#define VAR_SHIDU_DATA5	 (VAR_SHIDU_TIME5+16)
#define VAR_SHIDU_TIME6	 (VAR_SHIDU_DATA5+4)
#define VAR_SHIDU_DATA6	 (VAR_SHIDU_TIME6+16)
#define VAR_SHIDU_TIME7	 (VAR_SHIDU_DATA6+4)
#define VAR_SHIDU_DATA7	 (VAR_SHIDU_TIME7+16)

#define VAR_PM25_TIME1	 (VAR_SHIDU_DATA7+4)
#define VAR_PM25_DATA1	 (VAR_PM25_TIME1+16)
#define VAR_PM25_TIME2	 (VAR_PM25_DATA1+4)
#define VAR_PM25_DATA2	 (VAR_PM25_TIME2+16)
#define VAR_PM25_TIME3	 (VAR_PM25_DATA2+4)
#define VAR_PM25_DATA3	 (VAR_PM25_TIME3+16)
#define VAR_PM25_TIME4	 (VAR_PM25_DATA3+4)
#define VAR_PM25_DATA4	 (VAR_PM25_TIME4+16)
#define VAR_PM25_TIME5	 (VAR_PM25_DATA4+4)
#define VAR_PM25_DATA5	 (VAR_PM25_TIME5+16)
#define VAR_PM25_TIME6	 (VAR_PM25_DATA5+4)
#define VAR_PM25_DATA6	 (VAR_PM25_TIME6+16)
#define VAR_PM25_TIME7	 (VAR_PM25_DATA6+4)
#define VAR_PM25_DATA7	 (VAR_PM25_TIME7+16)

#define TOUCH_DETAIL_CO	 			0x0102
#define TOUCH_DETAIL_CO2 			0x0103
#define TOUCH_DETAIL_HCHO 			0x0104
#define TOUCH_DETAIL_TEMP			0x0105
#define TOUCH_DETAIL_SHIDU			0x0106
#define TOUCH_DETAIL_PM25			0x0107
#define TOUCH_UPDATE_CO	 			0x0108
#define TOUCH_UPDATE_CO2 			0x0110
#define TOUCH_UPDATE_HCHO 			0x0112
#define TOUCH_UPDATE_TEMP			0x0114
#define TOUCH_UPDATE_SHIDU			0x0116
#define TOUCH_UPDATE_PM25			0x0117
#define TOUCH_SENSOR_NETWORK_STATE	0x0121
#define TOUCH_LOGIN_HISTORY			0x012e
#define USER_NAME_ADDR				0x048d
#define USER_PWD_ADDR				0x0497
#define TOUCH_WIFI_HANDLE			0x0140
#define WIFI_AP_NAME_ADDR			0x01c2
#define WIFI_AP_PWD_ADDR			0x01a8
#define TOUCH_TIME_SET_MANUL		0x0142
#define TOUCH_TIME_SET_AUTO			0x0143
#define TIME_YEAR_ADDR				0x01af
#define TIME_MON_ADDR				0x01b3
#define TIME_DAY_ADDR				0x04e3
#define TIME_HOUR_ADDR				0x04e5
#define TIME_MIN_ADDR				0x04e7
#define TIME_SECONDS_ADDR			0x04e9
#define TOUCH_SET_TIME				0x0132
#define TOUCH_LIST_DISPLAY			0x0498
#define TOUCH_WIFI_ENTER			0x013e
#define TOUCH_VERIFY_SENSOR			0x04c7
#define VAR_USER_NAME_ADDR			0x04eb
#define VAR_USER_PLACE_ADDR			0x0513
#define VAR_USER_ADDR_ADDR			0x0577
#define VAR_USER_PHONE_ADDR			0x059f
#define VAR_USER_CONTRACT_ADDR		0x05c7

#define RET_MAIN_CO			0x0109
#define RET_MAIN_CO2		0x0111
#define RET_MAIN_HCHO		0x0113
#define RET_MAIN_TMP		0x0115
#define RET_MAIN_PM25		0x0118
#define RET_MAIN_SHIDU		0x0119

#define VAR_ALARM_TYPE_3 0x0110
#define VAR_ALARM_TYPE_4 0x0111
#define VAR_RUN_TIME	 0x0112
#define VAR_REAL_TIME_TEMP 0x0113
#endif
#define TYPE_SENSOR_CO_WEISHEN	0x0101
#define TYPE_SENSOR_CO_DD	0x0102
#define TYPE_SENSOR_CO2_WEISHEN	0x0201
#define TYPE_SENSOR_CO2_RUDIAN	0x0202
#define TYPE_SENSOR_CH2O_WEISHEN	0x0301
#define TYPE_SENSOR_CH2O_AERSHEN	0x0302
#define TYPE_SENSOR_PM25_WEISHEN	0x0401
#define TYPE_SENSOR_PM25_WEISHEN2	0x0402
#define TYPE_SENSOR_WENSHI_RUSHI	0x0501
#define TYPE_SENSOR_QIYA_RUSHI	0x0801
#define TYPE_SENSOR_FENGSU	0x0701
#define TYPE_SENSOR_ZHAOSHEN	0x0601
#define URL "http://123.57.26.24:8080/saveData/airmessage/messMgr.do"
//#define URL "http://101.200.182.92:8080/saveData/airmessage/messMgr.do"
#define START_BYTE 0x6C
#define CAP_TO_ARM 0xAA
#define ARM_TO_CAP 0xBB
#define VERIFY_BYTE	0x0003
#define RESEND_BYTE	0x0002
#define TIME_BYTE	0x0001
#define ERROR_BYTE	0xFF
#define NORMAL_MODE	0
#define TUN_ZERO_MODE	1
#define SENSOR_VERIFY_MODE	2
#define CONFIG_FILE "sensor_alarm.cfg"
#define FILE_PATH	"/home/user/history/"
#define PERM S_IRUSR|S_IWUSR  
#define ALARM_NONE		0
#define ALARM_BELOW 	1
#define ALARM_UP		2
#define ALARM_UNINSERT 	4
#define STATE_IDLE 	0
#define STATE_6C 	1
#define STATE_AA 	2
#define STATE_MESSAGE_TYPE 3
#define STATE_MESSAGE_LEN 4
#define STATE_MESSAGE 5
#define STATE_CRC 6
void	send_cmd_to_cap(char *cmd,int len);
int cap_init();
void return_zero_point(int co);

#endif
