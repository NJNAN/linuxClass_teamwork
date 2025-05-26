# 聊天服务器部署指南

本文档介绍如何将聊天服务器部署到远程服务器上。

## 部署步骤

### 1. 准备文件

将以下文件传输到远程服务器：
- `chat_server.c` - 服务器源代码
- `Makefile` - 编译规则
- `deploy_server.sh` - 部署脚本

您可以使用 `scp` 命令将文件传输到服务器：

```bash
scp chat_server.c Makefile deploy_server.sh username@server_ip:/path/to/destination/
```

替换 `username` 为您的服务器用户名，`server_ip` 为服务器IP地址，`/path/to/destination/` 为目标目录。

### 2. 服务器上的设置

登录到服务器：

```bash
ssh username@server_ip
```

进入文件所在目录：

```bash
cd /path/to/destination/
```

给部署脚本添加执行权限：

```bash
chmod +x deploy_server.sh
```

### 3. 运行部署脚本

安装必要的依赖：

```bash
./deploy_server.sh --install
```

启动服务器：

```bash
./deploy_server.sh --run
```

### 4. 验证服务器运行状态

检查服务器是否正在运行：

```bash
ps aux | grep chat_server
```

查看服务器日志：

```bash
cat server_log.txt
```

### 5. 停止服务器

如需停止服务器：

```bash
./deploy_server.sh --stop
```

## 修改客户端配置

要让客户端连接到远程服务器，需要修改客户端代码中的 `SERVER_IP` 定义：

1. 打开 `chat_client.c` 文件
2. 找到以下行：
   ```c
   #define SERVER_IP "127.0.0.1"
   ```
3. 将 IP 地址修改为服务器的公网 IP 地址：
   ```c
   #define SERVER_IP "your_server_ip"
   ```
4. 重新编译客户端：
   ```bash
   make client
   ```

## 防火墙设置

确保服务器防火墙允许 8888 端口的入站连接。根据服务器的防火墙配置，可能需要运行以下命令：

对于 UFW (Ubuntu):
```bash
sudo ufw allow 8888/tcp
```

对于 firewalld (CentOS/RHEL):
```bash
sudo firewall-cmd --permanent --add-port=8888/tcp
sudo firewall-cmd --reload
```

## 故障排除

如果客户端无法连接到服务器：

1. 确认服务器正在运行：`ps aux | grep chat_server`
2. 检查防火墙设置：确保 8888 端口已开放
3. 验证客户端的 SERVER_IP 设置正确
4. 检查服务器日志中是否有错误信息：`cat server_log.txt`

## 高级配置

### 更改默认端口

如需更改默认端口 (8888)：

1. 修改 `chat_server.c` 中的 `PORT` 定义
2. 修改 `chat_client.c` 中的 `PORT` 定义
3. 确保防火墙允许新端口的连接
4. 重新编译服务器和客户端

### 设置为系统服务

要将聊天服务器设置为系统服务（开机自启动），可以创建一个 systemd 服务文件。具体步骤请参考服务器操作系统的文档。
