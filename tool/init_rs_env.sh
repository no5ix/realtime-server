#!/bin/sh


chmod u+x build_rs.sh;
chmod u+x kill_rs.sh;
chmod u+x run_rs.sh;
chmod u+x show_rs.sh;
chmod u+x init_rs_env.sh;

BUILD_DIR=../build

if [ ! -d ${BUILD_DIR} ]; then
    mkdir ${BUILD_DIR}
    echo "BUILD_DIR         is created successfully."
else
    echo "BUILD_DIR         is already exist."
fi


RS_PATH=`cd .. && pwd`
RS_SYMBOLIC_LINK=~/rs

if [ ! -L "$RS_SYMBOLIC_LINK" ]; then
    ln -s $RS_PATH $RS_SYMBOLIC_LINK;
    echo "RS_SYMBOLIC_LINK is created successfully."
else
    echo "RS_SYMBOLIC_LINK is already exist."
fi


RS_TOOL_PATH=`pwd`
RS_TOOL_SYMBOLIC_LINK=~/tool_rs

if [ ! -L "$RS_TOOL_SYMBOLIC_LINK" ]; then
    ln -s $RS_TOOL_PATH $RS_TOOL_SYMBOLIC_LINK;
    echo "RS_TOOL_SYMBOLIC_LINK is created successfully."
else
    echo "RS_TOOL_SYMBOLIC_LINK is already exist."
fi


exit 0