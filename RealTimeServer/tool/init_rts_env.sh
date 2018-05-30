#!/bin/sh


chmod u+x build_rts.sh;
chmod u+x kill_rts.sh;
chmod u+x run_rts.sh;
chmod u+x show_rts.sh;
chmod u+x init_rts_env.sh;

BUILD_DIR=../build

if [ ! -d ${BUILD_DIR} ]; then
    mkdir ${BUILD_DIR}
else
    echo "BUILD_DIR is already exist."
fi


RTS_PATH=`cd .. && pwd`
RTS_SYMBOLIC_LINK=~/rts

if [ ! -L "$RTS_SYMBOLIC_LINK" ]; then
    ln -s $RTS_PATH $RTS_SYMBOLIC_LINK;
else
    echo "RTS_SYMBOLIC_LINK is already exist."
fi


exit 0