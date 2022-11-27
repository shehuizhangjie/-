#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/time.h>
#include<sys/socket.h>
#include<sys/types.h>
#define SERV_PORT 29999

// 客户端连接信息
struct cli_info{
    struct sockaddr_in cliaddr;
    int connfd;
};
// 用链表存储客户端的每一条消息
//零长数组使结构体长度可变
typedef struct List{
    int len;
    struct List* next;
    char data[0];
}ListNode;
// 存储客户端连接的数据结构
struct status{
    char name[16];
    struct cli_info *ts;
    ListNode* head;
    ListNode* cur;
};

struct status cli_status[FD_SETSIZE];

void show_welcome(int index);
void show_offline(int index);
void destructor(int index);
void rev_str(char *str,int len);
void save_data(int index,char* recvMsg,int len);

int main(int argc,char **argv){
    int i,len;
    int maxfd,listenfd,connfd,sockfd;
    int client[FD_SETSIZE],maxi,nready;
    char recvMsg[BUFSIZ];
    struct sockaddr_in cli_addr,serv_addr;
    socklen_t cli_addr_len;
    fd_set rset,allset;
    int ret = -1;

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd == -1){
        perror("socket error");
        exit(1);
    }

    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    ret = bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    if(ret == -1){
        perror("bind error");
        exit(1);
    }

    ret = listen(listenfd,128);
    if(ret == -1){
        perror("listen error");
        exit(1);
    }

    maxfd = listenfd;

    maxi = -1;
    for(i = 0;i < FD_SETSIZE; i++)
        client[i]=-1;
    
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);

    for(;;){
        rset = allset;
        nready = select(maxfd+1,&rset,NULL,NULL,NULL);
        
        //就绪套接字是listenfd
        if(FD_ISSET(listenfd,&rset)){
            // accept
            cli_addr_len = sizeof(cli_addr);
            connfd = accept(listenfd,(struct sockaddr *)&cli_addr,&cli_addr_len);
            if(connfd == -1){
                perror("socket error");
                exit(1);
            }
            
            //找到该connfd应存放的位置
            for(i = 0;i < FD_SETSIZE; i++){
                if(client[i]<0){
                    client [i] = connfd;
                    // // 存储客户端连接状态信息
                    struct cli_info ts;
                    ts.cliaddr = cli_addr;
                    ts.connfd = connfd;
                    cli_status[i].ts = &ts;
                    cli_status[i].head = (ListNode*) malloc(sizeof(ListNode));
                    cli_status[i].cur = cli_status[i].head;
                    break;
                }
            }

            //已经达到FD_SET的上限
            if(i == FD_SETSIZE){
                printf("too many clients\n");
                exit(-1);
            }
            
            //将当前connfd添加到allset中
            FD_SET(connfd,&allset);
            if(connfd > maxfd)
                maxfd = connfd;
            
            //更新maxi
            if(i > maxi)
                maxi = i;

            //打印客户端上线消息
            char str[INET_ADDRSTRLEN];
            printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)),
                ntohs(cli_addr.sin_port));
            
            //判断是否还有就绪的套接字
            if(--nready <= 0)
                continue;
        }

        //就绪套接字是客户端套接字
        for(i = 0;i <= maxi; i++){
            if((sockfd = client[i]) < 0)
                continue;
            
            if(FD_ISSET(sockfd,&rset)){
                len = read(sockfd, recvMsg, sizeof(recvMsg));
                
                if(len > 0){
                    rev_str(recvMsg,len-1);
                    recvMsg[len-1] = '\0';

                    save_data(i,recvMsg,len);

                    write(sockfd,recvMsg,len);
                }else{
                    show_offline(i);
                    destructor(i);
                    FD_CLR(sockfd,&allset);
                    client[i] = -1; 
                    break;
                }

                if(--nready <=0)
                    break;
            }
        }
    }
}



void show_welcome(int index){
    struct status *status = &cli_status[index];
    struct cli_info *ts = status->ts;
    char str[INET_ADDRSTRLEN];
    printf("received from %s at PORT %d\n",
        inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
        ntohs((*ts).cliaddr.sin_port));  
}

void show_offline(int index){
    struct status *status = &cli_status[index];
    struct cli_info *ts = status->ts;
    char str[INET_ADDRSTRLEN];
    printf("%s at PORT %d closed\n",
        inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
        ntohs((*ts).cliaddr.sin_port));  
}

// 保存数据
void save_data(int index,char* recvMsg,int len){
    ListNode *temp = (ListNode*) malloc(sizeof(ListNode) + len);
    //填充数据
    temp->len = strlen(recvMsg);
    for(int i = 0; i < len-1; ++i){
        temp->data[i] = recvMsg[i];
    }
    temp->data[len-1] = '\0';
    temp->next = NULL;
    // 挂载到链表中
    
    struct status *status = &cli_status[index];
    status->cur->next = temp;
    status->cur = status->cur->next;
}

void destructor(int index){
    struct status *status = &cli_status[index];
    // 打印所有发送过的消息
    ListNode* head = status->head;
    ListNode* temp;
    printf("The client sent these messages:\n");
    while(head){
        temp = head;
        printf("%s ",head->data);
        head = head->next;
        free(temp);
    }
    printf("\n");

    //关闭 fd
    close(status->ts->connfd);
}


// 字符串翻转
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

