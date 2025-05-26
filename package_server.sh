#!/bin/bash
# 打包聊天服务器文件用于部署

# 设置打包的文件名
PACKAGE_NAME="chat_server_package.tar.gz"

# 显示信息
echo "===== 聊天服务器打包工具 ====="
echo "正在准备打包以下文件:"
echo "- chat_server.c"
echo "- Makefile"
echo "- deploy_server.sh"
echo "- SERVER_DEPLOYMENT.md"
echo "- users.dat (如果存在)"

# 创建临时目录
TEMP_DIR="temp_package"
mkdir -p "$TEMP_DIR"

# 复制必要文件
cp chat_server.c Makefile deploy_server.sh SERVER_DEPLOYMENT.md "$TEMP_DIR/"

# 如果用户数据文件存在，也复制
if [ -f users.dat ]; then
    cp users.dat "$TEMP_DIR/"
    echo "- 已包含用户数据文件"
else
    echo "- 未找到用户数据文件，将在服务器上创建默认管理员账户"
fi

# 打包文件
tar -czf "$PACKAGE_NAME" -C "$TEMP_DIR" .
if [ $? -eq 0 ]; then
    echo "打包成功: $PACKAGE_NAME"
else
    echo "打包失败，请检查错误信息"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# 清理临时目录
rm -rf "$TEMP_DIR"

echo ""
echo "使用以下命令将文件传输到服务器:"
echo "scp $PACKAGE_NAME username@server_ip:/path/to/destination/"
echo ""
echo "然后在服务器上解压文件:"
echo "tar -xzf $PACKAGE_NAME"
echo ""
echo "按照 SERVER_DEPLOYMENT.md 中的说明完成部署"
echo "================================"
