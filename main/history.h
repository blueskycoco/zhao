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
extern key_t shmid_co,shmid_co2,shmid_hcho,shmid_temp,shmid_pm25,shmid_shidu;
extern key_t shmid_co_cnt,shmid_co2_cnt,shmid_hcho_cnt,shmid_temp_cnt,shmid_pm25_cnt,shmid_shidu_cnt;
extern key_t history_load_done_shmid;
extern struct nano *g_history_co,*g_history_temp,*g_history_shidu,*g_history_co2,*g_history_hcho,*g_history_pm25;
extern long *g_co_cnt,*g_co2_cnt,*g_hcho_cnt,*g_temp_cnt,*g_pm25_cnt,*g_shidu_cnt;
extern int *history_done;
void load_history(const char *name);
#endif
