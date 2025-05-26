#!/bin/bash

# 聊天室自动测试脚本

echo "=== 聊天室自动测试 ==="
echo "这个脚本将启动服务端并模拟客户端连接"
echo

# 编译程序
echo "1. 编译程序..."
make all
if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi

echo "2. 启动服务端..."
./chat_server &
SERVER_PID=$!
echo "服务端已启动 (PID: $SERVER_PID)"

# 等待服务端启动
sleep 2

echo "3. 服务端运行中，您现在可以："
echo "   - 运行 './chat_client' 来启动客户端"
echo "   - 或者运行 './start.sh' 选择选项2"
echo "   - 使用 Ctrl+C 结束这个测试，服务端会自动停止"
echo

# 等待用户中断
trap "echo '正在停止服务端...'; kill $SERVER_PID 2>/dev/null; echo '测试结束'; exit 0" INT

echo "按 Ctrl+C 结束测试..."
while true; do
    sleep 1
done
