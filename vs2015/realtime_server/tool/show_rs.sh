#!/bin/bash


# if [ $# -lt 1 ]; then
#     lsof -i:44444 ;
# else
#     lsof -i:$1
# fi

# lsof -c rs | head -1 && lsof -c rs | grep -w "IPv4"
# netstat -anlp | grep 'rs'

# top -p `lsof -i:44444 | grep rs | awk '{print $2}'` ;

## 两次检测间隔时间 秒
interval=1
PORT=$1

if ! [[ "${PORT}" =~ ^[0-9]+$ ]] ;
then
    printf " Usage show_rs.sh <port number>\n\n"
    PORT=44444
fi

show_server_state()
{
    lsof -i:${PORT}
}

while :
do
    show_server_state
    sleep $interval
    printf "\n"
done
exit 0
