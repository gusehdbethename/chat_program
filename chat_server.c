#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//  message size
#define BUF_SIZE 1024  

//  Structure to hold two sockets
struct sock_args{
    int clnt1_sock;
    int clnt2_sock;
}typedef SOCK_ARGS;

//  packet send thread functions
void *send_client1(void *args);
void *send_client2(void *args);

int main(int argc, char *argv[]){
    int serv_sock;  //  server_socket
    SOCK_ARGS sock; //  two client socket

    struct sockaddr_in serv_adr;    //  server socket address

    int opt = 1;    //  used to setsockopt() function
    socklen_t adr_size = sizeof(serv_adr);  //  used to accept

    //  packet send threads
    pthread_t send_clnt1_th;
    pthread_t send_clnt2_th;
    
    if(argc != 2){
        printf("Usage : %s <PORT>\n",argv[0]);
        exit(1);
    }

    //  socket create
    serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serv_sock < 0){
        perror("socket() error");
        exit(1);
    }

    //  socket option set
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt() error");
        exit(EXIT_FAILURE);
    }

    //  socket address set
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //  socket binding
    if(bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0){
        perror("bind() error");
        exit(1);
    }

    //  socket listening
    if(listen(serv_sock, 2) < 0){
        perror("listen() error");
        exit(1);
    }

    //  client accpet
    sock.clnt1_sock = accept(serv_sock, (struct sockaddr *)&serv_adr, &adr_size);
    if(sock.clnt1_sock < 0){
        perror("accept() client1 error");
        exit(1);
    }else{
        printf("Client 1 Connected......\n");
    }

    //  another client accpet
    sock.clnt2_sock = accept(serv_sock, (struct sockaddr *)&serv_adr, &adr_size);
    if(sock.clnt2_sock < 0){
        perror("accept() client2 error");
        exit(1);
    }else{
        printf("Client 2 Connected......\n");
    }

    //  packet relay threads start
    pthread_create(&send_clnt1_th, NULL, send_client1, (void *)&sock);
    pthread_create(&send_clnt2_th, NULL, send_client2, (void *)&sock);

    pthread_join(send_clnt1_th, NULL);
    pthread_join(send_clnt2_th, NULL);
    
    close(sock.clnt1_sock);
    close(sock.clnt2_sock);
    close(serv_sock);
    return 0;
}

//  receive packet from client and send them to another client 
void *send_client1(void *args){
    SOCK_ARGS *sock = (SOCK_ARGS *)args;
    char message[BUF_SIZE];
    int str_len;

    while(1){
        memset(message, 0, sizeof(message));   

        while((str_len = read(sock->clnt2_sock, message, BUF_SIZE)) != 0)
            write(sock->clnt1_sock, message, str_len);
    }

    return 0;
}

//  receive packet from client and send them to another client 
void *send_client2(void *args){
    SOCK_ARGS *sock = (SOCK_ARGS *)args;
    char message[BUF_SIZE];
    int str_len;

    while(1){
        memset(message, 0, sizeof(message));   

        while((str_len = read(sock->clnt1_sock, message, BUF_SIZE)) != 0)
            write(sock->clnt2_sock, message, str_len);
    }

    return 0;
}
