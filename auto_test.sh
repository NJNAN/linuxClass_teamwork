#!/bin/bash

echo "=== 自动化测试聊天客户端提示符修复 ==="
echo "测试目标：验证聊天消息格式正确，无多余提示符"
echo ""

# 创建临时输入文件
cat > test_input.txt << 'EOF'
1
admin
Admin123
hello world
test message
EOF

echo "启动客户端并自动输入测试消息..."

# 使用 expect 来自动化交互（如果没有 expect，使用简单的管道输入）
if command -v expect >/dev/null 2>&1; then
    expect << 'EOF'
spawn ./chat_client
expect "请选择操作:"
send "1\r"
expect "用户名:"
send "admin\r"
expect "密码:"
send "Admin123\r"
expect "请输入消息>>"
send "hello from automated test\r"
sleep 1
send "second message\r"
sleep 1
expect eof
EOF
else
    echo "使用简单管道输入进行测试..."
    timeout 10 ./chat_client < test_input.txt
fi

# 清理临时文件
rm -f test_input.txt

echo ""
echo "测试完成。如果看到上述输出中没有多余的 '请输入消息>>' 出现在消息前面，"
echo "那么修复成功。"
