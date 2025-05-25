CC = gcc
CFLAGS = -g -Wall -pthread
SERVER = server
CLIENT = client

all: $(SERVER) $(CLIENT)

$(SERVER): server.c
$(CC) $(CFLAGS) -o $(SERVER) server.c

$(CLIENT): client.c
$(CC) $(CFLAGS) -o $(CLIENT) client.c

clean:
rm -f $(SERVER) $(CLIENT) *.o

test: all
@echo "编译完成！"
@echo "服务器: $(SERVER)"
@echo "客户端: $(CLIENT)"

.PHONY: all clean test
