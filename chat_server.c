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
#include <ctype.h>

#define PORT 8888
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_HISTORY 30
#define MAX_USERS 1000

// 用户认证状态
typedef enum {
    USER_CONNECTING,
    USER_AUTHENTICATING, 
    USER_AUTHENTICATED,
    USER_DISCONNECTED
} user_state_t;

// 用户信息结构体
typedef struct {
    char username[50];
    char password[50];
    char email[64];
    time_t register_time;
} user_t;

// 客户端信息结构体
typedef struct {
    int socket;
    int id;
    char name[50];
    char username[50];
    struct sockaddr_in address;
    user_state_t auth_state;
    time_t connect_time;
} client_t;

// 历史消息结构体
typedef struct {
    char message[BUFFER_SIZE + 150];
    int is_system_msg;
} history_message_t;

// 全局变量
client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

// 用户数据库
user_t users[MAX_USERS];
int user_count = 0;
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;

// 历史消息缓存
history_message_t message_history[MAX_HISTORY];
int history_count = 0;
int history_start = 0;
pthread_mutex_t history_mutex = PTHREAD_MUTEX_INITIALIZER;

// 函数声明
void *handle_client(void *arg);
void send_message_to_all(char *message, int sender_id);
void send_message_to_sender(char *original_message, int sender_id, const char* sender_name, const char* time_str);
void remove_client(int id);
void add_client(client_t *client);
void get_time_string(char* time_str);
void add_message_to_history(char *message, int is_system);
void send_history_to_client(int client_socket);
int authenticate_user(const char* username, const char* password);
int register_user(const char* username, const char* password, const char* email);
int is_valid_username(const char* username);
int is_valid_password(const char* password);
int is_valid_email(const char* email);
int user_exists(const char* username);
void save_users();
void load_users();

// 获取当前时间字符串
void get_time_string(char* time_str) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_str, 20, "%H:%M:%S", tm_info);
}

// 验证用户名
int is_valid_username(const char* username) {
    if (!username || strlen(username) < 3 || strlen(username) > 20) {
        return 0;
    }
    
    if (!isalpha(username[0])) {
        return 0;
    }
    
    for (int i = 0; username[i]; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

// 验证密码
int is_valid_password(const char* password) {
    if (!password || strlen(password) < 6 || strlen(password) > 50) {
        return 0;
    }
    
    int has_upper = 0, has_lower = 0, has_digit = 0;
    
    for (int i = 0; password[i]; i++) {
        if (isupper(password[i])) has_upper = 1;
        if (islower(password[i])) has_lower = 1;
        if (isdigit(password[i])) has_digit = 1;
    }
    
    return (has_upper + has_lower + has_digit) >= 2;
}

// 验证邮箱
int is_valid_email(const char* email) {
    if (!email || strlen(email) < 5 || strlen(email) > 50) {
        return 0;
    }
    
    char* at_pos = strchr(email, '@');
    char* dot_pos = strrchr(email, '.');
    
    return (at_pos && dot_pos && at_pos < dot_pos && 
            at_pos > email && dot_pos < email + strlen(email) - 1);
}

// 检查用户是否存在
int user_exists(const char* username) {
    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            pthread_mutex_unlock(&users_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&users_mutex);
    return 0;
}

// 注册用户
int register_user(const char* username, const char* password, const char* email) {
    if (!is_valid_username(username)) {
        return 0; // 用户名无效
    }
    
    if (!is_valid_password(password)) {
        return 1; // 密码无效
    }
    
    if (!is_valid_email(email)) {
        return 2; // 邮箱无效
    }
    
    if (user_exists(username)) {
        return 3; // 用户已存在
    }
    
    pthread_mutex_lock(&users_mutex);
    if (user_count >= MAX_USERS) {
        pthread_mutex_unlock(&users_mutex);
        return 4; // 用户数量已满
    }
    
    user_t* new_user = &users[user_count];
    strcpy(new_user->username, username);
    strcpy(new_user->password, password);
    strcpy(new_user->email, email);
    new_user->register_time = time(NULL);
    
    user_count++;
    pthread_mutex_unlock(&users_mutex);
    
    save_users();
    printf("用户注册成功: %s (%s)\n", username, email);
    return 5; // 注册成功
}

// 用户认证
int authenticate_user(const char* username, const char* password) {
    if (!username || !password) {
        return -1; // 参数错误
    }
    
    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (strcmp(users[i].password, password) == 0) {
                pthread_mutex_unlock(&users_mutex);
                return 1; // 认证成功
            } else {
                pthread_mutex_unlock(&users_mutex);
                return 0; // 密码错误
            }
        }
    }
    pthread_mutex_unlock(&users_mutex);
    return -2; // 用户不存在
}

