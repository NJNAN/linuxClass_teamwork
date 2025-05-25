#!/bin/bash

clear
echo "========================================================"
echo "    Linux编程实训 - 双向交流软件项目演示"
echo "========================================================"
echo ""
echo "项目作者: 团队作业"
echo "完成时间: $(date '+%Y年%m月%d日')"
echo ""
echo "🎯 项目特色功能："
echo "  ✅ TCP Socket网络通信"
echo "  ✅ 多线程并发处理"
echo "  ✅ 时间戳显示功能"
echo "  ✅ 聊天界面左右分布（类似微信）"
echo "  ✅ 彩色消息显示"
echo "  ✅ 多客户端实时聊天"
echo ""
echo "📁 项目文件："
echo "  - server.c      (服务器端代码)"
echo "  - client.c      (客户端代码)"
echo "  - Makefile      (编译配置)"
echo "  - README.md     (项目说明)"
echo "  - 使用指南.md   (详细使用指南)"
echo "  - 项目总结.md   (项目总结报告)"
echo ""
echo "🔧 编译状态："
if [ -f "server" ] && [ -f "client" ]; then
    echo "  ✅ 服务器和客户端已编译完成"
else
    echo "  🔄 正在编译程序..."
    make clean >/dev/null 2>&1
    gcc -g -Wall -pthread -o server server.c 2>/dev/null
    gcc -g -Wall -pthread -o client client.c 2>/dev/null
    if [ -f "server" ] && [ -f "client" ]; then
        echo "  ✅ 编译成功！"
    else
        echo "  ❌ 编译失败，请检查代码"
        exit 1
    fi
fi
echo ""
echo "🚀 演示说明："
echo "  1. 当前终端将启动服务器"
echo "  2. 请打开新终端，运行: ./client 127.0.0.1"
echo "  3. 可以打开多个客户端进行聊天测试"
echo "  4. 观察时间戳和左右分布的消息布局"
echo "  5. 输入 'exit' 或按 Ctrl+C 退出"
echo ""
echo "💡 界面效果预览："
echo "  我 [17:15:25] 大家好！"
echo "                              [17:15:30] 张三: 你好，欢迎！"
echo ""
echo "========================================================"
echo "按任意键启动服务器..."
read -n 1 -s
echo ""
echo "🌟 正在启动聊天服务器..."
echo "========================================================"

# 启动服务器
./server
