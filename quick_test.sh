#!/bin/bash

echo "正在连接服务器进行测试..."
echo "用户名: 测试用户1"
echo "hello world" | timeout 3 ./client 127.0.0.1 <<EOF
测试用户1
hello world
exit
EOF
