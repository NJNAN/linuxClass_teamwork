#!/bin/bash
# 客户端服务器 IP 配置工具

# 设置颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 显示帮助信息
show_help() {
    echo -e "${BLUE}===== 聊天客户端配置工具 =====${NC}"
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -h, --help        显示此帮助信息"
    echo "  -s, --server IP   设置服务器 IP 地址"
    echo "  -p, --port PORT   设置服务器端口"
    echo "  -c, --check       检查当前配置"
    echo "  -r, --reset       重置为本地配置 (127.0.0.1:8888)"
    echo -e "${BLUE}=============================${NC}"
}

# 检查当前配置
check_config() {
    echo -e "${BLUE}当前客户端配置:${NC}"
    
    # 提取当前的服务器 IP 和端口
    CURRENT_IP=$(grep -E "^#define SERVER_IP" chat_client.c | awk -F'"' '{print $2}')
    CURRENT_PORT=$(grep -E "^#define PORT" chat_client.c | awk '{print $3}')
    
    echo -e "服务器 IP: ${GREEN}$CURRENT_IP${NC}"
    echo -e "服务器端口: ${GREEN}$CURRENT_PORT${NC}"
    
    # 测试连接
    echo "正在测试与服务器的连接..."
    if command -v nc &> /dev/null; then
        nc -z -w 2 $CURRENT_IP $CURRENT_PORT
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}连接成功! 服务器在线.${NC}"
        else
            echo -e "${RED}连接失败! 服务器不可达或端口未开放.${NC}"
        fi
    else
        echo -e "${RED}未安装 netcat，无法测试连接.${NC}"
    fi
}

# 设置服务器 IP
set_server_ip() {
    if [ -z "$1" ]; then
        echo -e "${RED}错误: 未指定 IP 地址${NC}"
        show_help
        exit 1
    fi
    
    IP="$1"
    
    # 验证 IP 地址格式
    if ! [[ $IP =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]] && ! [[ $IP =~ ^[a-zA-Z0-9][-a-zA-Z0-9]*(\.[a-zA-Z0-9][-a-zA-Z0-9]*)+$ ]]; then
        echo -e "${RED}错误: 无效的 IP 地址或主机名${NC}"
        exit 1
    fi
    
    # 备份原文件
    cp chat_client.c chat_client.c.bak
    
    # 替换 IP 地址
    sed -i "s/#define SERVER_IP \"[^\"]*\"/#define SERVER_IP \"$IP\"/" chat_client.c
    
    echo -e "${GREEN}服务器 IP 已更新为: $IP${NC}"
    echo "备份文件保存为: chat_client.c.bak"
    
    # 重新编译
    make client
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}客户端已重新编译成功!${NC}"
    else
        echo -e "${RED}编译失败!${NC}"
        echo "恢复备份..."
        cp chat_client.c.bak chat_client.c
        exit 1
    fi
}

# 设置服务器端口
set_server_port() {
    if [ -z "$1" ]; then
        echo -e "${RED}错误: 未指定端口${NC}"
        show_help
        exit 1
    fi
    
    PORT="$1"
    
    # 验证端口是否为数字且在有效范围内
    if ! [[ $PORT =~ ^[0-9]+$ ]] || [ $PORT -lt 1 ] || [ $PORT -gt 65535 ]; then
        echo -e "${RED}错误: 无效的端口号 (应为 1-65535 之间的数字)${NC}"
        exit 1
    fi
    
    # 备份原文件
    cp chat_client.c chat_client.c.bak
    
    # 替换端口
    sed -i "s/#define PORT [0-9]*/#define PORT $PORT/" chat_client.c
    
    echo -e "${GREEN}服务器端口已更新为: $PORT${NC}"
    echo "备份文件保存为: chat_client.c.bak"
    
    # 重新编译
    make client
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}客户端已重新编译成功!${NC}"
    else
        echo -e "${RED}编译失败!${NC}"
        echo "恢复备份..."
        cp chat_client.c.bak chat_client.c
        exit 1
    fi
}

# 重置为本地配置
reset_config() {
    # 备份原文件
    cp chat_client.c chat_client.c.bak
    
    # 替换为本地设置
    sed -i "s/#define SERVER_IP \"[^\"]*\"/#define SERVER_IP \"127.0.0.1\"/" chat_client.c
    sed -i "s/#define PORT [0-9]*/#define PORT 8888/" chat_client.c
    
    echo -e "${GREEN}已重置为本地配置 (127.0.0.1:8888)${NC}"
    echo "备份文件保存为: chat_client.c.bak"
    
    # 重新编译
    make client
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}客户端已重新编译成功!${NC}"
    else
        echo -e "${RED}编译失败!${NC}"
        echo "恢复备份..."
        cp chat_client.c.bak chat_client.c
        exit 1
    fi
}

# 主函数
main() {
    if [ $# -eq 0 ]; then
        show_help
        exit 0
    fi
    
    while [ $# -gt 0 ]; do
        case "$1" in
            -h|--help)
                show_help
                exit 0
                ;;
            -s|--server)
                shift
                set_server_ip "$1"
                ;;
            -p|--port)
                shift
                set_server_port "$1"
                ;;
            -c|--check)
                check_config
                ;;
            -r|--reset)
                reset_config
                ;;
            *)
                echo -e "${RED}未知选项: $1${NC}"
                show_help
                exit 1
                ;;
        esac
        shift
    done
}

# 执行主函数
main "$@"
