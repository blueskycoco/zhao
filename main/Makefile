CC = arm-fsl-linux-gnueabi-gcc
CFLAGS = -g -Wall -lm

INCLUDE_PATH = -I ./

all: ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./uart.c ./xfer.c
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o cap-imx280 ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./uart.c ./xfer.c
clean:
	$(RM) *.o cap-imx280
