#ifndef _HISTORY_H
#define _HISTORY_H
#include <sys/shm.h>
#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>
void load_history(const char *name);
void manul_reloading(char *b_year, char *b_mon, char *b_day, char *b_hour, char *b_min,
	char *e_year, char *e_mon, char *e_day, char *e_hour, char *e_min);
#endif
