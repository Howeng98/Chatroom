#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define PORT 8000
#define BUFFER_SIZE 256

int main(int argc, char* argv[]){

    int sockfd,err;
    struct sockaddr_in server_addr;
    struct hostent* server;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("Error Opening Sockfd");
    }        

    server = gethostbyname("127.0.0.1");
    if(server == NULL){
        perror("Error No such Host");
    }
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(PORT);

    err = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(err < 0){
        perror("Connection Failed");
        exit(1);
    }

    while(1){
        bzero(buffer, BUFFER_SIZE);
        printf("Client:");
        fgets(buffer, BUFFER_SIZE, stdin);
        err = write(sockfd, buffer, strlen(buffer));
        if(err < 0){
            perror("Error on Reading");
        }
        //printf("Server:%s\n",buffer);
        bzero(buffer, BUFFER_SIZE);
        err = read(sockfd, buffer, BUFFER_SIZE);
        //printf("Err:%d\n",err);
        if(err < 0){
            perror("Error on Reading");
        }
        printf("\nServer:%s\n",buffer);
        if(strncmp("Exit", buffer, strlen("Exit")) == 0)
            break;
    }

    close(sockfd);
    return 0;
}