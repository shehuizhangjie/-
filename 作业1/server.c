#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>


int main(int argc, char **argv)
{
	if (argc != 3){
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	int					listenfd, connfd;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	int ret = -1;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket error");
        exit(1);
    }


	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    servaddr.sin_port = htons((uint16_t)atoi(argv[2]));

	ret = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(ret == -1){
        perror("bind error");
        exit(1);
    }

	ret = listen(listenfd,10);
	if(ret == -1){
        perror("listen error");
        exit(1);
    }

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
	
	close(connfd);
	close(listenfd);
	
	return 0;
}