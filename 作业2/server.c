#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>

void rev_str(char *str,int len);

int main(int argc, char **argv)
{
	int					listenfd, connfd;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	int len;
	char recvMsg[100];
	int ret = -1;
	
	if (argc != 3){
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

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
	
	for( ; ; ){
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
	    if(connfd == -1){
			perror("socket error");
			exit(1);
    	}
		//打印发起连接的客户端信息
		char str[INET_ADDRSTRLEN];
		printf("New Client: IP %s, PORT %d\n",
			inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
			ntohs(cliaddr.sin_port));

		for ( ; ; ) {
			memset(recvMsg,'\0',sizeof(recvMsg));
			
			len = read(connfd, recvMsg, sizeof(recvMsg));
                
			if(len > 0){
				rev_str(recvMsg,len-1);
				recvMsg[len-1] = '\0';

				write(connfd,recvMsg,len);
			}else{
				printf("Client leave\n");
				break;
			}
		}
		close(connfd);
	}
	
	close(listenfd);
	
	return 0;
}


void rev_str(char *str,int len){
    char *start = str;
    char *end = str + len - 1;
    char ch;

    if(str != NULL){
        while(start < end){
            ch = *start;
            *start++ = *end;
            *end-- = ch;
        }
    }
}