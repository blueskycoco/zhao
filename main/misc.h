#ifndef _MISC_H
#define _MISC_H
void get_sensor_alarm_info();
void save_sensor_alarm_info();
int open_com_port(char *dev);
int set_alarm(int hour,int mintue,int sec);
void set_time(int year,int mon,int day,int hour,int minute,int second);
void dump_curr_time(int fd);
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);

#endif
