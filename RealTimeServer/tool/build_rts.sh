#!/bin/sh


CUR_DIR=`pwd`
SOURCE_DIR="${CUR_DIR}/../RealTimeServer"
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
