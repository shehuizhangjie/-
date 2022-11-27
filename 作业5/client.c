#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>  
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>


void fun(int);
int main(int argc, char **argv)
{
	int	sockfd;
	struct sockaddr_in	servaddr;
    struct sigaction act;
    act.sa_handler = fun;
    act.sa_flags = 0;
    // 安装信号时，设置SA_RESTART属性来自动恢复
    act.sa_flags |= SA_RESTART;
    sigaction(SIGINT,&act,NULL);
	int ret = -1;
	int len;
	char sendMsg[100],recvMsg[100];
	
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

	while(1){
		ret = connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
		if(ret == -1){
			if (errno == EINTR) {
				printf("interupted , and restart.\n"); 
			}
		}else {
			printf("connect sucessed.\n");
		}
	}


	for(;;)
	{
		memset(recvMsg,'\0',sizeof(recvMsg));
		memset(sendMsg,'\0',sizeof(recvMsg));

		fgets(sendMsg,sizeof(sendMsg),stdin);
			if((strncmp(sendMsg,"EOF",3))==0)
				return 0;
		
		write(sockfd, sendMsg, strlen(sendMsg));
		
		len = read(sockfd, recvMsg, sizeof(recvMsg));
		
		write(STDOUT_FILENO,recvMsg,len);
	}
	
	close(sockfd);
	
	return 0;
}
void fun(int sig){
    printf("in signal function. \n");
    int seconds = time((time_t*)NULL);
    printf("%d\n", seconds);
}
