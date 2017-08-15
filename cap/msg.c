#include <sys/msg.h>
#include "mymsg.h"
#include "log.h"
#define MSG_PROCESS "[Msg: ]"
struct msg_st
{  
	long int msg_type;
	int len;
	char text[512];
};
int send_msg(int msgid, unsigned char msg_type, char *text, int len)
{
	struct msg_st data;
	data.msg_type = msg_type;
	data.len=len;
	memset(data.text,'\0',512);
	if(text!=NULL)
	{
		memcpy(data.text,text,len);
	}
	if((result = msgsnd(msgid, (void*)&data, 
				sizeof(struct msg_st)-sizeof(long int), IPC_NOWAIT)) == -1)
	{
		printfLog(MSG_PROCESS"msgsnd failed %s\n",strerror(errno));
	}
	return result;
}

int rcv_msg(int msgid, unsigned char msg_type, char *text, int *len)
{
	struct msg_st data;
	int result = 0;
	if((result = msgrcv(msgid, (void*)&data, 
					sizeof(struct msg_st)-sizeof(long int), msg_type , 0)) >= 0)
	{
		text = (char *)malloc(data.len);
		memcpy(text, data.text, data.len);
		*len = data.len;
	}
	else
	{
		text = NULL;
		*len = 0;
		printfLog(MSG_PROCESS"msgrcv failed %s\n",strerror(errno));
	}

	return result;
}

int init_msg(void)
{
	int msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	if(msgid == -1)
	{  
		printfLog(MSG_PROCESS"msgget failed with error: %d\n", errno);  
		exit(-1);  
	}
	else
		printfLog(MSG_PROCESS"msgid %d\n",msgid);
	return msgid;
}
