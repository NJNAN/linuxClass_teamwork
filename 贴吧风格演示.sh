#!/bin/bash

# 贴吧风格聊天室演示脚本
echo "=== 贴吧风格聊天室演示 ==="
echo "此演示将展示贴吧风格的聊天界面："
echo "- 时间戳在左上角"
echo "- 所有发言左对齐"
echo "- 楼层显示效果"
echo "- 用框线围绕每条消息"
echo ""

# 创建一个临时目录用于演示
DEMO_DIR="/tmp/chatroom_demo"
mkdir -p $DEMO_DIR

echo "1. 启动服务器..."
cd /home/njnan/class/linux编程实训/teamwork
./server &
SERVER_PID=$!
echo "服务器已启动 (PID: $SERVER_PID)"

sleep 2

echo ""
echo "2. 准备启动两个客户端进行演示..."
echo "第一个客户端将自动发送几条消息"
echo "第二个客户端也会发送消息"
echo ""

# 创建第一个客户端的自动输入
cat > $DEMO_DIR/client1_input.txt << 'EOF'
用户1
大家好！这是贴吧风格的聊天室！
可以看到时间戳在左上角
所有消息都左对齐显示
而且有楼层编号
exit
EOF

# 创建第二个客户端的自动输入
cat > $DEMO_DIR/client2_input.txt << 'EOF'
用户2
哇，这个界面真的很像贴吧！
每条消息都有漂亮的边框
青色是自己的消息，黄色是别人的
非常清晰易读
exit
EOF

echo "3. 启动第一个客户端..."
gnome-terminal --title="贴吧聊天室 - 用户1" -- bash -c "
cd /home/njnan/class/linux编程实训/teamwork
./client 127.0.0.1 < $DEMO_DIR/client1_input.txt
echo '按任意键关闭...'
read -n 1
" &
CLIENT1_PID=$!

sleep 3

echo "4. 启动第二个客户端..."
gnome-terminal --title="贴吧聊天室 - 用户2" -- bash -c "
cd /home/njnan/class/linux编程实训/teamwork
./client 127.0.0.1 < $DEMO_DIR/client2_input.txt
echo '按任意键关闭...'
read -n 1
" &
CLIENT2_PID=$!

echo ""
echo "演示已启动！"
echo "- 两个客户端窗口将自动打开"
echo "- 观察贴吧风格的消息显示格式"
echo "- 注意时间戳、楼层号和边框效果"
echo ""
echo "按Ctrl+C或等待演示结束后清理进程..."

# 等待用户中断或一定时间后清理
trap 'echo "清理进程..."; kill $SERVER_PID $CLIENT1_PID $CLIENT2_PID 2>/dev/null; rm -rf $DEMO_DIR; exit' INT

sleep 30

echo "演示结束，清理进程..."
kill $SERVER_PID $CLIENT1_PID $CLIENT2_PID 2>/dev/null
rm -rf $DEMO_DIR

echo "演示完成！"
echo ""
echo "贴吧风格特点："
echo "✓ 时间戳显示在消息框的左上角"
echo "✓ 所有消息都左对齐，保持时间顺序"
echo "✓ 楼层编号显示，模仿贴吧楼层效果"
echo "✓ 用框线围绕每条消息，清晰分隔"
echo "✓ 颜色区分：青色(自己)，黄色(别人)"
