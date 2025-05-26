#!/bin/bash

# 聊天客户端启动脚本

echo "===== 聊天客户端启动工具 ====="

if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    echo "用法: $0 [服务器IP地址]"
    echo "示例:"
    echo "  $0              # 使用默认IP (127.0.0.1)"
    echo "  $0 192.168.1.5  # 连接到指定IP"
    echo ""
    echo "您也可以直接使用客户端程序:"
    echo "  ./chat_client           # 使用默认IP"
    echo "  ./chat_client 10.0.0.1  # 连接到指定IP"
    exit 0
fi

# 如果没有提供IP地址，使用默认的127.0.0.1
if [ -z "$1" ]; then
    echo "启动客户端，连接到默认服务器 (127.0.0.1)..."
    ./chat_client
else
    echo "启动客户端，连接到服务器 $1..."
    ./chat_client "$1"
fi
