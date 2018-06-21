#!/bin/sh 


SRV_NAME=rts

p_id=` ps -ef | grep -w ${SRV_NAME} | grep -v "grep" | awk '{print $2}' `
if [ "$p_id" = "" ]; then
    echo "[`date '+%x %X'`] No ${SRV_NAME} exist."
else
	echo "[`date '+%x %X'`] Kill ${srv} Now."
    kill -9 $p_id
fi


exit 0
