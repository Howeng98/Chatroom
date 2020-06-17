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

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master;    
    fd_set recvFD;  
    int sockFD_num,listener,new_sockFD;
    int i,j,rv,available=1,error;  
    struct sockaddr_storage remoteaddr; //client address
    socklen_t addrlen;

    char buf[256];      
    char remoteIP[INET6_ADDRSTRLEN];
    struct addrinfo addrinfo, *temp, *request;
    
    memset(&addrinfo, 0, sizeof addrinfo);
    addrinfo.ai_family = AF_UNSPEC;
    addrinfo.ai_socktype = SOCK_STREAM;
    addrinfo.ai_flags = AI_PASSIVE;
    FD_ZERO(&master);  
    FD_ZERO(&recvFD); 
    if ((rv = getaddrinfo(NULL, PORT, &addrinfo, &temp)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
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

    freeaddrinfo(temp);
    puts("Binding successful!");
    // listen
    if (listen(listener, 10) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
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

        for(i = 0; i <= sockFD_num; i++)
        {
            if (FD_ISSET(i, &recvFD))
            {   // we got one
                if (i == listener)
                {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    new_sockFD = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (new_sockFD == -1)                    
                        perror("accept");                    
                    else
                    {
                        FD_SET(new_sockFD, &master);
                        if (new_sockFD > sockFD_num)
                        {   // keep track of the max
                            sockFD_num = new_sockFD;
                        }
                        printf("selectserver: new connection from %s on "
                               "socket %d\n", inet_ntop(remoteaddr.ss_family,
                               get_in_addr((struct sockaddr*)&remoteaddr),
                               remoteIP, INET6_ADDRSTRLEN), new_sockFD);
                    }
                } 
                else if ((error = recv(i, buf, sizeof buf, 0)) <= 0){   
                        if (error == 0)                        
                            printf("Selectserver: socket %d dropped\n", i);                
                        else                        
                            perror("Recv");                    
                        close(i);
                        FD_CLR(i, &master);
                }
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
