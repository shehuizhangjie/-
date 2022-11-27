#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int	sockfd;
	struct sockaddr_in	servaddr;
	int len;
	char sendMsg[100],recvMsg[100];
	int ret = -1;
	
	if (argc != 3){
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket error");
        exit(1);
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons((uint16_t)atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	ret = connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	if(ret == -1){
        perror("connect error");
        exit(1);
    }

	for(;;)
	{
		memset(recvMsg,'\0',sizeof(recvMsg));

		memset(sendMsg,'\0',sizeof(recvMsg));

		fgets(sendMsg,sizeof(sendMsg),stdin);
			if( (strncmp(sendMsg,"EOF",3))==0 )
				return 0;
		
		write(sockfd, sendMsg, strlen(sendMsg));
		
		len = read(sockfd, recvMsg, sizeof(recvMsg));
		
		write(STDOUT_FILENO,recvMsg,len);

		printf("\n");
	}
	
	close(sockfd);
	
	return 0;
}
