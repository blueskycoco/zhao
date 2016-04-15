#ifndef _HISTORY_H
#define _HISTORY_H
#include <sys/shm.h>
#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>
struct nano{
	char time[20];
	char data[10];
};
void load_history(const char *name);
#endif
