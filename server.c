// compile command: gcc -o server server.c -lpthread
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT "8888"

// use this function to seperate different type of sin_addr condition
void *get_in_addr(struct sockaddr *sockAddr)
{
    if (sockAddr->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sockAddr)->sin_addr);
    
    return &(((struct sockaddr_in6*)sockAddr)->sin6_addr);
}
int main(void)
{
    fd_set master;    
    fd_set recvFD;  
    int sockFD_num,listener,new_sockFD;
    int i,j,rv,available=1,error,result;  
    // Client Address
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    // Command to quit
    char buf[256];      
    char remoteIP[INET6_ADDRSTRLEN];
    struct addrinfo addrinfo, *temp, *request;
    
    // Initialize addrinfo for client connect
    memset(&addrinfo, 0, sizeof addrinfo);
    addrinfo.ai_family = AF_UNSPEC;
    addrinfo.ai_socktype = SOCK_STREAM;
    addrinfo.ai_flags = AI_PASSIVE;

    // memset
    FD_ZERO(&master);  
    FD_ZERO(&recvFD);
    if ((rv = getaddrinfo(NULL, PORT, &addrinfo, &temp)) != 0) {
        fprintf(stderr, "Selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    // socket and bind the first available client from the request queue
    for(request = temp; request != NULL; request = request->ai_next)
    {
        listener = socket(request->ai_family, request->ai_socktype, request->ai_protocol);
        if (listener < 0) { 
            continue;
        }                
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &available, sizeof(int));
        if (bind(listener, request->ai_addr, request->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }
    if(request == NULL) {
        fprintf(stderr, "Selectserver: failed to bind\n");
        exit(EXIT_FAILURE);
    }

    // we use temp as head of queue, after binding just release it
    freeaddrinfo(temp);
    puts("Binding successful!");

    // Listen
    if (listen(listener, 10) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Port
    printf("Listening on port:%s\n",PORT);
    FD_SET(listener, &master);
    sockFD_num = listener;

    while(1)
    {
        recvFD = master;
        if (select(sockFD_num+1, &recvFD, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }
        // Run through all client user = sockFD queue
        for(i = 0; i <= sockFD_num; i++)
        {
            if (FD_ISSET(i, &recvFD))
            {   
                if (i == listener)
                {
                    // Accept
                    addrlen = sizeof remoteaddr;
                    new_sockFD = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
                    if (new_sockFD == -1)                    
                        perror("accept");                    
                    else
                    {
                        FD_SET(new_sockFD, &master);
                        if (new_sockFD > sockFD_num)
                        {   
                            //Update the max number of sockFD
                            sockFD_num = new_sockFD;
                        }                 
                        printf("Selectserver: Connection from %s on "
                               "socket %d\n", inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN), new_sockFD);
                    }
                } 
                // check recv data from clients and remove it from master set
                else if ((error = recv(i, buf, sizeof buf, 0)) <= 0){   
                        if (error == 0)                        
                            printf("Selectserver: Socket %d dropped\n", i);                
                        else                        
                            perror("Recv");                    
                        close(i);
                        FD_CLR(i, &master);
                }
                // sending data and set it to master set
                else{                    
                    for(j = 0; j <= sockFD_num; j++)
                    {                        
                        if (FD_ISSET(j, &master) && (j != listener && j != i)) 
                        {                            
                            if (send(j, buf, error, 0) == -1)
                            {
                                perror("Send");
                            }                            
                        }
                    }
                }                
            }
        }
    }             
    return 0;
}
