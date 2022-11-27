#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include "helper.h"

struct cl_info
{
	struct sockaddr_in cliaddr;
	int connfd;
};

void * do_work(void *arg);

int main(int argc, char **argv)
{
	int					listenfd, connfd;
    pthread_t tid;
    int clilen;
	struct sockaddr_in	cliaddr, servaddr;
	struct cl_info ts[1024];
    int ret = -1;
	int i = 0;
	
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

		ts[i].cliaddr = cliaddr;
		ts[i].connfd = connfd;

		pthread_create(&tid, NULL, do_work, (void*)&ts[i]);
        pthread_detach(tid);//子线程分离，避免僵尸进程.
        i++;
	}
}

void * do_work(void *arg){
	struct cl_info *ts = (struct cl_info*)arg;

    // 打印发起连接的客户端信息
    char str[INET_ADDRSTRLEN];
    printf("New Client: IP %s, PORT %d\n",
        inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
        ntohs((*ts).cliaddr.sin_port));

	do_process(ts->connfd);

    close(ts->connfd);
    
    return (void *)0;
}


