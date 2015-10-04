#! /bin/bash -e
arm-linux-gcc cap.c weblib.c json/cJSON.c -Ijson -o /tftpboot/cap -lm
#arm-linux-gcc web.c weblib.c json/cJSON.c -Ijson -o /tftpboot/web -lm
chmod 777 /tftpboot/cap
#chmod 777 /tftpboot/web

