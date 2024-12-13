#!/bin/bash

# 设置静态Web服务器的路径
EXPECTED_DIR="/home/tiandeng/inet/build"
PID_FILE="/tmp/static_web_server.pid"
SERVER_PATH="$EXPECTED_DIR/StaticWebServer"

# 切换到指定目录
cd ${EXPECTED_DIR} || { echo "Failed to change directory to $EXPECTED_DIR"; exit 1; }

start() {
    if [ -f "$PID_FILE" ]; then
        echo "Server is already running with PID $(cat $PID_FILE)."
    else
        echo "Starting StaticWebServer..."
        nohup $SERVER_PATH > /dev/null 2>&1 &
        echo $! > $PID_FILE
        echo "Server started with PID $(cat $PID_FILE)."
    fi
}

stop() {
    if [ -f "$PID_FILE" ]; then
        PID=$(cat $PID_FILE)
        echo "Stopping StaticWebServer with PID $PID..."
        kill $PID
        rm $PID_FILE
        echo "Server stopped."
    else
        echo "No running server found."
    fi
}

restart() {
    stop
    start
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        restart
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac
