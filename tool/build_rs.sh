#!/bin/sh

chmod u+x build_rs.sh;
chmod u+x kill_rs.sh;
chmod u+x run_rs.sh;
chmod u+x show_rs.sh;



RS_DIR=`cd .. && pwd`
RS_SYMBOLIC_LINK=~/rs

if [ ! -L "$RS_SYMBOLIC_LINK" ]; then
    ln -s $RS_DIR $RS_SYMBOLIC_LINK;
    echo "RS_SYMBOLIC_LINK is created successfully."
else
    echo "RS_SYMBOLIC_LINK is already exist."
fi


RS_BIN_DIR=`cd ../build/bin && pwd`
RS_BIN_SYMBOLIC_LINK=bin_rs

if [ ! -L "$RS_BIN_SYMBOLIC_LINK" ]; then
    ln -s $RS_BIN_DIR $RS_BIN_SYMBOLIC_LINK;
    echo "RS_BIN_SYMBOLIC_LINK is created successfully."
else
    echo "RS_BIN_SYMBOLIC_LINK is already exist."
fi


# toluapp

echo "toluapping..."

EXAMPLE_CODE_DIR="../example/for_ue4_demo/"
tolua++5.1 -o ${EXAMPLE_CODE_DIR}lua_Character.cpp ${EXAMPLE_CODE_DIR}Character.pkg


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
