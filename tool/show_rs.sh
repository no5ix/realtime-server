#!/bin/bash


# if [ $# -lt 1 ]; then
#     lsof -i:44444 ;
# else
#     lsof -i:$1
# fi

# lsof -c rs | head -1 && lsof -c rs | grep -w "IPv4"
# netstat -anlp | grep 'rs'

# top -p `lsof -i:44444 | grep rs | awk '{print $2}'` ;


PORT=$1

if ! [[ "${PORT}" =~ ^[0-9]+$ ]] ;
then
    # printf "error: '${PORT}' is not a number.\n\nUsage killport <port number>\n"
    # exit 1
    PORT=44444
fi

lsof -i:${PORT}

exit 0
