#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>
#include <pthread.h>

#define SERV_PORT 29999

pthread_key_t key;
// 客户端连接信息
struct cli_info{
    struct sockaddr_in cliaddr;
    int connfd;
};
// 用链表存储客户端的每一条消息——零长数组使结构体变长
typedef struct List{
    int len;
    struct List* next;
    char data[0];
}ListNode;
// TSD value
struct status{
    char name[16];
    struct cli_info *ts;
    ListNode* head;
    ListNode* cur;
};

void show_welcome();
void show_offline();
void* work(void *arg);
void destructor(void *arg);
void rev_str(char *str,int len);
void save_data(char* recvMsg,int len);


int main(int argc, char **argv){
    int listenfd, connfd;
    struct sockaddr_in cliaddr, servaddr;
	socklen_t clilen;
    pthread_t tid;
    int cur_index = 0;
    struct cli_info ts[128];
    int ret = -1;
    pthread_key_create(&key,destructor);

    // socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket error");
        exit(1);
    }

    // init sockaddr
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);
    
    // bind
	ret = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(ret == -1){
        perror("bind error");
        exit(1);
    }

    // listen
	ret = listen(listenfd,10);
    if(ret == -1){
        perror("listen error");
        exit(1);
    }
    
    while(1){
        //accept
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if(connfd == -1){
			perror("socket error");
			exit(1);
    	}
        
        //
        ts[cur_index].cliaddr = cliaddr;
        ts[cur_index].connfd = connfd;

        //create thread to work
        pthread_create(&tid,NULL,work,(void*)&ts[cur_index]);
        //线程分离，避免僵尸线程
        pthread_detach(tid);
        cur_index++;
	}
	
	return 0;
}

void* work(void* arg){
    int len;
    char recvMsg[1024];
    struct cli_info *ts = (struct cli_info*) arg;
    // 构造TSD
    struct status cli_status;
    cli_status.ts = ts;
    cli_status.head = (ListNode*) malloc(sizeof(ListNode));
    cli_status.cur = cli_status.head;
    pthread_setspecific(key,(void*) &cli_status);

    show_welcome();

    while(1){
        len = read(ts->connfd,recvMsg,sizeof(recvMsg));

        if(len > 0){
            //反转字符串
            rev_str(recvMsg,len-1);
            recvMsg[len-1] = '\0';

            save_data(recvMsg,len);

            write(ts->connfd,recvMsg,len);
        }else{
            show_offline();
            break;
        }
    }

    return (void *)0;
}



void show_welcome(){
    struct status *cli_status = (struct status*) pthread_getspecific(key);
    struct cli_info *ts = cli_status->ts;
    char str[INET_ADDRSTRLEN];
    printf("received from %s at PORT %d\n",
        inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
        ntohs((*ts).cliaddr.sin_port));  
}

void show_offline(){
    struct status *cli_status = (struct status*) pthread_getspecific(key);
    struct cli_info *ts = cli_status->ts;
    char str[INET_ADDRSTRLEN];
    printf("%s at PORT %d closed\n",
        inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
        ntohs((*ts).cliaddr.sin_port));  
}

// 保存数据
void save_data(char* recvMsg,int len){
    ListNode *temp = (ListNode*) malloc(sizeof(ListNode) + len);
    //填充数据
    temp->len = strlen(recvMsg);
    for(int i = 0; i < len-1; ++i){
        temp->data[i] = recvMsg[i];
    }
    temp->data[len-1] = '\0';
    temp->next = NULL;
    // 挂载到链表中
    struct status *cli_status = (struct status*) pthread_getspecific(key);
    cli_status->cur->next = temp;
    cli_status->cur = cli_status->cur->next;
}

// TSD析构函数
void destructor(void *arg){
    struct status *cli_status = (struct status*) arg;
    // 打印所有发送过的消息
    ListNode* head = cli_status->head;
    ListNode* temp;

    while(head){
        temp = head;
        printf("%s",head->data);
        head = head->next;
        free(temp);
    }
    printf("\n");

    //关闭 fd
    close(cli_status->ts->connfd);
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