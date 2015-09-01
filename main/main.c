#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <errno.h>  
#include <sys/msg.h>  
#include <signal.h>
#define LOG_PREFX						"[MainCtlSystem]:"
struct msg_st  
{  
	long int msg_type; 
	int id;
	char text[512];  
}; 

int main(int argc, char *argv[])
{

	struct msg_st data;
	int msgid = -1;

	if(argv[1]!=NULL)
	{
		if(atoi(argv[1])==0)
		{
			if(argv[2]==NULL||argv[3]==NULL||argv[4]==NULL)
			{
				fprintf(stderr, "msg \n[0 send msg,1 rcv msg]\n[msg_type 0x03,0x04]\n[id 0x0d,0x0e]\n[text xxx;yyy]\n");  
				return 0;
			}
		}
		else
		{
			if(argv[2]==NULL||argv[3]==NULL)
			{
				fprintf(stderr, "msg \n[0 send msg,1 rcv msg]\n[msg_type 0x03,0x04]\n[id 0x0d,0x0e]\n");  
				return 0;
			}
		}
	}
	else
	{
		fprintf(stderr, "msg \n[0 send msg,1 rcv msg]\n[msg_type 0x03,0x04]\n[id 0x0d,0x0e]\n[text xxx;yyy]\n");  
		return 0;
	}
	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);  
	if(msgid == -1)  
	{  
		fprintf(stderr, "msgget failed with error: %s\n", strerror(errno));  
		exit(-1);  
	}
	if(atoi(argv[1])==2)
	{
		msgctl(msgid,IPC_RMID,NULL);
		return 0;
	}
	printf(LOG_PREFX"msgid %d\n",msgid);
	if(atoi(argv[1])==0)
	{
		data.msg_type = atoi(argv[2]);
		data.id=atoi(argv[3]);
		memset(data.text,'\0',512);
		printf(LOG_PREFX"send msg\n");
		printf(LOG_PREFX"MSG_TYPE %ld\n",data.msg_type);
		printf(LOG_PREFX"MSG_ID %d\n",data.id);
		memcpy(data.text,argv[4],strlen(argv[4]));
		printf(LOG_PREFX"MSG_TEXT %s\n",data.text);
		if(msgsnd(msgid, (void*)&data, sizeof(struct msg_st)-sizeof(long int), IPC_NOWAIT) == -1)  
		{  
			fprintf(stderr, LOG_PREFX"msgsnd failed %s\n",strerror(errno));
			system("ipcs -q");
		}
		printf(LOG_PREFX"send msg done\n");
	}
	else
	{
		if(msgrcv(msgid, (void*)&data, sizeof(struct msg_st)-sizeof(long int), atoi(argv[2]) , IPC_NOWAIT)>=0)
		{
			if(data.id!=atoi(argv[3]))
				printf(LOG_PREFX"not our msg %d\n",data.id);
			else
				printf(LOG_PREFX"got msg id %d\n",data.id);

			printf(LOG_PREFX"got msg text %s\n",data.text);
		}
		else
		{
			printf(LOG_PREFX"no msg got for %d %s\n",atoi(argv[2]),strerror(errno));
		}		
	}
	return 0;
}

