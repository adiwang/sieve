#!/bin/bash  
#chkconfig:2345 80 05 --指定在哪几个级别执行，0一般指关机，6指的是重启，其他为正常启动。80为启动的优先级，05为关闭的优先级别  
#description:simple example service  
#processname: casd
RETVAL=0  
ROOT_DIR=/home/hzfy/dist


init()
{
    source /opt/pylon5/bin/pylon-setup-env.sh /opt/pylon5
    source /home/hzfy/.profile_halcon
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/home/hzfy/workspace/cpp/sieve/lib
}

startp()
{
    echo "starting ${1}..."
    $ROOT_DIR/${1}/${1} $ROOT_DIR/conf/${1}.conf >> $ROOT_DIR/logs/${1}.out 2>&1 &
    if [ $? -eq 0 ]
    then
        echo "${1} start complete."
    else
        echo "${1} start failed."
    fi
}

start()
{ 
    #启动服务的入口函数  
    init
    echo "sieve service is started..."  
    startp casd
    sleep 5
    startp cpsd
    cur_path=`pwd`
    cd $ROOT_DIR/images
    echo "starting http server"
    python -m SimpleHTTPServer >> $ROOT_DIR/logs/http.out 2>&1 &
    echo "http server start complete."
    cd ${cur_path}
    echo "all sieve process started."
}  

stopp()
{
    echo "stopping ${1} ..."
    pid=`ps aux | grep ${1} | grep -v grep | grep -v vim | grep -v tail | awk '{print $2}'`
    if [ -z ${pid} ]
    then
        echo "process ${1} not exists."
    else
        kill ${pid}
        echo "${1} stopped."
    fi
}
  
stop()
{ 
    #关闭服务的入口函数  
    echo "sieve service is stoped..."  
    stopp SimpleHTTPServer
    stopp cpsd 
    stopp casd
}  
  
#使用case选择  
case $1 in  
    start)  
        start  
        ;;  
    stop)  
        stop  
        ;;  
    restart)
        stop;
        start;
        ;;
    *)  
        echo "error choice ! please input start or stop"
        RETVAL=1
        ;;  
esac  
exit $RETVAL 

