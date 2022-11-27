#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/time.h>
#include "helper.h"

int main(int argc,char **argv){
    int i,n;
    int maxfd,listenfd,connfd,sockfd;
    int client[FD_SETSIZE],maxi,nready;
    struct sockaddr_in cliaddr,servaddr;
    socklen_t clilen;
    fd_set rset,allset;
    int ret = -1;

    // 提示使用方式 获取服务器端口和映射目录
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

    maxfd = listenfd;
    
    // 将所有的client初值设置为-1
    maxi = -1;
    for(i = 0;i < FD_SETSIZE; i++)
        client[i]=-1;
    
    // 将listenfd添加到allset中
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);

    while(1){
        rset = allset;
        nready = select(maxfd+1,&rset,NULL,NULL,NULL);
        
        //如果就绪描述符是listenfd
        if(FD_ISSET(listenfd,&rset)){
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
            if (connfd == -1) {   
                perror("accept error");
                exit(1);
            }

            //打印发起连接的客户端信息
            char str[INET_ADDRSTRLEN];
            printf("New Client: IP %s, PORT %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                ntohs(cliaddr.sin_port));
            
            // 为connfd找到一个存放的位置
            for(i = 0;i < FD_SETSIZE; i++){
                if(client[i]<0){
                    client [i] = connfd;
                    break;
                }
            }

            if(i == FD_SETSIZE){
                perror("too many clients");
                exit(1);
            }

            FD_SET(connfd,&allset);
            if(connfd > maxfd)
                maxfd = connfd;

            if(i > maxi)
                maxi = i;

            if(--nready <= 0)
                continue;
        }

        for(i = 0;i <= maxi; i++){
            if((sockfd = client[i]) < 0)
                continue;
            
            // 如果就绪描述符是客户描述符
            if(FD_ISSET(sockfd,&rset)){
                do_process(sockfd);

                FD_CLR(sockfd,&allset);

                if(--nready <=0)
                    break;
            }
        }
    }
}
