#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <errno.h>  
#include <sys/msg.h>  
#include <signal.h>
#include <fnmatch.h>
#define PERM S_IRUSR|S_IWUSR  
#define TYPE_MAIN_TO_WEB 				0x02
#define TYPE_WEB_TO_MAIN 				0x01
#define MAIN_TO_WEB 					0x03
#define WEB_TO_MAIN 					0x01
#define LOG_PREFX 						"[WebSubSystem]:"
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
#define MAX_COUNT_TIMES 12
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

struct msg_st
{  
	long int msg_type; 
	int id;
	char text[512];  
}; 

