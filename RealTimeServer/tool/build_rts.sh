#!/bin/sh


cd ~/rts/build;
cmake ../RealTimeServer;
#make clean
make

# lsof -i:44444 | grep rts | awk '{print $2}' | xargs kill -9

# ./rts

# lsof -i:44444

exit 0
