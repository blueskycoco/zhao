#define LOG_PREFX						"[CapSubSystem]:"
#define START_BYTE 0x6C
#define CAP_TO_ARM 0xAA
#define ARM_TO_CAP 0xBB
#define RESEND_BYTE	0x02
#define TIME_BYTE	0x01
#define ERROR_BYTE	0xFF
#define URL "http://101.200.182.92:8080/saveData/airmessage/messMgr.do"
char server_time[20]={0};

//*******************************************************************
//
// 名称: CRC_check
// 说明: 
// 功能: 16位CRC校验
// 调用: 无
// 输入: Data 校验数据，Data_length 数据长度
// 返回值: CRC 校验结果
// ***************************************************************************
unsigned int CRC_check(uchar *Data,uchar Data_length)
{
	unsigned int mid=0;
	unsigned char times=0,Data_index=0;
	unsigned int CRC=0xFFFF;
	while(Data_length)
  	{
		CRC=Data[Data_index]^CRC;
		for(times=0;times<8;times++)
		{
			mid=CRC;
			CRC=CRC>>1;
			if(mid & 0x0001)
			{
				CRC=CRC^0xA001;
			}
		}
		Data_index++;
		Data_length--;
  	}
	return CRC;
}
void get_ip(char *ip)
{
	struct hostent *he;
    char hostname[20] = {0};

    gethostname(hostname,sizeof(hostname));
    he = gethostbyname(hostname);
    printf("hostname=%s\n",hostname);
    printf("%s\n",inet_ntoa(*(struct in_addr*)(he->h_addr)));
	strcpy(ip,inet_ntoa(*(struct in_addr*)(he->h_addr)));
	return ;
}
int read_uart(int fd)
{
    int len,fs_sel,i=0,j=0,get_start=0,get_stop=0,message_len;
	char ch[100]={0},ip[20]={0};
	char id[32]={0},char data[32]={0},char date[32]={0},char error[32]={0};
    fd_set fs_read;
   	char *post_message=NULL,*rcv=NULL;
    struct timeval time;
   
    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);
    time.tv_sec = 10;
    time.tv_usec = 0;
    if(select(fd+1,&fs_read,NULL,NULL,&time)
	{
		len=read(fd,&ch,100);
		for(i=0;i<len;i++)
		{
			printf("%02x ",ch[i]);
		}
		printf("\r\n");		
		get_ip(ip);
		post_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
		post_message=add_item(post_message,ID_DEVICE_UID,"230FFEE9981283737D");
		post_message=add_item(post_message,ID_DEVICE_IP_ADDR,ip);
		post_message=add_item(post_message,ID_DEVICE_PORT,"9517");
		i=0;
		while(1)
		{
		   	memset(id,'\0',sizeof(id));
			memset(data,'\0',sizeof(data));
			memset(date,'\0',sizeof(date));
			memset(error,'\0',sizeof(error));
			while(ch[i]!=START_BYTE)
				i++;
			if(i==len)
				return 0;
			if(ch[i]==START_BYTE && ch[i+1]==CAP_TO_ARM)
			{
				if(CRC_check(ch,ch[i+3]+4)==(ch[ch[i+3]+5]<<8|ch[ch[i+3]+6]))
				{
					switch(ch[2])
					{
						case TIME_BYTE:
						{
							sprintf(date,"20%02d-%02d-%02d %02d:%02d",ch[3],ch[4],ch[5],ch[6],ch[7]);
							printf("date is %s\r\n",date);
							post_message=add_item(post_message,ID_DEVICE_CAP_TIME,date);
						}
						break;
						case ERROR_BYTE:
						{
							sprintf(error,"%dth sensor possible error",ch[3]);
							post_message=add_item(post_message,ID_ALERT_CAP_FAILED,error);
						}
						break;
						case RESEND_BYTE:
						{
							write(fd,server_time,13);
						}
						break;
						default:
						{
							/*get cap data*/
							sprintf(id,"%d",ch[2]);
							sprintf(data,"%d%d",ch[3],ch[4]);
							for(j=strlen(data);j>strlen(data)-ch[5];j--)
								data[j+1]=data[j];
							data[j]='.';
							printf("id %s data %s\r\n",id,data);
							post_message=add_item(post_message,id,data);
						}
						break;
					}
				}
				else
					printf("CRC error Get %02x <> Count %02x\r\n",(ch[len-2]<<8|ch[len-1]),CRC_check(ch,len-2));
			}
			else
				printf("wrong info %02x %02x\r\n",ch[0],ch[1]);
			i=ch[i+3]+6;
		}
    }
	j=0;
	for(i=0;i<strlen(post_message);i++)
	{
		if(post_message[i]=='\n'||post_message[i]=='\r'||post_message[i]=='\t')
			j++;
	}
	char *out1=malloc(strlen(post_message)-j+1);
	memset(out1,'\0',strlen(post_message)-j+1);
	j=0;
	for(i=0;i<strlen(post_message);i++)
	{
		if(post_message[i]!='\r'&&post_message[i]!='\n'&&post_message[i]!='\t')		
		{
			out1[j++]=post_message[i];
		}
	}
	rcv=send_web(URL,out1,9);
	free(post_message);
	free(out1);
	if(rcv!=NULL)
	{	
		int len=strlen(rcv);
		printf(LOG_PREFX"<=== %s\n",rcv);
			printf(LOG_PREFX"send ok\n");
			char *starttime=NULL;
			char *tmp=NULL;
			if(atoi(type)==5)
			{
				starttime=doit_data(rcv,(char *)"210");
				tmp=doit_data(rcv,(char *)"211");
				printf("201 %s\r\n",doit(rcv,"201"));
				printf("202 %s\r\n",doit(rcv,"202"));
			}
			else if(atoi(type)==6)
			{
				starttime=doit_data(rcv,(char *)"101");
				tmp=doit_data(rcv,(char *)"102");
			}
			if(starttime!=NULL)
			{
				printf("%s\r\n",starttime);
				free(starttime);
			}
			if(tmp!=NULL)
			{
				printf("%s\r\n",tmp);
				free(tmp);
			}
			result=1;
		free(rcv);
	}
    return len;
}

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) { 
		perror("SetupSerial 1");
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
		perror("com set error");
		return -1;
	}
	printf("set done!\n");
	return 0;
}

int open_com_port()
{
	int fd;
	long  vdisable;
		
	fd = open( "/dev/ttySAC3", O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
		perror("Can't Open Serial Port0");
		return(-1);
	}
	else 
		printf("open tts/0 .....\n");

	if(fcntl(fd, F_SETFL, FNDELAY)<0)
		printf("fcntl failed!\n");
	else
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,FNDELAY));
	if(isatty(STDIN_FILENO)==0)

		printf("standard input is not a terminal device\n");
	else
		printf("isatty success!\n");
	printf("fd-open=%d\n",fd);
	return fd;
}

int main(int argc, char *argv[])
{

	int fd_com=0;
	char rcv_buf[512];

	if((fd_com=open_com_port())<0)
	{
		perror("open_port error");
		return -1;
	}
	if(set_opt(fd_com,9600,8,'N',1)<0)
	{
		perror(" set_opt error");
		return -1;
	}
	while(1)
	{
		read_uart(fd_com);
		Sleep(30);
	}
	return 0;
}

