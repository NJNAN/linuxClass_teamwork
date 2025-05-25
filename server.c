#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#define PORT 8888
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// 客户端信息结构体
typedef struct {
    int socket;
    int id;
    char name[50];
    struct sockaddr_in address;
} client_t;

// 全局变量
client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

// 函数声明
void *handle_client(void *arg);
void send_message_to_all(char *message, int sender_id);
void send_message_to_sender(char *message, int sender_id);
void remove_client(int id);
void add_client(client_t *client);
void get_time_string(char* time_str);

// 添加客户端到列表
void add_client(client_t *client) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i]) {
            clients[i] = client;
            client_count++;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 从列表中移除客户端
void remove_client(int id) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->id == id) {
            clients[i] = NULL;
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 向所有客户端发送消息
void send_message_to_all(char *message, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->id != sender_id) {
            if (send(clients[i]->socket, message, strlen(message), 0) < 0) {
                perror("发送消息失败");
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 向发送者发送自己的消息（带标识）
void send_message_to_sender(char *message, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->id == sender_id) {
            char self_message[BUFFER_SIZE + 200];
            sprintf(self_message, "SELF:%s", message);  // 添加SELF标识
            if (send(clients[i]->socket, self_message, strlen(self_message), 0) < 0) {
                perror("发送消息失败");
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 获取当前时间字符串
void get_time_string(char* time_str) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_str, 20, "%H:%M:%S", tm_info);
}

// 处理客户端连接的线程函数
void *handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 150];  // 增加缓冲区大小以容纳时间戳
    char time_str[20];
    int leave_flag = 0;
    
    client_t *client = (client_t *)arg;
    
    get_time_string(time_str);
    printf("[%s] 客户端 %s (%d) 已连接\n", time_str, client->name, client->id);
    
    // 通知其他客户端有新用户加入
    get_time_string(time_str);
    sprintf(message, "[%s] >>> %s 加入了聊天室\n", time_str, client->name);
    send_message_to_all(message, client->id);
    
    // 持续接收客户端消息
    while (1) {
        if (leave_flag) {
            break;
        }
        
        int receive = recv(client->socket, buffer, BUFFER_SIZE, 0);
        if (receive > 0) {
            if (strlen(buffer) > 0) {
                get_time_string(time_str);
                sprintf(message, "[%s] %s: %s\n", time_str, client->name, buffer);
                send_message_to_all(message, client->id);  // 发送给其他人
                send_message_to_sender(message, client->id);  // 发送给自己
                printf("[%s] %s: %s\n", time_str, client->name, buffer);
            }
        } else if (receive == 0 || strcmp(buffer, "exit") == 0) {
            get_time_string(time_str);
            sprintf(message, "[%s] >>> %s 离开了聊天室\n", time_str, client->name);
            send_message_to_all(message, client->id);
            printf("[%s] 客户端 %s (%d) 断开连接\n", time_str, client->name, client->id);
            leave_flag = 1;
        } else {
            get_time_string(time_str);
            printf("[%s] 接收消息错误: %s\n", time_str, strerror(errno));
            leave_flag = 1;
        }
        
        memset(buffer, 0, BUFFER_SIZE);
    }
    
    // 清理资源
    close(client->socket);
    remove_client(client->id);
    free(client);
    pthread_detach(pthread_self());
    
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t tid;
    
    // 创建套接字
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }
    
    // 设置套接字选项，允许重用地址
    int option = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        perror("设置套接字选项失败");
        exit(EXIT_FAILURE);
    }
    
    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // 绑定套接字
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("绑定失败");
        exit(EXIT_FAILURE);
    }
    
    // 开始监听
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("监听失败");
        exit(EXIT_FAILURE);
    }
    
    printf("=== 聊天服务器启动 ===\n");
    printf("监听端口: %d\n", PORT);
    printf("最大客户端数: %d\n", MAX_CLIENTS);
    printf("等待客户端连接...\n\n");
    
    // 主循环，接受客户端连接
    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            perror("接受连接失败");
            continue;
        }
        
        // 检查客户端数量
        if (client_count >= MAX_CLIENTS) {
            printf("达到最大客户端数量。拒绝连接。\n");
            close(client_socket);
            continue;
        }
        
        // 创建客户端结构体
        client_t *client = (client_t *)malloc(sizeof(client_t));
        client->address = client_addr;
        client->socket = client_socket;
        client->id = client_socket;
        
        // 接收客户端用户名
        recv(client->socket, client->name, 32, 0);
        client->name[strcspn(client->name, "\n")] = 0; // 移除换行符
        
        // 添加客户端到列表
        add_client(client);
        
        // 创建线程处理客户端
        pthread_create(&tid, NULL, &handle_client, (void*)client);
    }
    
    return 0;
}
