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
void co_flash_alarm();
void read_curr_time(char *out);
int ping_server_by_gprs();
void gprs_state(int state,int pic);
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen);
void show_fan(int on);
void show_audio(int on);
void ask_hw_ver();
void get_alarm_val(char *file_path);
void set_alarm_val(char *file_path);
void show_main_alarm();
#endif
