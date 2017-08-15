#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include "log.h"
#include "mymsg.h"
#define STATE_IDLE 	0
#define STATE_6C 	1
#define STATE_AA 	2
#define STATE_MESSAGE_TYPE 3
#define STATE_MESSAGE_LEN 4
#define STATE_MESSAGE 5
#define STATE_CRC 6

#define MAXEVENTS 64
#define SERIAL_TAG "[serial: ] "
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0)
	{
		printfLog(SERIAL_TAG"SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E': 
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':  
			newtio.c_cflag &= ~PARENB;
			break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		printfLog(SERIAL_TAG"com set error");
		return -1;
	}
	return 0;
}

int open_com_port(char *dev)
{
	int fd;
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
		printfLog(SERIAL_TAG"Can't Open Serial %s", dev);
		return(-1);
	}

	if(fcntl(fd, F_SETFL, FNDELAY)<0)
	{
		close(fd);
		fd = -1;
		printfLog(SERIAL_TAG"fcntl failed!\n");
	}

	if(isatty(STDIN_FILENO)==0)
	{
		close(fd);
		fd = -1;
		printfLog(SERIAL_TAG"standard input is not a terminal device\n");
	}
	return fd;
}
int process_serial(char *buf, int len)
{
	char 	ch = 0;
	static char 	state=STATE_IDLE;
	static int 	message_len=0;
	static char 	message[64]={0};
	static int 	i=0;
	int 	j=0,k=0;
	static char 	to_check[64]={0};
	static int 	crc=0;
	static int 	message_type=0;
	while(j < len)
	{
		ch = buf[j++];
		switch (state)
		{
			case STATE_IDLE:
				{
					if(ch==0x6c)
						state=STATE_6C;
				}
				break;
			case STATE_6C:
				{
					if(ch==0xaa||ch==0xa1)
					{
						state=STATE_AA;
						i=0;
					}						
				}
				break;
			case STATE_AA:
				{
					message_type=ch<<8;
					i=0;
					state=STATE_MESSAGE_TYPE;
				}
				break;
			case STATE_MESSAGE_TYPE:
				{
					message_type|=ch;
					state=STATE_MESSAGE_LEN;
				}
				break;
			case STATE_MESSAGE_LEN:
				{
					message_len=ch;
					state=STATE_MESSAGE;
					i=0;
				}
				break;
			case STATE_MESSAGE:
				{
					if(i!=message_len)
					{
						message[i++]=ch;
					}
					else
					{
						state=STATE_CRC;
						crc=ch<<8;
					}	
				}
				break;
			case STATE_CRC:
				{
					crc|=ch;
					for(i=0;i<message_len;i++)
					{
						to_check[5+i]=message[i];
					}
					to_check[0]=0x6c;
					to_check[1]=0xaa;
					to_check[2]=(message_type>>8)&0xff;
					to_check[3]=message_type&0xff;
					to_check[4]=message_len;
					to_check[5+message_len]=(crc>>8)&0xff;
					to_check[5+message_len+1]=crc&0xff;
					/*if(crc!=CRC_check((unsigned char *)to_check,message_len+5))
					{
						printfLog(CAP_PROCESS"CRC error 0x%04X\r\n",
						CRC_check((unsigned char *)to_check,message_len+5));
						for(i=0;i<message_len+5;i++)
							printfLog("0x%02x ",to_check[i]);
						printfLog("\n");
					}
					else*/
					//send_msg(g_share_memory->msgid,0x33,to_check,message_len+7);
					//printfLog(SERIAL_TAG"new cmd :\n");
					for(k=0; k<message_len+7; k++)
						printfLog("%02x ", to_check[k]);
					printfLog("\n");
					if (message_type == 0x0001)
						printfLog("\n");
					i=0;
					state=STATE_IDLE;
				}
				break;
			default:
				{
					i=0;
					state=STATE_IDLE;
				}	
				break;
		}
	}
	return 0;
}
int cap_serial(int fd)
{
	int efd,i,j;
	char buff[1024] = {0};
	struct epoll_event event;
	struct epoll_event *events;
	efd = epoll_create1 (0);
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl (efd, EPOLL_CTL_ADD, fd, &event);
	events = calloc (MAXEVENTS, sizeof(event));
	for(;;) {
		int n;
		n = epoll_wait (efd, events, MAXEVENTS, 5000);
		if(n > 0) {
			for (i=0; i<n; i++) {
				if (events[i].data.fd == fd &&
						(events[i].events & EPOLLIN)) {
					int length = read(events[i].data.fd, buff, sizeof(buff));

					if(length > 0) {
						/*printfLog(SERIAL_TAG"read %d bytes\n",length);
						for(j=0; j<length; j++) {
							printfLog("%x ", buff[j]);
						}
						printfLog("\n");*/
						process_serial(buff,length);
					}
					break;
				}
			}
		} /*else {
			printf("No data whthin 5 seconds.\n");
		}*/
	}
	free (events);
	close (fd);
}

uint32_t serial_init(char *serial, int baud) 
{
	int fd = open_com_port(serial);
	if (fd != -1)
		set_opt(fd, baud, 8, 'N', 1);

	return fd;
}
int main(int argc, void *argv[])
{
	init_log("cur_log");
	int fd = serial_init(argv[1], atoi(argv[2]);
	
	if (fd != -1)
		cap_serial(fd);
	
	return 0;
}
