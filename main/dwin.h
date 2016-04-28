#ifndef _DWIN_H
#define _DWIN_H
int lcd_init();
int read_dgus(int addr,char len,char *out);
void set_lcd_time(char *buf);
void clear_buf(int addr,int len);
void write_string(unsigned int addr,char *data,int len);
void write_data(unsigned int Index,int data);
void clear_point();
void switch_pic(unsigned char Index);
void ctl_fan(int on);
#endif
