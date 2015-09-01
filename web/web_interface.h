#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <errno.h>  
#include <sys/msg.h>  
#include <signal.h>
#include <fnmatch.h>
#define PERM S_IRUSR|S_IWUSR  
#define TYPE_MAIN_TO_WEB 				0x02
#define TYPE_WEB_TO_MAIN 				0x01
#define MAIN_TO_WEB 					0x03
#define WEB_TO_MAIN 					0x01
#define LOG_PREFX 						"[WebSubSystem]:"
struct msg_st
{  
	long int msg_type; 
	int id;
	char text[512];  
}; 

