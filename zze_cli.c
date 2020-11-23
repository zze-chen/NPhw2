#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 1025
#define MAXDATASIZE 100

char sendbuf[1024];
char recvbuf[1024];
char name[100];
int fd;
int board[9];
int zero=0;
int turn=0;
void print_board(int *board){
    printf("┌────┬────┬────┐        ┌───┬───┬───┐\n");
    printf("│ -0 │ -1 │ -2 │        │ %d │ %d │ %d │\n", board[0], board[1], board[2]);
    printf("├────┼────┼────┤        ├───┼───┼───┤\n");
    printf("│ -3 │ -4 │ -5 │        │ %d │ %d │ %d │\n", board[3], board[4], board[5]);
    printf("├────┼────┼────┤        ├───┼───┼───┤\n");
    printf("│ -6 │ -7 │ -8 │        │ %d │ %d │ %d │\n", board[6], board[7], board[8]);
    printf("└────┴────┴────┘        └───┴───┴───┘\n");
}

void write_on_board(int *board, int location){
    print_board(board);
    zero=0;
    for(int i=0;i<8;i++){
        if(board[i]==0){
            zero++;
        }
    }
    if(zero%2==0){
        turn=1;
    }else{
        turn=2;
    }
    printf("%d\n",turn);
    board[location] = turn;
    sprintf(sendbuf, "7  %d %d %d %d %d %d %d %d %d\n", board[0], board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8]);
}

void pthread_recv(void* ptr)
{
    int instruction;
    while(1)
    {
        memset(sendbuf,0,sizeof(sendbuf));
        instruction = 0;
        if ((recv(fd,recvbuf,MAXDATASIZE,0)) == -1)
        {
            printf("recv() error\n");
            exit(1);
        }
        sscanf (recvbuf,"%d",&instruction);
        //printf("%d",instruction);
        if(instruction==1){
            char name_register[100];
            sscanf (recvbuf,"%d %s",&instruction,name_register);
            printf("登錄成功\n");
            printf("登錄名: %s\n",name_register);
        }
        if(instruction==2){
	        printf("%s\n", &recvbuf[2]);
        }
        if(instruction==4){
            char inviter[100];
            sscanf(recvbuf,"%d %s",&instruction, inviter);
            printf("%s\n", &recvbuf[2]); 
            printf("accept, input:5 %s\n", inviter);
            printf("reject, input:9 %s\n", inviter);	
        }
        if(instruction==6){
            printf("遊戲開始!\n");
            printf("Inviter is 1\n");
            printf("Invitee is 2\n");
            printf("Inviter go first.\n");
            print_board(board);
        }
        if(instruction==8){
            char msg[100];
            sscanf (recvbuf,"%d %d%d%d%d%d%d%d%d%d %s",&instruction, &board[0],&board[1],&board[2],&board[3],&board[4],&board[5],&board[6], &board[7],&board[8], msg);
            print_board(board);
            printf("%s\n", msg);
            printf("Please input:-0~8\n");
        }
        if(instruction==10){
            printf("拜託拉 玩一下\n");
        }
        memset(recvbuf,0,sizeof(recvbuf));
    }
}

int main(int argc, char *argv[])
{
    int  numbytes;
    char buf[MAXDATASIZE];
    struct hostent *host;
    struct sockaddr_in server;

    if (argc !=2)
    {
        printf("Usage: %s <IP Address>\n",argv[0]);
        exit(1);
    }


    if ((host=gethostbyname(argv[1]))==NULL)
    {
        printf("gethostbyname() error\n");
        exit(1);
    }

    if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        printf("socket() error\n");
        exit(1);
    }

    bzero(&server,sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)host->h_addr);
    if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
    {
        printf("connect() error\n");
        exit(1);
    }

    printf("connect success\n");

    // usage
    printf("##      ## ######## ##        ######   #######  ##     ## ########\n"); 
    printf("##  ##  ## ##       ##       ##    ## ##     ## ###   ### ##   \n");        
    printf("##  ##  ## ##       ##       ##       ##     ## #### #### ##\n");        
    printf("##  ##  ## ######   ##       ##       ##     ## ## ### ## ###### \n");   
    printf("##  ##  ## ##       ##       ##       ##     ## ##     ## ## \n");       
    printf("##  ##  ## ##       ##       ##    ## ##     ## ##     ## ##  \n");      
    printf(" ###  ###  ######## ########  ######   #######  ##     ## ######## \n\n");
    printf("  #######  ##     ##     ######  ##     ## ########  ######   ######      \n");
    printf(" ##     ##  ##   ##     ##    ## ##     ## ##       ##    ## ##    ##     \n");
    printf(" ##     ##   ## ##      ##       ##     ## ##       ##       ##           \n");
    printf(" ##     ##    ###       ##       ######### ######    ######   ######      \n");
    printf(" ##     ##   ## ##      ##       ##     ## ##             ##       ##     \n");
    printf(" ##     ##  ##   ##     ##    ## ##     ## ##       ##    ## ##    ##     \n");
    printf("  #######  ##     ##     ######  ##     ## ########  ######   ######      \n");
    printf("┌─────────┐┌────────────────────────────────────────────────┐\n");
    printf("│    1    ││input 1 name to assign your name                │\n");
    printf("│    2    ││input 2 to show all the client                  │\n");
    printf("│    3    ││input 3 your_name others_name to invite other   │ \n");
    printf("│ logout  ││input logout to logout                          │\n");
    printf("└─────────┘└────────────────────────────────────────────────┘\n");

    pthread_t tid;
    pthread_create(&tid, NULL, (void*)pthread_recv, NULL);
    while(1){
        memset(sendbuf,0,sizeof(sendbuf));
        fgets(sendbuf,sizeof(sendbuf),stdin);   
        int location;
        if(sendbuf[0] == '-'){
            sscanf(&sendbuf[1], "%d", &location);
            write_on_board(board, location);
        }
        send(fd,sendbuf,(strlen(sendbuf)),0);  
        // Logout
        if(strcmp(sendbuf,"logout\n")==0){          
            memset(sendbuf,0,sizeof(sendbuf));
            printf("登出\n");
            return 0;
        }
    }
    pthread_join(tid,NULL);
    close(fd);
    return 0;
}
