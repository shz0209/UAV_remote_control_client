#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>
#include <pthread.h>
#include "../include/get_video.h"


#define HOST "127.0.0.1"        // 根据你服务器的IP地址修改
#define PORT 6666                   // 根据你服务器进程绑定的端口号修改
//#define BUFFER_SIZ (4 * 1024)           // 4k的数据区域
#define BUFFER_SIZ 1           // 4k的数据区域


int main()
{
    pthread_t thread; // 用于存放线程标识符的变量
    // 创建并启动摄像头捕获线程
    int result = pthread_create(&thread,NULL,get_video,NULL);
    if (result != 0) {  
        fprintf(stderr, "Error: Failed to create capture thread.\n");
        return 1;
    }
    
    struct termios oldt, newt;
    int ch;

    /* 获取当前终端设置 */
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    /* 将终端设置为不缓冲和不回显模式 */
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);


    int sockfd, ret;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZ];        //用于保存输入的文本

    // 创建套接字描述符
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create an endpoint for communication fail!\n");
        exit(1);
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(HOST);

    // 建立TCP连接
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        printf("connect server fail...\n");
        close(sockfd);
        exit(1);
    }

    printf("connect server success...\n");
    printf("please enter some text: ");

    while (1) {
        ch = getchar();

        // printf("please enter some text: ");
        //fgets(buffer, BUFFER_SIZ, stdin);
        buffer[0]=ch;
        
        //输入了end，退出循环（程序）
        if(strncmp(buffer, "end", 3) == 0)
            break;

        write(sockfd, buffer, sizeof(buffer));
    }

    close(sockfd);
    pthread_join(thread, NULL);
    printf("Main thread: Capture thread has finished.\n");
    return 0;
    exit(0);

}
