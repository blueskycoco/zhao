CC = /home/irdetogame/dillon/work/kernel/zhao/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc
CFLAGS = -g -Wall -lm -s

INCLUDE_PATH = -I ./

all: ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o cap-imx280 ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c
clean:
	$(RM) *.o cap-imx280
