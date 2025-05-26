#!/bin/bash

# 聊天室启动脚本

echo "=== 聊天室系统启动脚本 ==="
echo "1. 启动服务端"
echo "2. 启动客户端"
echo "3. 同时启动服务端和客户端（推荐用于测试）"
echo "4. 编译所有程序"
echo "5. 清理编译文件"
echo "6. 退出"
echo

read -p "请选择操作 (1-6): " choice

case $choice in
    1)
        echo "启动服务端..."
        make run-server
        ;;
    2)
        echo "启动客户端..."
        make run-client
        ;;
    3)
        echo "同时启动服务端和客户端..."
        echo "服务端将在后台运行，客户端在前台运行"
        echo "使用 Ctrl+C 结束客户端，然后使用 'pkill chat_server' 结束服务端"
        echo
        ./chat_server &
        SERVER_PID=$!
        echo "服务端已启动 (PID: $SERVER_PID)"
        sleep 2
        echo "正在启动客户端..."
        ./chat_client
        echo "正在停止服务端..."
        kill $SERVER_PID 2>/dev/null
        ;;
    4)
        echo "编译所有程序..."
        make all
        echo "编译完成！"
        ;;
    5)
        echo "清理编译文件..."
        make clean
        echo "清理完成！"
        ;;
    6)
        echo "再见！"
        exit 0
        ;;
    *)
        echo "无效选择，请重试"
        ;;
esac
