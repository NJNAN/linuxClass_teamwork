#!/bin/bash

echo "========================================"
echo "  聊天室客户端 - 提示符修复演示"
echo "========================================"
echo ""
echo "修复内容："
echo "✅ 消息格式：自己的消息显示为 '[时间] 本人: 内容'"
echo "✅ 消息格式：他人的消息显示为 '[时间] 用户名: 内容'"
echo "✅ 清理显示：消息前不会出现多余的 '请输入消息>>' 提示符"
echo "✅ 用户体验：输入提示符只在适当时机显示"
echo ""

# 检查服务器状态
if ! pgrep -f "./chat_server" > /dev/null; then
    echo "🚀 启动聊天服务器..."
    ./chat_server &
    SERVER_PID=$!
    sleep 2
    echo "✅ 服务器已启动 (PID: $SERVER_PID)"
else
    echo "✅ 聊天服务器已在运行"
    SERVER_PID=$(pgrep -f "./chat_server")
fi

echo ""
echo "📋 测试步骤："
echo "1. 运行 './chat_client' 启动客户端"
echo "2. 选择 [1] 登录"
echo "3. 用户名：admin，密码：Admin123"
echo "4. 输入一些测试消息"
echo "5. 在另一个终端启动第二个客户端进行对话测试"
echo ""
echo "🔍 验证要点："
echo "- 自己发送的消息应显示为 '[时间] 本人: 消息内容'"
echo "- 收到的消息应显示为 '[时间] 发送者: 消息内容'"
echo "- 消息前不应该有 '请输入消息>>' 的多余显示"
echo ""
echo "按 Enter 键开始手动测试，或按 Ctrl+C 退出"

read -p ""

echo "🎯 启动第一个客户端进行测试..."
echo "   (提示：可以开启另一个终端运行第二个客户端进行对话测试)"
echo ""

./chat_client
