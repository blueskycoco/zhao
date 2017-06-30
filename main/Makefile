CC = /home/dillon/work/tools/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-none-linux-gnueabi-gcc
CFLAGS = -g -Wall -lm -s -lpthread
#CFLAGS += -DNO_LOG

INCLUDE_PATH = -I ./
all : debug release

debug: ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o ../cap-imx280 ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c
release: ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -DNO_LOG -o ../cap-imx280-no-log ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c
clean:
	$(RM) *.o cap-imx280*
