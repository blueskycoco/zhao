#ifndef _XFER_H
#define _XFER_H
void send_web_post(char *url,char *buf,int timeout,char **out);
int xfer_init();
#endif
