#include <stdio.h>          
#include <strings.h>
#include <string.h>         
#include <unistd.h>          
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <pthread.h> 
#include <stdlib.h>

#define PORT 1025
#define Maxsize 1024
int zero=0;
int online_people=0;
char message[1024];
struct userinfo {
    char id[100];
    int opponent;
}users[100];
int fdt[5]={0};
int win_column[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

int find_fd (char *name) {
    for (int i=0;i<100;i++){
        if (strcmp(name,users[i].id)==0){
            return i;
        }
    }
    return -1;
}

void instruction_handle(char *message,int sender){
    int instruction=0;
    sscanf (message,"%d",&instruction);
    //指令1時 儲存名字
    if(instruction==1){
	    char name[100];
        int len=0;
        sscanf (message,"1 %s",name);
        len=strlen(name);
        strncpy (users[sender].id,name,len);
        send(sender,message,strlen(message),0);
        printf("1:%s\n",name);	
    }
    //指令2時 輸出所有的儲存名字,給出在線人數
    if(instruction==2){
	    char buf[Maxsize],tmp[100];
        //char online[Maxsize];
        //sprintf(online,"2 ");
        int p = sprintf(buf,"2 ");
        online_people=0;
        for (int i=0;i<100;i++){
            if (strcmp(users[i].id,"")!=0){
                online_people++;
                sscanf(users[i].id,"%s",tmp);
                p = sprintf(buf+p,"%s ", tmp) + p;
            }
	    }
        p = sprintf(buf+p,"\nonline:%d ", online_people) + p;
        printf("2:%s\n",buf);
        send(sender,buf,strlen(buf),0);
    }
    //指令3邀請人,回傳4給client處理
    if(instruction==3){
        char a[100],b[100];
        char buf[Maxsize];
        sscanf (message,"3 %s %s",a,b);
        int b_fd = find_fd(b);
        sprintf(buf, "4 %s invite %s\n", a,b);
        send(b_fd, buf, strlen(buf), 0);
        printf("3:%s", buf);
    }
    //同意或拒絕
    if(instruction==5){
        int state;
        char inviter[100];
        sscanf(message, "5 %s", inviter);
        send(sender, "6\n", 2, 0);
        send(find_fd(inviter), "6\n", 2, 0);
        int fd = find_fd(inviter);
        users[sender].opponent = fd;
        users[fd].opponent = sender;
        printf("6:\n");
    }
    //判斷輸贏 平手
    if(instruction==7){
        int board[9];
        char state[100];
        char buf[Maxsize];
        sscanf(message, "7  %d %d %d %d %d %d %d %d %d",&board[0],&board[1],&board[2],&board[3],&board[4],&board[5],&board[6],&board[7],&board[8]);
        for (int i=0;i<100;i++){
            state[i] = '\0';
        }
        memset(buf,'\0',Maxsize);
        memset(state,'\0',sizeof(state));
        strcat(state, users[sender].id);
        //獲勝的情況 回傳8給client
        for (int i = 0; i < 8;i++)  {
            if (board[win_column[i][0]]==board[win_column[i][1]] && board[win_column[i][1]]==board[win_column[i][2]]) {
                if (board[win_column[i][0]]!=0) {
                    strcat(state, "_Win!\n");
                    sprintf (buf,"8  %d %d %d %d %d %d %d %d %d %s\n",board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8],state);
                    printf ("7:%s",buf);
                    send(sender,buf,sizeof(buf),0);
                    send(users[sender].opponent,buf,sizeof(buf),0);
                    return;
                }
            }
        }
        memset(buf,'\0',Maxsize);
        memset(state,'\0',sizeof(state));
        //平手的情況 比到最後沒有勝負 回傳8
        zero=0;
        for(int i=0;i<9;i++){
            if(board[i]==0){
                zero++;
            }
        }
        if (zero==0) {
            strcat(state, "_Tie!\n");
            sprintf (buf,"8  %d %d %d %d %d %d %d %d %d %s\n",board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8],state);
            printf ("7:%s",buf);
            send(sender,buf,sizeof(buf),0);
            send(users[sender].opponent,buf,sizeof(buf),0);
            return;
        }
        memset(buf,'\0',Maxsize);
        memset(state,'\0',sizeof(state));
        //換人 回傳8
        strcat(state, users[users[sender].opponent].id);
        strcat(state, "_your_turn!\n");
        sprintf (buf,"8  %d %d %d %d %d %d %d %d %d %s\n",board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8],state);
        printf ("7:%s",buf);
        send(sender,buf,sizeof(buf),0);
        send(users[sender].opponent,buf,sizeof(buf),0);
    }
    if(instruction==9){
        send(sender,"10",2,0);
    }
}

void *pthread_serv(void* arg_fd){
    int fd= *(int *)arg_fd;
    while (1){
        int recv_byte=0;
        int i;
        recv_byte=recv(fd,message,Maxsize,0);
        if(recv_byte<0){
            for(i=0;i<5;i++){
                if(fd==fdt[i]){
                    fdt[i]=0;               
                }
            }
            memset(users[fd].id,'\0',sizeof(users[fd].id));
            users[fd].opponent = -1;
            break;
        }
        instruction_handle(message,fd);
        bzero(message,Maxsize);
    }
    close(fd);
}

int main(){
    int listenfd, connectfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sin_size=sizeof(struct sockaddr_in); 
    int fd;
    int number=0;
    for (int i=0;i<100;i++) {
        for (int j=0;j<100;j++){
            users[i].id[j] ='\0';
        }
        users[i].opponent= -1;
    }
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {   
        perror("Creating socket failed.");
        exit(1);
    }
    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server,sizeof(server));  

    server.sin_family=AF_INET; 
    server.sin_port=htons(PORT); 
    server.sin_addr.s_addr = htonl (INADDR_ANY); 

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) { 
        perror("Bind error.");
        exit(1); 
    }  
    if(listen(listenfd,1) == -1)  {  
        perror("listen() error\n"); 
        exit(1); 
    } 
    printf("等待客戶端連線\n");
    while(1)    {
        if ((fd = accept(listenfd,(struct sockaddr *)&client,&sin_size))==-1) {
            perror("accept() error\n"); 
            exit(1); 
        }
        if(number>=5){
            printf("客戶端已滿\n");
            close(fd);
        }
        int i;
        for(i=0;i<5;i++){
            if(fdt[i]==0){
                fdt[i]=fd;
                break;
            }
        }
        pthread_t tid;
        pthread_create(&tid,NULL,(void*)pthread_serv,&fd);
        number=number+1;
    }
    close(listenfd);            
}

