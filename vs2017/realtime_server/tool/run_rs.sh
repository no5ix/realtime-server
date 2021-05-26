#!/bin/sh

# lsof -i:44444 | grep rs | awk '{print $2}' | xargs kill -9

EXAMPLE_NAME=rs_example_for_ue4_demo
EXAMPLE_NAME_PLUS=rs_example_for_ue4_demo_plus


CUR_DIR=`pwd`
BIN_PATH="${CUR_DIR}/../build/bin/"

if [ ! -e "${BIN_PATH}${EXAMPLE_NAME_PLUS}" ]; then
    SRV_NAME="${EXAMPLE_NAME}"
else
    SRV_NAME="${EXAMPLE_NAME_PLUS}"
fi

./kill_rs.sh

echo "[`date '+%x %X'`] Start a new server."

cd ${BIN_PATH};
chmod +x ${SRV_NAME}
./${SRV_NAME} $*;
cd -;

# sleep 1
# netstat -anlp | grep ':44444'
# netstat -anlp | grep -w ${SRV_NAME}
# lsof -c ${SRV_NAME} | head -1 && lsof -c ${SRV_NAME} | grep -w "IPv4"

# ./show_rs.sh


exit 0
