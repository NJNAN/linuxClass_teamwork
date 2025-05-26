# Makefile for Chat Application

CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
TARGET_SERVER = chat_server
TARGET_CLIENT = chat_client
SOURCE_SERVER = chat_server.c
SOURCE_CLIENT = chat_client.c

.PHONY: all clean server client run-server run-client

all: $(TARGET_SERVER) $(TARGET_CLIENT)

server: $(TARGET_SERVER)

client: $(TARGET_CLIENT)

$(TARGET_SERVER): $(SOURCE_SERVER)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SOURCE_SERVER)

$(TARGET_CLIENT): $(SOURCE_CLIENT)
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(SOURCE_CLIENT)

run-server: $(TARGET_SERVER)
	./$(TARGET_SERVER)

run-client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT) users.dat *.o

help:
	@echo "可用的命令:"
	@echo "  make all        - 编译服务端和客户端"
	@echo "  make server     - 只编译服务端"
	@echo "  make client     - 只编译客户端"
	@echo "  make run-server - 编译并运行服务端"
	@echo "  make run-client - 编译并运行客户端"
	@echo "  make clean      - 清理编译文件"
	@echo "  make help       - 显示此帮助信息"
