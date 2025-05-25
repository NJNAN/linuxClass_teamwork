#!/bin/bash

# 双向交流软件演示脚本
echo "================================"
echo "  双向交流软件 - 演示脚本"
echo "================================"

# 检查是否已编译
if [ ! -f "server" ] || [ ! -f "client" ]; then
    echo "正在编译程序..."
    gcc -g -Wall -pthread -o server server.c
    gcc -g -Wall -pthread -o client client.c
    
    if [ $? -ne 0 ]; then
        echo "编译失败，请检查代码"
        exit 1
    fi
    echo "编译完成！"
    echo ""
fi

echo "程序已准备就绪！"
echo ""
echo "演示步骤："
echo "1. 当前终端将启动服务器"
echo "2. 请打开新终端运行: ./client 127.0.0.1"
echo "3. 可以打开多个客户端进行聊天测试"
echo "4. 在客户端输入用户名后开始聊天"
echo "5. 输入 'exit' 或按 Ctrl+C 退出"
echo ""

echo "现在启动服务器..."
echo "================================"

# 启动服务器
./server
