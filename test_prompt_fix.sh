#!/bin/bash

echo "=== 测试提示符修复 ==="
echo "这个测试将验证聊天室是否正确显示消息格式"
echo "- 自己的消息应该显示为 '[时间] 本人: 消息'"
echo "- 其他人的消息应该显示为 '[时间] 用户名: 消息'"
echo "- 不应该在消息前看到多余的 '请输入消息>>' 提示符"
echo ""

# 检查服务器是否运行
if ! pgrep -f "./chat_server" > /dev/null; then
    echo "启动聊天服务器..."
    ./chat_server &
    SERVER_PID=$!
    sleep 2
    echo "服务器已启动 (PID: $SERVER_PID)"
else
    echo "聊天服务器已在运行"
    SERVER_PID=$(pgrep -f "./chat_server")
fi

echo ""
echo "现在可以启动客户端进行测试："
echo "1. 运行 './chat_client' 启动第一个客户端"
echo "2. 使用 admin/Admin123 登录"
echo "3. 在另一个终端运行第二个客户端"
echo "4. 观察消息格式是否正确"
echo ""
echo "按 Ctrl+C 停止测试"

# 等待用户中断
trap "echo ''; echo '正在停止服务器...'; kill $SERVER_PID 2>/dev/null; echo '测试结束'; exit 0" SIGINT

echo "服务器正在运行，等待客户端连接..."
wait