// 保存用户数据
void save_users() {
    FILE* file = fopen("users.dat", "wb");
    if (!file) {
        return;
    }
    
    pthread_mutex_lock(&users_mutex);
    fwrite(&user_count, sizeof(int), 1, file);
    fwrite(users, sizeof(user_t), user_count, file);
    pthread_mutex_unlock(&users_mutex);
    
    fclose(file);
}

// 加载用户数据
void load_users() {
    FILE* file = fopen("users.dat", "rb");
    if (!file) {
        // 创建默认管理员账户
        user_t* admin = &users[0];
        strcpy(admin->username, "admin");
        strcpy(admin->password, "Admin123");
        strcpy(admin->email, "admin@chatserver.com");
        admin->register_time = time(NULL);
        user_count = 1;
        save_users();
        printf("创建默认管理员账户: admin/Admin123\n");
        return;
    }
    
    if (fread(&user_count, sizeof(int), 1, file) != 1) {
        fclose(file);
        user_count = 0;
        return;
    }
    
    if (user_count > MAX_USERS) {
        user_count = MAX_USERS;
    }
    
    fread(users, sizeof(user_t), user_count, file);
    fclose(file);
    
    printf("加载用户数据成功，共 %d 个用户\n", user_count);
}

// 添加消息到历史记录
void add_message_to_history(char *message, int is_system) {
    pthread_mutex_lock(&history_mutex);
    
    int index = (history_start + history_count) % MAX_HISTORY;
    strcpy(message_history[index].message, message);
    message_history[index].is_system_msg = is_system;
    
    if (history_count < MAX_HISTORY) {
        history_count++;
    } else {
        history_start = (history_start + 1) % MAX_HISTORY;
    }
    
    pthread_mutex_unlock(&history_mutex);
}

// 向新客户端发送历史消息
void send_history_to_client(int client_socket) {
    pthread_mutex_lock(&history_mutex);
    
    if (history_count > 0) {
        char welcome_msg[BUFFER_SIZE];
        char time_str[20];
        get_time_string(time_str);
        sprintf(welcome_msg, "[%s] >>> 以下是最近的聊天记录 <<<\n", time_str);
        send(client_socket, welcome_msg, strlen(welcome_msg), 0);
        
        for (int i = 0; i < history_count; i++) {
            int index = (history_start + i) % MAX_HISTORY;
            send(client_socket, message_history[index].message, strlen(message_history[index].message), 0);
            usleep(10000);
        }
        
        sprintf(welcome_msg, "[%s] >>> 历史记录结束，以下是实时消息 <<<\n", time_str);
        send(client_socket, welcome_msg, strlen(welcome_msg), 0);
    }
    
    pthread_mutex_unlock(&history_mutex);
}

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

