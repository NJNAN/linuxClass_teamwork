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
#include <signal.h>
#include <termios.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_LINE_LENGTH 80  // 每行最大字符数

// 全局变量
int sockfd;
char username[50];
int authenticated = 0;
int running = 1;
struct termios orig_termios;  // 保存原始终端设置

// 函数声明
void *receive_handler(void *socket);
void login_menu();
void register_user();
int login_user();
void clear_screen();
void show_welcome();
void handle_input();
void display_prompt();
void word_wrap_print(const char* text, int max_width);
void cleanup_and_exit();
void disable_echo();
void enable_echo();

// 清屏函数
void clear_screen() {
    printf("\033[H\033[J");
}

// 显示欢迎界面
void show_welcome() {
    clear_screen();
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║              聊天室客户端                    ║\n");
    printf("║                                              ║\n");
    printf("║   [1] 登录                                   ║\n");
    printf("║   [2] 注册                                   ║\n");
    printf("║   [3] 退出                                   ║\n");
    printf("║                                              ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("请选择操作: ");
}

// 自动换行打印函数
void word_wrap_print(const char* text, int max_width) {
    int len = strlen(text);
    int pos = 0;
    
    while (pos < len) {
        int end_pos = pos + max_width;
        if (end_pos >= len) {
            // 打印剩余部分
            printf("%s", text + pos);
            break;
        }
        
        // 寻找合适的断点（空格或标点）
        int break_pos = end_pos;
        for (int i = end_pos; i > pos; i--) {
            if (text[i] == ' ' || text[i] == ',' || text[i] == '!' || text[i] == '?') {
                break_pos = i;
                break;
            }
        }
        
        // 如果找不到合适断点，就在最大宽度处断开
        if (break_pos == pos) {
            break_pos = end_pos;
        }
        
        // 打印这一行
        for (int i = pos; i < break_pos && i < len; i++) {
            printf("%c", text[i]);
        }
        printf("\n");
        
        // 跳过空格
        pos = break_pos;
        while (pos < len && text[pos] == ' ') {
            pos++;
        }
    }
}

// 禁用终端回显
void disable_echo() {
    struct termios new_termios;
    
    // 获取当前终端设置
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        return;
    }
    
    // 复制当前设置
    new_termios = orig_termios;
    
    // 禁用回显，并禁用规范模式以便字符级输入
    new_termios.c_lflag &= ~(ECHO | ICANON);
    new_termios.c_cc[VMIN] = 1;   // 最少读取1个字符
    new_termios.c_cc[VTIME] = 0;  // 不设置超时
    
    // 应用新设置
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios) == -1) {
        perror("tcsetattr");
    }
}

// 启用终端回显
void enable_echo() {
    // 恢复原始终端设置
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr");
    }
}

// 显示输入提示符
void display_prompt() {
    printf("请输入消息>> "); // 移除了前导 \\n
    fflush(stdout);
}

// 注册用户
void register_user() {
    char username[50], password[50], email[64];
    char buffer[BUFFER_SIZE];
    
    printf("\n=== 用户注册 ===\n");
    printf("用户名 (3-20字符，字母开头): ");
    scanf("%s", username);
    
    printf("密码 (6-50字符，需包含大小写字母、数字中至少两种): ");
    scanf("%s", password);
    
    printf("邮箱: ");
    scanf("%s", email);
    
    // 发送注册请求
    sprintf(buffer, "REGISTER:%s:%s:%s", username, password, email);
    send(sockfd, buffer, strlen(buffer), 0);
    
    // 接收注册结果
    int receive = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (receive > 0) {
        buffer[receive] = '\0';
        
        if (strcmp(buffer, "REG_SUCCESS") == 0) {
            printf("注册成功！可以使用该账户登录了。\n");
        } else if (strcmp(buffer, "REG_INVALID_USERNAME") == 0) {
            printf("用户名格式无效！\n");
        } else if (strcmp(buffer, "REG_INVALID_PASSWORD") == 0) {
            printf("密码格式无效！\n");
        } else if (strcmp(buffer, "REG_INVALID_EMAIL") == 0) {
            printf("邮箱格式无效！\n");
        } else if (strcmp(buffer, "REG_USER_EXISTS") == 0) {
            printf("用户名已存在！\n");
        } else {
            printf("注册失败，请稍后重试。\n");
        }
    }
    
    printf("按回车键继续...");
    getchar();
    getchar();
}

