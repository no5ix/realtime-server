#!/bin/sh 

lsof -i:44444 ;

top -p `lsof -i:44444 | grep rts | awk '{print $2}'` ;

exit 0
