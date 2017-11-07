#! /bin/bash

#./run -b 192.168.30.236:9092 -t userevents -p 1

export LD_LIBRARY_PATH=/usr/local/lib
proc="./crm_noah_online"
ulimit -c unlimited

if [[ $1 == "stop" ]]; then
    ps -aux | grep "$proc" | grep -v grep | awk '{print $2}' | xargs kill -9
elif [[ $1 == "restart" ]]; then
    ps -aux | grep "$proc" | grep -v grep | awk '{print $2}' | xargs kill -9
    sleep 2
    nohup $proc 2>&1 > ./log/run_error.log &
elif [[ $1 == "start" ]]; then
    nohup $proc 2>&1 ./log/run_error.log &
else
    echo "please input start|restart|stop?"
fi
