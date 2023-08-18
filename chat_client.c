#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024   //  message size

void *recv_msg(void *arg);  //  receive packet thread function
void *send_msg(void *arg);  //  send packet thread function

int main(int argc, char *argv[]){
    int sock;   //  server_socket
    struct sockaddr_in serv_adr;    //  server socket address 
    pthread_t send_th, recv_th;     //  send, receive message threads

    if(argc != 3){
        printf("Usage : %s <IP> <port>\n",argv[0]);
        exit(1);
    }

    //  socket create
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(socket < 0){
        perror("socket() error");
        exit(1);
    }

    //  socket address set
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    //  connect to server socket 
    if(connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0){
        perror("connect() error");
        exit(1);
    }else{
        printf("Connected........\n");
    }

    //  packet receive, send thread start 
    pthread_create(&recv_th, NULL, recv_msg, (void *)&sock);
    pthread_create(&send_th, NULL, send_msg, (void *)&sock);

    pthread_join(recv_th, NULL);
    pthread_join(send_th, NULL);

    close(sock);
}    

//  input the message, and send them to server
void *send_msg(void *arg){
    int sock = *(int *)arg;
    char message[BUF_SIZE];
    int str_len, recv_cnt, recv_len;
    int i;

    while(1){    
        memset(message, 0, BUF_SIZE);

        fgets(message, BUF_SIZE, stdin);
        message[strlen(message) - 1] = 0;   //  new line delete
        
        str_len = strlen(message);

        for(i=str_len; i>=1; i--){
            message[i] = message[i - 1]; 
        }
        message[0] = (char)str_len;

        write(sock, message, str_len + 1);
    }
}

//  receive message from server, and show them in terminal
void *recv_msg(void *arg){
    int sock = *(int *)arg;
    char message[BUF_SIZE];
    int str_len, recv_len, recv_cnt;

    while(1){
        memset(message, 0, BUF_SIZE);

        recv_len = read(sock, &str_len, 1);
        if(recv_len < 0){
            perror("read() error");
            exit(1);
        }
        recv_len = 0;

        while(recv_len < str_len){
            recv_cnt = read(sock, message, str_len);

            if(recv_cnt < 0){
                perror("read() error");
                exit(1);
            }
            recv_len += recv_cnt;
        }
        message[recv_len] = 0;

        printf("(other people) : %s\n",message);
    }
}
