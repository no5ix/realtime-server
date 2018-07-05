#!/bin/sh

# lsof -i:44444 | grep rs | awk '{print $2}' | xargs kill -9

SRV_NAME=rs_example_for_ue4_demo
SRV_NAME_WITH_HIREDIS=rs_example_for_ue4_demo_with_redis

CUR_DIR=`pwd`
BIN_PATH="${CUR_DIR}/../build/bin/"

if [ ! -e "${BIN_PATH}${SRV_NAME_WITH_HIREDIS}" ]; then
    BIN_PATH="${BIN_PATH}${SRV_NAME}"
else
    BIN_PATH="${BIN_PATH}${SRV_NAME_WITH_HIREDIS}"
fi

./kill_rs.sh

echo "[`date '+%x %X'`] Start a new server."

${BIN_PATH} $*;

sleep 1
# netstat -anlp | grep ':44444'
# netstat -anlp | grep -w ${SRV_NAME}
# lsof -c ${SRV_NAME} | head -1 && lsof -c ${SRV_NAME} | grep -w "IPv4"

./show_rs.sh


exit 0