// 向发送者发送自己的消息（显示为"本人"）
void send_message_to_sender(char *original_message, int sender_id, const char* sender_name, const char* time_str) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->id == sender_id) {
            char self_message[BUFFER_SIZE + 200];
            sprintf(self_message, "[%s] 本人: %s\n", time_str, original_message);
            if (send(clients[i]->socket, self_message, strlen(self_message), 0) < 0) {
                perror("发送消息失败");
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 处理客户端连接的线程函数
void *handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 150];
    char time_str[20];
    int leave_flag = 0;
    
    client_t *client = (client_t *)arg;
    client->auth_state = USER_CONNECTING;
    
    get_time_string(time_str);
    printf("[%s] 客户端 (%d) 已连接，等待认证\n", time_str, client->id);
    
    // 认证循环
    while (client->auth_state != USER_AUTHENTICATED && !leave_flag) {
        int receive = recv(client->socket, buffer, BUFFER_SIZE, 0);
        if (receive <= 0) {
            leave_flag = 1;
            break;
        }
        
        buffer[receive] = '\0';
        
        // 处理登录请求
        if (strncmp(buffer, "LOGIN:", 6) == 0) {
            char username[50], password[50];
            char* token = strtok(buffer + 6, ":");
            if (token) {
                strcpy(username, token);
                token = strtok(NULL, ":");
                if (token) {
                    strcpy(password, token);
                    
                    int auth_result = authenticate_user(username, password);
                    
                    if (auth_result == 1) {
                        strcpy(client->username, username);
                        strcpy(client->name, username);
                        client->auth_state = USER_AUTHENTICATED;
                        send(client->socket, "AUTH_SUCCESS", 12, 0);
                        
                        get_time_string(time_str);
                        printf("[%s] 用户 %s 登录成功\n", time_str, username);
                        
                        send_history_to_client(client->socket);
                        
                        sprintf(message, "[%s] >>> %s 加入了聊天室\n", time_str, client->name);
                        send_message_to_all(message, client->id);
                        add_message_to_history(message, 1);
                        
                    } else if (auth_result == 0) {
                        send(client->socket, "AUTH_INVALID_PASSWORD", 21, 0);
                    } else {
                        send(client->socket, "AUTH_INVALID_USER", 17, 0);
                    }
                } else {
                    send(client->socket, "AUTH_ERROR", 10, 0);
                }
            } else {
                send(client->socket, "AUTH_ERROR", 10, 0);
            }
        }
        // 处理注册请求
        else if (strncmp(buffer, "REGISTER:", 9) == 0) {
            char username[50], password[50], email[64];
            char* token = strtok(buffer + 9, ":");
            if (token) {
                strcpy(username, token);
                token = strtok(NULL, ":");
                if (token) {
                    strcpy(password, token);
                    token = strtok(NULL, ":");
                    if (token) {
                        strcpy(email, token);
                        
                        int reg_result = register_user(username, password, email);
                        
                        if (reg_result == 5) {
                            send(client->socket, "REG_SUCCESS", 11, 0);
                        } else if (reg_result == 0) {
                            send(client->socket, "REG_INVALID_USERNAME", 20, 0);
                        } else if (reg_result == 1) {
                            send(client->socket, "REG_INVALID_PASSWORD", 20, 0);
                        } else if (reg_result == 2) {
                            send(client->socket, "REG_INVALID_EMAIL", 17, 0);
                        } else if (reg_result == 3) {
                            send(client->socket, "REG_USER_EXISTS", 15, 0);
                        } else {
                            send(client->socket, "REG_ERROR", 9, 0);
                        }
                    } else {
                        send(client->socket, "REG_ERROR", 9, 0);
                    }
                } else {
                    send(client->socket, "REG_ERROR", 9, 0);
                }
            } else {
                send(client->socket, "REG_ERROR", 9, 0);
            }
        }
        
        memset(buffer, 0, BUFFER_SIZE);
    }
    
    // 认证成功后的消息处理循环
    while (client->auth_state == USER_AUTHENTICATED && !leave_flag) {
        int receive = recv(client->socket, buffer, BUFFER_SIZE, 0);
        if (receive > 0) {
            if (strlen(buffer) > 0) {
                get_time_string(time_str);
                sprintf(message, "[%s] %s: %s\n", time_str, client->name, buffer);
                // 发送给其他人（显示真实用户名）
                send_message_to_all(message, client->id);
                // 发送给自己（显示为"本人"）
                send_message_to_sender(buffer, client->id, client->name, time_str);
                add_message_to_history(message, 0);
                printf("[%s] %s: %s\n", time_str, client->name, buffer);
            }
        } else if (receive == 0 || strcmp(buffer, "exit") == 0) {
            get_time_string(time_str);
            sprintf(message, "[%s] >>> %s 离开了聊天室\n", time_str, client->name);
            send_message_to_all(message, client->id);
            add_message_to_history(message, 1);
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
    
    // 加载用户数据
    load_users();
    
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
    printf("用户数据库: %d 个用户\n", user_count);
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
        client->auth_state = USER_CONNECTING;
        client->connect_time = time(NULL);
        strcpy(client->name, "");
        strcpy(client->username, "");
        
        // 添加客户端到列表
        add_client(client);
        
        // 创建线程处理客户端
        pthread_create(&tid, NULL, &handle_client, (void*)client);
    }
    
    return 0;
}
