#ifndef _PROCESS_H
#define _PROCESS_H
struct msg_st  
{  
	long int msg_type;
	int len;
	char text[512];  
}; 
#define MESSAGE_TYPE_CAP_TIME		0x0001
#define MESSAGE_TYPE_CAP_RESEND		0x0002
#define MESSAGE_TYPE_CAP_ACK		0x0005
#define MESSAGE_TYPE_SENSOR_LIST	0x0004
#define MESSAGE_TYPE_VERIFY_LIST	0x0003
#define MESSAGE_TYPE_VERSION		0x0006

uint32_t process_message(void);
#endif