// 用户登录
int login_user() {
    char password[50];
    char buffer[BUFFER_SIZE];
    
    printf("\n=== 用户登录 ===\n");
    printf("用户名: ");
    scanf("%s", username);
    
    printf("密码: ");
    scanf("%s", password);
    
    // 发送登录请求
    sprintf(buffer, "LOGIN:%s:%s", username, password);
    send(sockfd, buffer, strlen(buffer), 0);
    
    // 接收认证结果
    int receive = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (receive > 0) {
        buffer[receive] = '\0';
        
        if (strcmp(buffer, "AUTH_SUCCESS") == 0) {
            printf("登录成功！欢迎 %s\n", username);
            authenticated = 1;
            return 1;
        } else if (strcmp(buffer, "AUTH_INVALID_PASSWORD") == 0) {
            printf("密码错误！\n");
        } else if (strcmp(buffer, "AUTH_INVALID_USER") == 0) {
            printf("用户不存在！\n");
        } else {
            printf("登录失败，请稍后重试。\n");
        }
    }
    
    printf("按回车键继续...");
    getchar();
    getchar();
    return 0;
}

// 登录菜单
void login_menu() {
    int choice;
    
    while (!authenticated) {
        show_welcome();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                if (login_user()) {
                    return;
                }
                break;
            case 2:
                register_user();
                break;
            case 3:
                printf("再见！\n");
                cleanup_and_exit();
                break;
            default:
                printf("无效选择，请重试。\n");
                printf("按回车键继续...");
                getchar();
                getchar();
                break;
        }
    }
}

// 接收消息的线程函数
void *receive_handler(void *arg) {
    char message[BUFFER_SIZE];
    (void)arg; // 避免未使用参数警告
    
    while (running) {
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0) {
            message[receive] = '\0';
            
            // 先清除当前行上的输入提示符
            printf("\r\033[K");
            
            // 显示收到的消息（自动换行）
            word_wrap_print(message, MAX_LINE_LENGTH);
            
            // 重新显示提示符
            if (authenticated) {
                display_prompt();
            }
        } else if (receive == 0) {
            printf("\n服务器断开连接。\n");
            running = 0;
            break;
        } else {
            if (running) {
                printf("\n接收消息错误。\n");
            }
            break;
        }
    }
    
    return NULL;
}

// 处理输入
void handle_input() {
    char line[BUFFER_SIZE];
    
    // 显示初始提示符
    if (authenticated) {
        display_prompt();
    }
    
    while (running) {
        if (fgets(line, sizeof(line), stdin) != NULL) {
            // 移除换行符
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
                len--;
            }
            
            if (len > 0) {
                // 发送消息前先清除用户刚刚输入的内容
                // 移动光标到行首并清除整行
                printf("\033[1A\r\033[K");
                fflush(stdout);
                
                // 发送消息
                send(sockfd, line, strlen(line), 0);
                
                // 不立即显示提示符，等待服务器响应
            } else {
                // 如果是空输入，也重新显示提示符
                printf("\r\033[K");
                if (authenticated) {
                    display_prompt();
                }
            }
        }
    }
}

// 清理并退出
void cleanup_and_exit() {
    running = 0;
    // enable_echo();  // 不需要恢复终端设置，因为我们没有改变它
    if (sockfd > 0) {
        close(sockfd);
    }
    exit(0);
}

// 信号处理函数
void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\n正在退出...\n");
        cleanup_and_exit();
    }
}

// 显示使用帮助
void show_usage(char* program_name) {
    printf("用法: %s [服务器IP地址]\n", program_name);
    printf("如未指定IP地址，将使用默认地址: %s\n", SERVER_IP);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;
    pthread_t receive_thread;
    char server_ip[16] = SERVER_IP;  // 默认使用宏定义的IP
    
    // 解析命令行参数
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            show_usage(argv[0]);
            exit(0);
        } else {
            // 检查IP地址格式是否有效
            if (inet_addr(argv[1]) == INADDR_NONE) {
                printf("错误: 无效的IP地址格式: %s\n", argv[1]);
                show_usage(argv[0]);
                exit(1);
            }
            strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
            server_ip[sizeof(server_ip) - 1] = '\0';  // 确保字符串以null结尾
            printf("使用指定的服务器IP地址: %s\n", server_ip);
        }
    } else {
        printf("使用默认服务器IP地址: %s\n", server_ip);
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    
    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }
    
    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    
    // 连接到服务器
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("连接服务器失败");
        exit(EXIT_FAILURE);
    }
    
    printf("成功连接到服务器！\n");
    
    // 显示登录菜单并进行认证
    login_menu();
    
    if (!authenticated) {
        cleanup_and_exit();
    }
    
    // 清屏并显示聊天界面
    clear_screen();
    printf("=== 欢迎来到聊天室 ===\n");
    printf("输入消息即可聊天，Ctrl+C 退出\n");
    printf("消息会自动换行，每行最多 %d 字符\n", MAX_LINE_LENGTH);
    printf("========================\n\n");
    
    // 创建接收消息的线程
    pthread_create(&receive_thread, NULL, receive_handler, (void*)&sockfd);
    
    // 不在这里显示初始输入提示符，由 handle_input 函数负责显示
    
    // 处理用户输入
    handle_input();
    
    // 等待接收线程结束
    pthread_join(receive_thread, NULL);
    
    cleanup_and_exit();
    return 0;
}
