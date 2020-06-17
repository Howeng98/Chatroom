//compile command : gcc -o client client.c -lpthread
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT "8888"
#define MAXDATASIZE 256 // max number of bytes we can get at once 
#define MAXNAMESIZE 10

// this function is used to let the thread listening to the server and recv the data the is sent from the server and print it out
void *receiveHandler(void *sock_fd)
{
    //int* sFd = (int*) sock_fd;
	int sockFD = (intptr_t) sock_fd;
    char buffer[MAXDATASIZE];
    int num;    
    while(1)
    {
        memset(&buffer,sizeof(buffer),0);
        if ((num = recv(sockFD, buffer, MAXDATASIZE-1, 0)) == -1)
        {
            perror("Error");
            exit(1);
        }
        else{
            buffer[num] = '\0';            
        }
        printf("%s", buffer);
    }
}

void *get_in_addr(struct sockaddr *sock_addr)
{
    if (sock_addr->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sock_addr)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sock_addr)->sin6_addr);
}

int main(int argc, char *argv[])
{
    char message[MAXDATASIZE];
    char username[MAXNAMESIZE];    
    char s[INET6_ADDRSTRLEN];
    int sockfd;//, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo addrInfo, *serverInfo, *request;
    int error,count;

    memset(&addrInfo, 0, sizeof(addrInfo));
    addrInfo.ai_family = AF_UNSPEC;
    addrInfo.ai_socktype = SOCK_STREAM;

    if ((error = getaddrinfo("127.0.0.1", PORT, &addrInfo, &serverInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(request = serverInfo; request != NULL; request = request->ai_next) {
        if ((sockfd = socket(request->ai_family, request->ai_socktype,request->ai_protocol)) == -1) {
            perror("sockFD failed");
            continue;
        }

        if (connect(sockfd, request->ai_addr, request->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect failed");
            continue;
        }
        break;
    }

    if (request == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return EXIT_FAILURE;
    }

    inet_ntop(request->ai_family, get_in_addr((struct sockaddr *)request->ai_addr),s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(serverInfo);
    
    puts("Username:");
    memset(&username, sizeof(username),0);
    memset(&message, sizeof(message),0); 
    fgets(username, MAXNAMESIZE, stdin);
    
    // get a new thread and recv msg
    pthread_t recv_thread;    
    
    if(pthread_create(&recv_thread, NULL, receiveHandler, (void*)(intptr_t) sockfd) < 0)
    {   
        perror("create thread failed");
        return EXIT_FAILURE;
    }    
        
    //send message to server:
    puts("Connected\n");
    puts("/QUIT to quit");
    
    while(1)
	{
		char temp[6];
		memset(&temp, sizeof(temp),0);
        memset(&buf, sizeof(buf),0);        
        fgets(buf, 100, stdin);

        // quit chatroom when they enter "/QUIT"
		if(buf[0] == '/' && buf[1] == 'Q' && buf[2] == 'U' && buf[3] == 'I' && buf[4] == 'T')
			return 1;		

        count = 0;
        // String processing , add ":" when output
        while(count < strlen(username))
        {
            message[count] = username[count];
            count++;
        }
        count--;
        message[count] = ':';
        count++;
		
        // update message content
        for(int i = 0; i < strlen(buf); i++)
        {
            message[count] = buf[i];
            count++;
        }
        message[count] = '\0';        
        // sending
        if(send(sockfd, message, strlen(message), 0) < 0)
        {
            puts("Send failed");
            return EXIT_FAILURE;
        }
        // initialize buffer
        memset(&buf, sizeof(buf),0);        
    }

    // close socket
    puts("Closing socket connection");
    pthread_join(recv_thread , NULL);
    close(sockfd);

    return 0;
}
