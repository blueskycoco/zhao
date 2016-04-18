#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <errno.h>  
#include <sys/msg.h>  
#include <signal.h>
#include "log.h"
#define MAIN_PROCESS	"[MainCtlSystem]:"
extern struct share_memory *g_share_memory;
extern struct history *sensor_history;
extern key_t  shmid_history;
extern key_t  shmid_share_memory;

int main(int argc, char *argv[])
{
	int fpid;	
	long i;
	key_t shmid;
	signal(SIGCHLD, SIG_IGN);
	
	if((shmid_history = shmget(IPC_PRIVATE, sizeof(struct history)*100000, PERM)) == -1 )
	{
        printfLog(CAP_PROCESS"Create history share Error %s/n/a", strerror(errno));  
        exit(1);
    }
	if((shmid_share_memory = shmget(IPC_PRIVATE, sizeof(struct share_memory), PERM)) == -1 )
	{
        printfLog(CAP_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));
        exit(1);
    }	
	if((fpid=fork())==0)
	{
		g_share_memory = (struct share_memory *)shmat(shmid_share_memory,0,0);
		load_history("/home/user/history");
		return 0;
	}
	else
		printf("[PID]%d load history process\n",fpid);
	get_ip(ip);
	if((shmid = shmget(IPC_PRIVATE, 256, PERM)) == -1 )
	{
		fprintf(stderr, "Create Share Memory Error:%s/n/a", strerror(errno));  
		exit(1);  
	}
	get_sensor_alarm_info();
	server_time = (char *)shmat(shmid, 0, 0);
#ifdef S3C2440
	if((fd_com=open_com_port("/dev/s3c2410_serial1"))<0)
#else
	if((fd_com=open_com_port("/dev/ttySP0"))<0)
