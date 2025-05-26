#!/bin/bash

# 聊天室功能演示脚本

echo "=== 聊天室系统演示 ==="
echo "这个演示将展示聊天室的主要功能"
echo

# 检查程序是否已编译
if [ ! -f "chat_server" ] || [ ! -f "chat_client" ]; then
    echo "程序未编译，正在编译..."
    make all
    if [ $? -ne 0 ]; then
        echo "编译失败！请检查代码。"
        exit 1
    fi
    echo "编译完成！"
    echo
fi

echo "功能特性："
echo "✓ 用户注册和登录认证"
echo "✓ 多用户实时聊天"
echo "✓ 消息格式：[时间] 用户名: 内容"
echo "✓ 自动换行（每行最多80字符）"
echo "✓ 历史消息记录"
echo "✓ 发送者不会看到自己消息的回显"
echo "✓ 优雅的输入体验"
echo

echo "默认管理员账户："
echo "用户名: admin"
echo "密码: Admin123"
echo

echo "启动演示："
echo "1. 服务端将在后台启动"
echo "2. 客户端将在前台运行"
echo "3. 您可以使用默认账户登录"
echo "4. 尝试发送一些消息"
echo "5. 使用 Ctrl+C 退出"
echo

read -p "按 Enter 开始演示..." dummy

# 启动服务端
echo "启动服务端..."
./chat_server &
SERVER_PID=$!
echo "服务端已启动 (PID: $SERVER_PID)"

# 等待服务端启动
sleep 2

echo "启动客户端..."
echo "请使用以下账户登录："
echo "用户名: admin"
echo "密码: Admin123"
echo

# 设置陷阱以清理服务端
trap "echo ''; echo '正在停止服务端...'; kill $SERVER_PID 2>/dev/null; echo '演示结束'; exit 0" INT

# 启动客户端
./chat_client

# 演示结束后清理
echo "正在停止服务端..."
kill $SERVER_PID 2>/dev/null
echo "演示结束"
