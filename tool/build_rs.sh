#!/bin/sh

chmod u+x build_rs.sh;
chmod u+x kill_rs.sh;
chmod u+x run_rs.sh;
chmod u+x show_rs.sh;
chmod u+x init_rs_env.sh;



RS_DIR=`cd .. && pwd`
RS_SYMBOLIC_LINK=~/rs

if [ ! -L "$RS_SYMBOLIC_LINK" ]; then
    ln -s $RS_DIR $RS_SYMBOLIC_LINK;
    echo "RS_SYMBOLIC_LINK is created successfully."
else
    echo "RS_SYMBOLIC_LINK is already exist."
fi


RS_BIN_DIR=../build/bin
RS_BIN_SYMBOLIC_LINK=rs_bin

if [ ! -L "$RS_BIN_SYMBOLIC_LINK" ]; then
    ln -s $RS_BIN_DIR $RS_BIN_SYMBOLIC_LINK;
    echo "RS_BIN_SYMBOLIC_LINK is created successfully."
else
    echo "RS_BIN_SYMBOLIC_LINK is already exist."
fi


# cmake

CUR_DIR=`pwd`
SOURCE_DIR="${CUR_DIR}/../"
BUILD_DIR="${CUR_DIR}/../build"

if [ ! -d ${BUILD_DIR} ]; then
  mkdir ${BUILD_DIR}
fi

if [ -x ${BUILD_DIR} ]; then
    cd ${BUILD_DIR} \
        && cmake ${SOURCE_DIR} \
        && make $*
fi

exit 0
