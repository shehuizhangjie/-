#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>    
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include "helper.h"
#include "threadpool.h"

int parse_argc(int argc, char **argv);
int init_server(int port);
int do_accept(int listenfd);
void *process(void *arg);

// 线程池式Web服务器
int main(int argc, char **argv)
{	
    int client[4096];
    int i = 0;
    int port = parse_argc(argc,argv);

    int listenfd = init_server(port);
    
    // 创建线程池，池里最小3个线程，最大100，队列最大100
    threadpool_t *thp = threadpool_create(3,100,100);   
    printf("ThreadPool Inited\n");
    
    // 主进程负责接受并处理请求, 任务交给线程池中的线程进行处理
    while(1){
        int connfd = do_accept(listenfd);
        client[i] = connfd;
        
        //向线程池中添加任务
        threadpool_add(thp, process, (void*)&client[i]);

        i++;
        
        // connfd会在线程池中被关闭，这里不要调用close()
	}
    
    close(listenfd);
	// 销毁线程池
    threadpool_destroy(thp);

	return 0;
}

int parse_argc(int argc, char **argv){
    // 提示使用方式 获取服务器端口和映射目录
	if (argc != 3){
        fprintf(stderr, "usage: %s <port> <path>\n", argv[0]);
		exit(1);
	}
    int port = atoi(argv[1]);
    
    int ret = chdir(argv[2]);
    if(ret == -1){
        perror("chdir error");
        exit(1);
    }
    
    return port;
}

int init_server(int port){
    int listenfd;
    struct sockaddr_in servaddr;
    int ret = -1;

    // 建立listenfd套接字
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket error");
        exit(1);
    }

    // 设置属性
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    //设置地址重用
    int opt = 1;
    setsockopt(listenfd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&opt,sizeof(opt));
    
    // bind
	ret = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(ret == -1){
        perror("bind error");
        exit(1);
    }

    // listen
	ret = listen(listenfd,256);
    if(ret == -1){
        perror("listen error");
        exit(1);
    }

    return listenfd;
}

int do_accept(int listenfd){
	int connfd;
	socklen_t clilen;
	struct sockaddr_in	cliaddr;
    
    //accept
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    if (connfd == -1) {   
        perror("accept error");
        exit(1);
    }
    //打印发起连接的客户端信息
    char str[INET_ADDRSTRLEN];
    printf("New Client: IP %s, PORT %d\n",
        inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
        ntohs(cliaddr.sin_port));

    return connfd;
}


void *process(void *arg){
    int* pi = (int *) arg;
    int connfd = *pi;
    do_process(connfd);
    return NULL;
}