#!/bin/sh

# lsof -i:44444 | grep rs | awk '{print $2}' | xargs kill -9

SRV_NAME=rs_example_for_ue4_demo

CUR_DIR=`pwd`
BIN_PATH="${CUR_DIR}/../build/bin/${SRV_NAME}"

./kill_rs.sh

echo "[`date '+%x %X'`] Start a new server."

${BIN_PATH} $*;

sleep 1
# netstat -anlp | grep ':44444'
# netstat -anlp | grep -w ${SRV_NAME}
# lsof -c ${SRV_NAME} | head -1 && lsof -c ${SRV_NAME} | grep -w "IPv4"

./show_rs.sh


exit 0
