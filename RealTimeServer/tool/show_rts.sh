#!/bin/sh 

# lsof -i:44444 ;
lsof -c rts | head -1 && lsof -c rts | grep -w "IPv4"
# netstat -anlp | grep 'rts'

# top -p `lsof -i:44444 | grep rts | awk '{print $2}'` ;

exit 0
