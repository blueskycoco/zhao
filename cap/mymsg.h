#ifndef _MY_MSG_H
#define _MY_MSG_H
int init_msg(void);
int send_msg(int msgid, unsigned char msg_type, char *text, int len);
int rcv_msg(int msgid, unsigned char msg_type, char *text, int *len);
#endif
