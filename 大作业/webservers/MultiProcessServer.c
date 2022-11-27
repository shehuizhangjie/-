#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include "helper.h"

void sig_chld(int signo);//处理僵尸进程

int main(int argc, char **argv){
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
    int ret = -1;
    pid_t pid;

	if (argc != 3){
        fprintf(stderr, "usage: %s <port> <path>\n", argv[0]);
		exit(1);
	}
    int port = atoi(argv[1]);
    
    ret = chdir(argv[2]);
    if(ret == -1){
        perror("chdir error");
        exit(1);
    }

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

	while(1) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        if (connfd == -1) {   
            perror("accept error");
            exit(1);
        }

        // 创建进程，根据返回值确定父子进程
        pid = fork();
        // 子进程负责进行处理
		if(pid == 0){
            // 关闭listenfd套接字
            close(listenfd);

            // 打印发起连接的客户端信息
            char str[INET_ADDRSTRLEN];
            printf("New Client: IP %s, PORT %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                ntohs(cliaddr.sin_port));  
                
            do_process(connfd);
			close(connfd);
			return 0;
		}
        // 父进程负责接受请求
        else if(pid > 0){
            close(connfd);
            signal(SIGCHLD, sig_chld);
        }
        else{
            perror("fork error");
            exit(1);
        }
	}
}

void sig_chld(int signo) {
    pid_t   pid;
    int     stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
		printf("child %d terminated\n", pid);
    }
    return;
}