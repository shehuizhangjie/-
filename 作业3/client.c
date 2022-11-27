#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>

int main(int argc, char **argv){
	int	sockfd;
	struct sockaddr_in	servaddr;
	int len;
	char sendMsg[1024],recvMsg[1024];
	int ret = -1;
	
    // argc judge
	if (argc != 3){
        printf("Usage:%s <IP> <Port>\n",argv[0]);
		exit(-1);
	}

    // socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
        perror("socket error");
        exit(1);
    }
    // init sockaddr 
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons((uint16_t)atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    // connect
	ret = connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	if(ret == -1){
        perror("connect error");
        exit(1);
    }



	while(1){
		memset(recvMsg,'\0',sizeof(recvMsg));
		memset(sendMsg,'\0',sizeof(sendMsg));

		if(fgets(sendMsg,sizeof(sendMsg),stdin) == NULL) break;

		write(sockfd, sendMsg, strlen(sendMsg));
		
		len = read(sockfd, recvMsg, sizeof(recvMsg)-1);

		printf("Message from server:%s\n",recvMsg);
	}
	
	close(sockfd);
	
	return 0;
}
