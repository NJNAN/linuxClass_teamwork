#!/bin/bash
# 聊天服务器部署脚本

# 显示帮助信息
show_help() {
    echo "===== 聊天服务器部署脚本 ====="
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -h, --help    显示此帮助信息"
    echo "  -i, --install 安装必要依赖"
    echo "  -r, --run     启动服务器"
    echo "  -s, --stop    停止服务器"
    echo "  -c, --clean   清理旧文件"
    echo "======================="
}

# 安装依赖
install_dependencies() {
    echo "正在安装必要的依赖..."
    # 检查是否安装了 gcc 和 make
    if ! command -v gcc &> /dev/null || ! command -v make &> /dev/null; then
        echo "安装 gcc 和 make..."
        if command -v apt-get &> /dev/null; then
            sudo apt-get update
            sudo apt-get install -y gcc make
        elif command -v yum &> /dev/null; then
            sudo yum install -y gcc make
        else
            echo "无法确定包管理器，请手动安装 gcc 和 make"
            exit 1
        fi
    else
        echo "gcc 和 make 已安装，跳过"
    fi
    
    # 检查是否安装了 pthread 库
    if ! ldconfig -p | grep -q libpthread; then
        echo "安装 pthread 库..."
        if command -v apt-get &> /dev/null; then
            sudo apt-get install -y libpthread-stubs0-dev
        elif command -v yum &> /dev/null; then
            sudo yum install -y glibc-devel
        else
            echo "无法确定包管理器，请手动安装 pthread 库"
            exit 1
        fi
    else
        echo "pthread 库已安装，跳过"
    fi
    
    echo "依赖安装完成"
}

# 编译服务器
compile_server() {
    echo "正在编译聊天服务器..."
    make server
    if [ $? -ne 0 ]; then
        echo "编译失败，请检查错误信息"
        exit 1
    fi
    echo "编译成功"
}

# 启动服务器
run_server() {
    echo "正在启动聊天服务器..."
    
    # 检查是否已经有服务器在运行
    if pgrep -f "./chat_server" > /dev/null; then
        echo "警告: 聊天服务器已在运行"
        echo "如需重启，请先使用 -s 或 --stop 选项停止服务器"
        return
    fi
    
    # 启动服务器
    ./chat_server > server_log.txt 2>&1 &
    SERVER_PID=$!
    
    echo "服务器已启动，PID: $SERVER_PID"
    echo "日志保存在 server_log.txt"
    
    # 创建 PID 文件
    echo $SERVER_PID > server.pid
}

# 停止服务器
stop_server() {
    echo "正在停止聊天服务器..."
    
    # 通过 PID 文件找到服务器进程
    if [ -f server.pid ]; then
        SERVER_PID=$(cat server.pid)
        if ps -p $SERVER_PID > /dev/null; then
            kill $SERVER_PID
            echo "服务器已停止 (PID: $SERVER_PID)"
            rm server.pid
        else
            echo "找不到 PID 为 $SERVER_PID 的服务器进程"
        fi
    else
        # 尝试通过进程名找到服务器
        SERVER_PID=$(pgrep -f "./chat_server")
        if [ -n "$SERVER_PID" ]; then
            kill $SERVER_PID
            echo "服务器已停止 (PID: $SERVER_PID)"
        else
            echo "找不到正在运行的聊天服务器"
        fi
    fi
}

# 清理旧文件
clean_files() {
    echo "正在清理旧文件..."
    make clean
    rm -f server_log.txt server.pid
    echo "清理完成"
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
            -i|--install)
                install_dependencies
                ;;
            -r|--run)
                compile_server
                run_server
                ;;
            -s|--stop)
                stop_server
                ;;
            -c|--clean)
                stop_server
                clean_files
                ;;
            *)
                echo "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
        shift
    done
}

# 执行主函数
main "$@"
