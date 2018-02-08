CC = arm-none-linux-gnueabi-gcc
CFLAGS = -g -Wall -lm -s -lpthread -L ./curl-imx280/lib -lcurl
#CFLAGS += -DNO_LOG

INCLUDE_PATH = -I ./ -I ./curl-imx280/include
all : debug release

debug: ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c ./curl.c
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o ../cap-imx280 ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c ./curl.c
release: ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c ./curl.c
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -DNO_LOG -o ../cap-imx280-no-log ./cJSON.c ./dwin.c ./history.c ./misc.c ./netlib.c ./cap.c ./xfer.c ./main.c ./log.c ./curl.c
clean:
	$(RM) *.o cap-imx280*
