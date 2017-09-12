#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <sys/epoll.h>
#define MAXEVENTS 64
int fd_com = 0;
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0)
	{
		printf("SetupSerial 1\n");
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
		printf("com set error");
		return -1;
	}
	return 0;
}
int open_com_port(char *dev)
{
	int fd = -1;
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
		printf("Can't Open Serial %s\n", dev);
		return(-1);
	}

	if(fcntl(fd, F_SETFL, FNDELAY)<0)
		printf("fcntl failed!\n");
	if(isatty(STDIN_FILENO)==0)
		printf("standard input is not a terminal device\n");
	return fd;
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
	int n;
	n = epoll_wait (efd, events, MAXEVENTS, 5000);
	if(n > 0) {
		for (i=0; i<n; i++) {
			if (events[i].data.fd == fd &&
					(events[i].events & EPOLLIN)) {
				int length = read(events[i].data.fd, buff, sizeof(buff));

				if(length > 0) {
					printf("read %d bytes\n",length);
					for(j=0; j<length; j++) {
						printf("0x%2x ", buff[j]);
					}
					printf("\n");
					if (length == 9)
					{
						printf("light value %d\n", buff[3]<<24|buff[4]<<16|
								buff[5]<<8|buff[6]);
					}
				}
				break;
			}
		}
	} else {
		printf("No data whthin 5 seconds.\n");
	}
	free (events);
}
int main(int argc, char *argv[])
{
	uint8_t cmd[] = {0x01,0x03,0x00,0x07,0x00,0x02,0x75,0xca};
	int i=0;	
	if (argc !=3)
	{
		printf("./test 10 1\n");
		return 0;
	}
	if((fd_com=open_com_port("/dev/ttyUSB0"))<0)
	{
		return -1;
	}
	if (fd_com != -1) {
		if(set_opt(fd_com,9600,8,'N',1)<0)
		{
			printf("set_opt cap error");
			return -1;
		}

		for (i=0; i<atoi(argv[1]); i++)
		{
			write(fd_com, cmd, sizeof(cmd));
			cap_serial(fd_com);
			sleep(atoi(argv[2]));
		}
	}
}
