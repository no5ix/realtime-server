#!/bin/bash

PORT=$1

if ! [[ "${PORT}" =~ ^[0-9]+$ ]] ;
then
    # printf "error: '${PORT}' is not a number.\n\nUsage killport <port number>\n"
    # exit 1
    PORT=44444
fi

PID=$(lsof -ti:${PORT})

if ! [[ "$PID" =~ ^[0-9]+$ ]] ;
then
    printf "[`date '+%x %X'`] No server exist.\n"
    exit 0
fi

# printf "killing process $PID running on ${PORT}\n"
echo "[`date '+%x %X'`] A living server is running, kill the server Now."
kill -9 $PID