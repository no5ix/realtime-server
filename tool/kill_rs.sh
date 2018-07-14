#!/bin/bash

PORT=$1

if ! [[ "${PORT}" =~ ^[0-9]+$ ]] ;
then
    # printf "error: '${PORT}' is not a number.\n\nUsage killport <port number>\n"
    # exit 1
    PORT=44444
fi


PIDs=$(lsof -ti:${PORT} | xargs)
printf "$PIDs \n"

if ! [[ "$PIDs" =~ ^[0-9]+$ ]] ;
then
    printf "[`date '+%x %X'`] No server exist.\n"
    exit 1
fi

echo "[`date '+%x %X'`] Some living server is running, kill the server Now."
kill -9 $PIDs
