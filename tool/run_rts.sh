#!/bin/sh

# lsof -i:44444 | grep rts | awk '{print $2}' | xargs kill -9

BIN_PATH=~/rts/build/bin/rts
SRV_NAME=rts

p_id=` ps -ef | grep -w ${SRV_NAME} | grep -v "grep" | awk '{print $2}' `
if [ "$p_id" = "" ]; then
    echo "[`date '+%x %X'`] No living ${SRV_NAME} to kill, so just run."
else
	echo "[`date '+%x %X'`] A living ${SRV_NAME} is running, so kill it then run a new one."
    kill -9 $p_id
fi

${BIN_PATH} $*;

sleep 1
# netstat -anlp | grep ':44444'
# netstat -anlp | grep -w ${SRV_NAME}
lsof -c rts | head -1 && lsof -c rts | grep -w "IPv4"



exit 0
