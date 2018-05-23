#!/bin/sh


cd ~/rts/build;
cmake ..;
#make clean
make

lsof -i:44444 | grep rts | awk '{print $2}' | xargs kill -9

./rts

exit 0