#endif
	{
		perror("open_port cap error");
		return -1;
	}
	if(set_opt(fd_com,9600,8,'N',1)<0)
	{
		perror(" set_opt cap error");
		return -1;
	}
	if((fd_lcd=open_com_port("/dev/ttySP1"))<0)
	{
		perror("open_port lcd error");
		close(fd_com);
		return -1;
	}
	if(set_opt(fd_lcd,115200,8,'N',1)<0)
	{
		perror(" set_opt lcd error");
		close(fd_com);
		close(fd_lcd);
		return -1;
	}
	if((fd_gprs=open_com_port("/dev/ttySP2"))<0)
	{
		perror("open_port gprs error");
		close(fd_com);
		close(fd_lcd);
		return -1;
	}
	if(set_opt(fd_gprs,115200,8,'N',1)<0)
	{
		perror(" set_opt gprs error");
		close(fd_com);
		close(fd_lcd);
		close(fd_gprs);
		return -1;
	}
	//config_gprs();
	pthread_mutex_init(&mutex, NULL);
	memset(server_time,0,13);
	get_uuid();
	send_by_wifi = (char *)shmat(send_by_wifi_shmid, 0, 0);
	get_net_interface();
	#if 0
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,1);
	sleep(3);
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,0);
	sleep(3);
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,1);
	sleep(3);
	buzzer(fd_lcd,0x30);
	cut_pic(fd_lcd,0);
	#endif
	cap_init();
	fpid=fork();
	if(fpid==0)
	{
		printf(LCD_PROCESS"begin to shmat1\n");
		server_time = (char *)shmat(shmid, 0, 0);
		send_by_wifi = (char *)shmat(send_by_wifi_shmid, 0, 0);
		g_state = (struct state *)shmat(state_shmid, 0, 0);
		g_history_co = (struct nano *)shmat(shmid_co, 0, 0);
		g_history_co2 = (struct nano *)shmat(shmid_co2, 0, 0);
		g_history_hcho = (struct nano *)shmat(shmid_hcho, 0, 0);
		g_history_shidu = (struct nano *)shmat(shmid_shidu, 0, 0);
		g_history_temp = (struct nano *)shmat(shmid_temp, 0, 0);
		g_history_pm25 = (struct nano *)shmat(shmid_pm25, 0, 0);
		g_co_cnt = (long *)shmat(shmid_co_cnt, 0, 0);
		g_co2_cnt = (long *)shmat(shmid_co2_cnt, 0, 0);
		g_hcho_cnt = (long *)shmat(shmid_hcho_cnt, 0, 0);
		g_temp_cnt = (long *)shmat(shmid_temp_cnt, 0, 0);
		g_shidu_cnt = (long *)shmat(shmid_shidu_cnt, 0, 0);
		g_pm25_cnt = (long *)shmat(shmid_pm25_cnt, 0, 0);
		sensor_interface_mem = (int *)shmat(sensor_interface_shmid, 0, 0);
		factory_mode = (char *)shmat(factory_mode_shmid, 0, 0);
		jiaozhun_sensor = (int *)shmat(jiaozhun_sensor_shmid, 0, 0);
		g_zero_info = (struct cur_zero_info *)shmat(shmid_zero_info, 0, 0);
		g_verify_info = (struct verify_point_info *)shmat(shmid_verify_info, 0, 0);
		printf(LCD_PROCESS"end to shmat\n");
		signal(SIGALRM, set_upload_flag);
		alarm(600);
		while(1)
		{
			get_uart(fd_lcd,fd_com);
		}
	}
	else if(fpid>0)
	{
		printf("[PID]%d cap process\n",fpid);
		fpid=fork();
		if(fpid==0)
		{
			printf(LCD_PROCESS"begin to shmat1\n");
			server_time = (char *)shmat(shmid, 0, 0);
			send_by_wifi = (char *)shmat(send_by_wifi_shmid, 0, 0);
			g_state = (struct state *)shmat(state_shmid, 0, 0);
			g_history_co = (struct nano *)shmat(shmid_co, 0, 0);
			g_history_co2 = (struct nano *)shmat(shmid_co2, 0, 0);
			g_history_hcho = (struct nano *)shmat(shmid_hcho, 0, 0);
			g_history_shidu = (struct nano *)shmat(shmid_shidu, 0, 0);
			g_history_temp = (struct nano *)shmat(shmid_temp, 0, 0);
			g_history_pm25 = (struct nano *)shmat(shmid_pm25, 0, 0);
			g_co_cnt = (long *)shmat(shmid_co_cnt, 0, 0);
			g_co2_cnt = (long *)shmat(shmid_co2_cnt, 0, 0);
			g_hcho_cnt = (long *)shmat(shmid_hcho_cnt, 0, 0);
			g_temp_cnt = (long *)shmat(shmid_temp_cnt, 0, 0);
			g_shidu_cnt = (long *)shmat(shmid_shidu_cnt, 0, 0);
			g_pm25_cnt = (long *)shmat(shmid_pm25_cnt, 0, 0);
			history_done = (int *)shmat(history_load_done_shmid,0,0);
			g_zero_info = (struct cur_zero_info *)shmat(shmid_zero_info, 0, 0);
			sensor_interface_mem = (int *)shmat(sensor_interface_shmid, 0, 0);
			factory_mode = (char *)shmat(factory_mode_shmid, 0, 0);
			jiaozhun_sensor = (int *)shmat(jiaozhun_sensor_shmid, 0, 0);
			g_verify_info = (struct verify_point_info *)shmat(shmid_verify_info, 0, 0);
			sensor_interface_mem[0] = 0x1234;
			printf(LCD_PROCESS"end to shmat\n");
			signal(SIGALRM, lcd_off);
			alarm(300);
			//printf("to get interface\n");
			//def_interface();
			//ask_interface();
			//printf("end get interface\n");
			while(1)
			{
				lcd_loop(fd_lcd);
			}	
		}
		else if(fpid>0)
		{
			printf("[PID]%d lcd process\n",fpid);
			while(1)
			{
				sync_server(fd_com,0,1);
				if(server_time[0]!=0 &&server_time[5]!=0)
				{
					set_alarm(00,00,01);
					sync_server(fd_com,1,1);
				}
				else
					sleep(10);
			}
		}
	}
	return 0;
}

