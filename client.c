#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define PORT 8888
#define BUFFER_SIZE 1024

// 全局变量
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

// 函数声明
void str_overwrite_stdout();
void str_trim_lf(char* arr, int length);
void catch_ctrl_c_and_exit(int sig);
void get_time_string(char* time_str);
void print_message_formatted(char* message, int is_self);
void *send_msg_handler();
void *recv_msg_handler();

// 清空标准输出缓冲区并显示提示符
void str_overwrite_stdout() {
    printf("\n");
    printf("────────────────────────────────────────────────────────────────────────────────\n");
    printf("💬 输入消息: ");
    fflush(stdout);
}

// 获取当前时间字符串
void get_time_string(char* time_str) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_str, 20, "%H:%M:%S", tm_info);
}

// 贴吧风格格式化显示消息（时间戳在左上角，所有发言左对齐）
void print_message_formatted(char* message, int is_self) {
    char formatted_msg[BUFFER_SIZE + 300];
    char clean_msg[BUFFER_SIZE + 100];
    char time_str[20];
    char username[50];
    char content[BUFFER_SIZE];
    static int floor_num = 1;  // 楼层计数器
    
    // 移除消息末尾的换行符进行格式化
    strcpy(clean_msg, message);
    clean_msg[strcspn(clean_msg, "\n")] = 0;
    
    // 解析消息格式：[时间] 用户名: 内容
    if (sscanf(clean_msg, "[%[^]]] %[^:]: %[^\n]", time_str, username, content) == 3) {
        printf("\n┌─────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %s", time_str);  // 时间戳在左上角
        
        // 补充右侧空格到右边框
        int time_len = strlen(time_str);
        int spaces_needed = 77 - time_len;  // 77 = 80 - 3 (边框字符)
        for (int i = 0; i < spaces_needed; i++) {
            printf(" ");
        }
        printf("│\n");
        
        if (is_self) {
            // 自己的消息用青色
            printf("│ \033[36m#%d楼 [%s]:\033[0m", floor_num, username);
        } else {
            // 别人的消息用默认色
            printf("│ \033[33m#%d楼 [%s]:\033[0m", floor_num, username);
        }
        
        // 计算用户名行的长度并补齐
        char user_line[100];
        sprintf(user_line, "#%d楼 [%s]:", floor_num, username);
        int user_len = strlen(user_line);
        spaces_needed = 77 - user_len;
        for (int i = 0; i < spaces_needed; i++) {
            printf(" ");
        }
        printf("│\n");
        
        printf("│ ");
        if (is_self) {
            printf("\033[36m%s\033[0m", content);  // 青色显示自己的消息内容
        } else {
            printf("%s", content);  // 默认色显示别人的消息内容
        }
        
        // 计算内容行的长度并补齐
        int content_len = strlen(content);
        spaces_needed = 77 - content_len;
        for (int i = 0; i < spaces_needed; i++) {
            printf(" ");
        }
        printf("│\n");
        printf("└─────────────────────────────────────────────────────────────────────────────┘\n");
        
        floor_num++;  // 增加楼层计数
    } else {
        // 如果解析失败，使用简化的贴吧格式
        printf("\n┌─────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│ 系统消息");
        int spaces_needed = 77 - 8;  // "系统消息" 长度为8
        for (int i = 0; i < spaces_needed; i++) {
            printf(" ");
        }
        printf("│\n");
        printf("│ %s", clean_msg);
        
        int content_len = strlen(clean_msg);
        spaces_needed = 77 - content_len;
        for (int i = 0; i < spaces_needed; i++) {
            printf(" ");
        }
        printf("│\n");
        printf("└─────────────────────────────────────────────────────────────────────────────┘\n");
    }
}

// 去除字符串末尾的换行符
void str_trim_lf(char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

// 处理 Ctrl+C 信号
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

// 发送消息的线程函数
void *send_msg_handler() {
    char message[BUFFER_SIZE] = {};
    char buffer[BUFFER_SIZE + 32] = {};
    
    while (1) {
        str_overwrite_stdout();
        fgets(message, BUFFER_SIZE, stdin);
        str_trim_lf(message, BUFFER_SIZE);
        
        if (strcmp(message, "exit") == 0) {
            break;
        } else if (strlen(message) > 0) {
            sprintf(buffer, "%s", message);
            send(sockfd, buffer, strlen(buffer), 0);
        }
        
        memset(message, 0, BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE + 32);
    }
    catch_ctrl_c_and_exit(2);
    return NULL;
}

// 接收消息的线程函数
void *recv_msg_handler() {
    char message[BUFFER_SIZE] = {};
    while (1) {
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0) {
            // 检查是否是自己的消息
            if (strncmp(message, "SELF:", 5) == 0) {
                // 自己的消息，去掉SELF前缀并以贴吧风格显示
                char* actual_msg = message + 5;
                print_message_formatted(actual_msg, 1);
            } else {
                // 别人的消息，以贴吧风格显示
                print_message_formatted(message, 0);
            }
            str_overwrite_stdout();   // 重新显示输入提示符
        } else if (receive == 0) {
            break;
        } else {
            // 错误处理
        }
        memset(message, 0, sizeof(message));
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("使用方法: %s <服务器IP地址>\n", argv[0]);
        printf("示例: %s 127.0.0.1\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char *ip = argv[1];
    
    signal(SIGINT, catch_ctrl_c_and_exit);
    
    printf("请输入您的用户名: ");
    fgets(name, 32, stdin);
    str_trim_lf(name, strlen(name));
    
    if (strlen(name) > 32 || strlen(name) < 2) {
        printf("用户名长度必须在2-30个字符之间。\n");
        return EXIT_FAILURE;
    }
    
    struct sockaddr_in server_addr;
    
    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("创建套接字失败");
        return EXIT_FAILURE;
    }
    
    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(PORT);
    
    // 连接到服务器
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("无法连接到服务器 %s:%d\n", ip, PORT);
        printf("请检查:\n");
        printf("1. 服务器是否已启动\n");
        printf("2. IP地址是否正确\n");
        printf("3. 网络连接是否正常\n");
        return EXIT_FAILURE;
    }
    
    // 发送用户名到服务器
    send(sockfd, name, 32, 0);
    
    printf("================================================================================\n");
    printf("🎉 欢迎来到贴吧风格聊天室 🎉\n");
    printf("连接成功！服务器: %s:%d\n", ip, PORT);
    printf("💡 提示：输入 'exit' 退出聊天室\n");
    printf("💡 格式：贴吧风格布局 - 时间戳在左上角，所有发言左对齐，楼层显示\n");
    printf("💡 颜色：您的发言为青色，其他用户为黄色标识\n");
    printf("================================================================================\n");
    
    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf("创建发送线程失败!\n");
        return EXIT_FAILURE;
    }
    
    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf("创建接收线程失败!\n");
        return EXIT_FAILURE;
    }
    
    while (1) {
        if (flag) {
            printf("\n再见！\n");
            break;
        }
    }
    
    close(sockfd);
    
    return EXIT_SUCCESS;
}
