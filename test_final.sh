#!/bin/bash

echo "=== 最终测试脚本 ==="
echo "验证聊天室消息格式是否正确："
echo "✓ 用户看到自己的消息格式: [时间] 本人: 消息内容"
echo "✓ 用户看到他人的消息格式: [时间] 用户名: 消息内容"
echo "✓ 输入提示符: 请输入消息>>"
echo ""

# 创建测试输入
cat > test_input1.txt << 'EOF'
1
admin
Admin123
你好，这是测试消息1
这是第二条测试消息
exit
EOF

cat > test_input2.txt << 'EOF'
1
admin
Admin123
这是来自另一个用户的消息
exit
EOF

echo "测试步骤："
echo "1. 启动第一个客户端进行基本测试"
echo "2. 观察消息格式是否正确"
echo ""

echo "开始测试..."
echo "================================="

# 启动客户端进行测试
timeout 10 ./chat_client < test_input1.txt

echo ""
echo "================================="
echo "测试完成！"
echo ""
echo "请手动验证："
echo "- 是否看到 '[时间] 本人: 消息内容' 格式"
echo "- 输入提示符是否为 '请输入消息>>'"

# 清理测试文件
rm -f test_input1.txt test_input2.txt
