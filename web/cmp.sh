#! /bin/bash -e
/root/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc cap.c weblib.c json/cJSON.c -Ijson -o /tftpboot/cap-imx280 -lm
arm-linux-gcc cap.c weblib.c json/cJSON.c -Ijson -DS3C2440 -o /tftpboot/cap-s3c2440 -lm
#arm-linux-gcc web.c weblib.c json/cJSON.c -Ijson -o /tftpboot/web -lm
chmod 777 /tftpboot/cap-s3c2440
chmod 777 /tftpboot/cap-imx280
#chmod 777 /tftpboot/web

