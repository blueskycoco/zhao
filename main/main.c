#include "cap.h"
#include "history.h"
#include "misc.h"
#include "xfer.h"
#include "dwin.h"
#define MAIN_PROCESS	"[MainCtlSystem] "
struct share_memory *g_share_memory;
struct history sensor_history;
key_t	shmid_history_co;
key_t	shmid_history_co2;
key_t	shmid_history_hcho;
key_t	shmid_history_temp;
key_t	shmid_history_shidu;
key_t	shmid_history_pm25;
key_t	shmid_history_wind;
key_t	shmid_history_noise;
key_t	shmid_history_o3;
key_t	shmid_history_press;
key_t	shmid_history_tvoc;
key_t	shmid_history_pm10;
key_t	shmid_share_memory;
int main(int argc, char *argv[])
{
	int fpid;	
	//long i;
	//key_t shmid;
	signal(SIGCHLD, SIG_IGN);
	init_log();
	if((shmid_history_co = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        printfLog(MAIN_PROCESS"Create co history share Error %s/n/a", strerror(errno));  
        exit(1);
    }
	if((shmid_history_co2 = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        printfLog(MAIN_PROCESS"Create co2 history share Error %s/n/a", strerror(errno));  
        exit(1);
    }
	if((shmid_history_hcho = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        printfLog(MAIN_PROCESS"Create hcho history share Error %s/n/a", strerror(errno));  
        exit(1);
    }
	if((shmid_history_temp = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        printfLog(MAIN_PROCESS"Create temp history share Error %s/n/a", strerror(errno));  
        exit(1);
    }
	if((shmid_history_shidu = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        printfLog(MAIN_PROCESS"Create shidu history share Error %s/n/a", strerror(errno));  
        exit(1);
    }
	if((shmid_history_pm25 = shmget(IPC_PRIVATE, sizeof(struct nano)*100000, PERM)) == -1 )
	{
        printfLog(MAIN_PROCESS"Create pm25 history share Error %s/n/a", strerror(errno));  
        exit(1);
    }
	
	if((shmid_share_memory = shmget(IPC_PRIVATE, sizeof(struct share_memory), PERM)) == -1 )
	{
        printfLog(MAIN_PROCESS"Create Share Memory Error:%s/n/a", strerror(errno));
        exit(1);
    }	
	
	sensor_history.co= (struct nano *)shmat(shmid_history_co,0, 0);
	sensor_history.co2 = (struct nano *)shmat(shmid_history_co2,0, 0);
	sensor_history.hcho= (struct nano *)shmat(shmid_history_hcho,0, 0);
	sensor_history.temp= (struct nano *)shmat(shmid_history_temp,0, 0);
	sensor_history.shidu= (struct nano *)shmat(shmid_history_shidu,0, 0);
	sensor_history.pm25= (struct nano *)shmat(shmid_history_pm25,0, 0); 	
	sensor_history.pm10= (struct nano *)shmat(shmid_history_co,0, 0);
	sensor_history.noise = (struct nano *)shmat(shmid_history_co2,0, 0);
	sensor_history.press= (struct nano *)shmat(shmid_history_hcho,0, 0);
	sensor_history.tvoc= (struct nano *)shmat(shmid_history_temp,0, 0);
	sensor_history.o3= (struct nano *)shmat(shmid_history_shidu,0, 0);
	sensor_history.wind= (struct nano *)shmat(shmid_history_pm25,0, 0);
	g_share_memory		= (struct share_memory *)shmat(shmid_share_memory,	 0, 0);	
	g_share_memory->factory_mode=NORMAL_MODE;
	g_share_memory->history_done=0;
	g_share_memory->ppm=0;
	get_ip();
	get_sensor_alarm_info();
	get_uuid();
	get_net_interface();
	if((g_share_memory->fd_com=open_com_port("/dev/ttySP0"))<0)
	{
		printfLog(MAIN_PROCESS"open_port cap error");
		return -1;
	}
	if(set_opt(g_share_memory->fd_com,9600,8,'N',1)<0)
	{
		printfLog(MAIN_PROCESS"set_opt cap error");
		return -1;
	}
	if((g_share_memory->fd_gprs=open_com_port("/dev/ttySP2"))<0)
	{
		printfLog(MAIN_PROCESS"open_port gprs error");
		return -1;
	}
	if(set_opt(g_share_memory->fd_gprs,115200,8,'N',1)<0)
	{
		printfLog(MAIN_PROCESS"set_opt gprs error");
		close(g_share_memory->fd_gprs);
		return -1;
	}
	if((g_share_memory->fd_lcd=open_com_port("/dev/ttySP1"))<0)
	{
		printfLog(MAIN_PROCESS"open_port lcd error");
		return -1;
	}
	if(set_opt(g_share_memory->fd_lcd,115200,8,'N',1)<0)
	{
		printfLog(MAIN_PROCESS" set_opt lcd error");
		close(g_share_memory->fd_lcd);
		return -1;
	}

	xfer_init();
	if((fpid=fork())==0)
	{
		load_history(HISTORY_PATH);
		return 0;
	}
	else
		printfLog(MAIN_PROCESS"[PID]%d load history process\n",fpid);
	lcd_init();//process lcd
	cap_init();//process cap
	while(1)
	{
		//sync server with resend data 00:01 every night
		sync_server(0,1);
		if(g_share_memory->server_time[0]!=0 &&g_share_memory->server_time[5]!=0)
		{
			set_alarm(00,00,01);
			sync_server(1,1);
		}
		else
			sleep(10);
	}
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
	#if 0
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
	#endif
	return 0;
}

