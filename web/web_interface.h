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
#define ID_CAP_AN_QI					"78"
#define ID_CAP_HS						"79"
#define ID_CAP_NO						"160"
#define ID_CAP_NO2						"161"
#define ID_CAP_ZHAO_DU					"163"
#define ID_CAP_WEI_S_WU					"164"

#define ID_ALERT_OFF					"90"
#define ID_ALERT_SWITCH_CHANNEL			"91"
#define ID_ALERT_CAP_FAILED				"92"
#define ID_ALERT_NO_ACK					"93"
#define ID_ALERT_POWER_OFF				"94"
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

struct msg_st
{  
	long int msg_type; 
	int id;
	char text[512];  
}; 

