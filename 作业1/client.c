#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 3){
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	int	sockfd;
	struct sockaddr_in	servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket error");
        exit(1);
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons((uint16_t)atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	int ret = connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	if(ret == -1){
        perror("connect error");
        exit(1);
    }
	
	close(sockfd);
	
	return 0;
}
