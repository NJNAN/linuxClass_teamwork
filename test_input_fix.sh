#!/bin/bash

echo "=== 测试聊天客户端输入显示修复 ==="
echo
echo "此脚本将启动客户端来测试输入显示问题是否已修复"
echo "请按照以下步骤测试："
echo "1. 客户端启动后，选择登录 (输入 1)"
echo "2. 使用用户名: test, 密码: Test123"
echo "3. 进入聊天室后，输入一些消息"
echo "4. 检查是否还能看到原始用户输入文本"
echo "5. 预期结果: 只应该看到格式化的消息，不应该看到原始输入"
echo
echo "按 Enter 键开始测试..."
read

echo "启动客户端..."
./chat_client
