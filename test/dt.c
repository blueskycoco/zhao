#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include <signal.h>
#include <fnmatch.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <iconv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <linux/rtc.h>
#include <sys/time.h>
int open_com_port(char *dev)
{
	int fd;
	//long  vdisable;
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
		printf("Can't Open Serial %s", dev);
		return(-1);
	}

	if(fcntl(fd, F_SETFL, FNDELAY)<0)
		printf("fcntl failed!\n");
	
	fcntl(fd, F_SETFL,FNDELAY);
	
	if(isatty(STDIN_FILENO)==0)
		printf("standard input is not a terminal device\n");
	return fd;
}
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0)
	{
		printf("SetupSerial 1");
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
int main(int argc, void *argv[])
{
	uint8_t cmd[] = {0xa5,0x5a,0x02,0x80,0xaa};
	uint8_t cmd1[] = {0xff,0x01,0x78,0x40,0x00,0x00,0x00,0x00,0x47};
	uint8_t cmd2[] = {0xff,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
	uint8_t zd[] = {0x01,0x03,0x00,0x00,0x00,0x02,0xc4,0x0b};
	int efd,i,hcho = -1;
	int fd = 0;
	int mode = 0;
	char buff[1024] = {0};
	struct epoll_event event;
	struct epoll_event *events;	

		mode = atoi(argv[1]);

	if (access(argv[2], F_OK) != 0) 
		return hcho;

	if((fd = open_com_port(argv[2]))<0)
	{
		printf("open_port zd error");
		return hcho;
	}
	if(set_opt(fd,9600,8,'N',1)<0)
	{
		printf("set_opt zd error");
		close(fd);
		return hcho;
	}
	efd = epoll_create1 (0);
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl (efd, EPOLL_CTL_ADD, fd, &event);
	events = calloc (64, sizeof(event));
	int n,j,length;
	if (mode==1) {
		//write(fd, cmd1, sizeof(cmd1));		
	}
	while (1) {
		if(mode == 1) {
			sleep(1);
			write(fd, cmd, sizeof(cmd));
		} else if (mode == 0) {
			//write(fd, cmd2, sizeof(cmd2));
		} else if (mode == 2) {
			sleep(1);
			write(fd, zd, sizeof(zd));
		}
		n = epoll_wait (efd, events, 64, 5000);
		if(n > 0) {
			for (i=0; i<n; i++) {
				if (events[i].data.fd == fd &&
						(events[i].events & EPOLLIN)) {
					length += read(events[i].data.fd, buff+length, sizeof(buff));

					if(length >= 9) {
						/*printf("read %d bytes\n",length);
						for(j=0; j<length; j++) {
							printf("0x%02x ", buff[j]);
						}
						printf("\n");*/
						//if (length == 9)
						{
							if (mode == 0) {
								if (buff[0] == 0xff && buff[1] == 0x17 &&
										buff[2] == 0x04) {
								hcho = buff[4] * 256 + buff[5];
								printf("hcho value %f\n", ((float)hcho)/10000.0);
								}
							} else if (mode == 1) {
								if (buff[0] == 0xa5 && buff[1] == 0x5a &&
										buff[2] == 0x06 && buff[3] == 0x80) {
									hcho = buff[4] * 256 + buff[5];
									printf("hcho value %f\n", ((float)hcho)/100.0);
								}
							} else if (mode == 2) {
								hcho = buff[3]<<24|buff[4]<<16| buff[5]<<8|buff[6];
								printf("light value %d\n", hcho);
							}
						}
						length = 0;
					}
					break;
				}
			}
		} //else
			//printf("no data in 5 sec\r\n");
	}
	free (events);
	close(fd);
	close(efd);
}
