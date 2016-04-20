#ifndef _MISC_H
#define _MISC_H
void get_sensor_alarm_info();
void save_sensor_alarm_info();
int open_com_port(char *dev);
int set_alarm(int hour,int mintue,int sec);
void set_time(int year,int mon,int day,int hour,int minute,int second);
void dump_curr_time(int fd);
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
int ping_server();
unsigned int CRC_check(unsigned char *Data,unsigned char Data_length);
int CaculateWeekDay(int y,int m, int d);
void set_net_interface();
void sync_server(int resend,int set_local);
void ask_interface();
void get_ip();
void get_uuid();
void get_net_interface();
void save_to_file(char *date,char *message);

#endif
