#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define PORT 8000
#define BUFFER_SIZE 256
#define MAX_CLIENT_NUM 5
static int client_num = 0,uid = 10;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

//define a client struct and for distinguish each client
typedef struct {
    struct sockaddr_in address;
    int uid;
    int sockfd;
    char username[10];
} client_t;

//create a array to store all client's info
client_t *clients_array[MAX_CLIENT_NUM];

void push(client_t *client){
    pthread_mutex_lock(&clients_mutex);
    for(int i=0;i<MAX_CLIENT_NUM;i++){
        if(!clients_array[i]){
            clients_array[i] = client;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void pop(int uid){
    pthread_mutex_lock(&clients_mutex);
    for(int i=0;i<MAX_CLIENT_NUM;i++){
        if(clients_array[i]){
            if(clients_array[i]->uid == uid){
                clients_array[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_msg(int uid,char* msg){
    pthread_mutex_lock(&clients_mutex);
    int error;
    for(int i=0;i<MAX_CLIENT_NUM;i++){
        if(clients_array[i] != NULL){
            if(clients_array[i]->uid != uid){
                error = write(clients_array[i]->sockfd, msg, strlen(msg));
                if(error < 0){
                    perror("Error on Writing");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


int main(int argc,char* argv[]){
    int sockfd, newsockfd,err;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr,client_addr;
    socklen_t len;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("Error opening Socket");
    }

    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    err = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(err < 0){
        perror("Binding Failed");
        exit(1);
    }
    listen(sockfd, 5);
    len = sizeof(client_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &len);
    
    if(newsockfd < 0){
        perror("Error on Accept");
        exit(1);
    }
    while(1){
        printf("======Welcome to the Chatroom=======\n");
        bzero(buffer,BUFFER_SIZE);
        err = read(newsockfd, buffer, BUFFER_SIZE);
        if(err < 0){
            perror("Error on Reading");
            exit(1);
        }
        printf("\nClient:%s\n",buffer);
        
        bzero(buffer,BUFFER_SIZE);
        printf("Server:");
        fgets(buffer, BUFFER_SIZE, stdin);
        err = write(newsockfd, buffer, strlen(buffer));
        //printf("Err:%d\n",err);
        if(err < 0){
            perror("Error on Writing");
            exit(1);
        }
        if(strncmp("Exit", buffer, strlen("Exit")) == 0)
            break;
    }
    close(newsockfd);
    close(sockfd);
    return 0;
}