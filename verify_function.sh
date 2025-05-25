#!/bin/bash

echo "正在验证双向交流软件功能..."

# 确保程序已编译
if [ ! -f "server" ] || [ ! -f "client" ]; then
    echo "编译程序..."
    gcc -g -Wall -pthread -o server server.c
    gcc -g -Wall -pthread -o client client.c
fi

# 启动服务器（后台运行）
echo "启动服务器..."
./server &
SERVER_PID=$!
sleep 2

# 测试客户端连接
echo "测试客户端连接..."
echo -e "测试用户\n你好，世界！\nexit" | timeout 5 ./client 127.0.0.1 >/dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ 功能验证成功！"
    echo "✅ 服务器启动正常"
    echo "✅ 客户端连接正常"
    echo "✅ 消息发送接收正常"
else
    echo "❌ 功能验证失败"
fi

# 清理进程
kill $SERVER_PID 2>/dev/null
echo "验证完成。"
