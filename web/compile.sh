#! /bin/bash -e
gcc web_interface.c weblib.c json/cJSON.c -Ijson -o web -lm
