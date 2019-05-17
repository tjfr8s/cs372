#define _POSIX_C_SOURCE 200112L 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h> 
#include <sys/wait.h> 
#include <signal.h>

int startServer(const char* portNum) {
    struct addrinfo         hints;
    struct addrinfo*        serverInfo;
    struct sockaddr_storage theirAddr; 
    int                     status;
    int                     sockfd;
    int                     controlfd;
    socklen_t               theirAddrSize;

    // Get address info for server.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(NULL, portNum, &hints, &serverInfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status)); 
        exit(1);
    }

    // Get socket file descriptor
    sockfd = socket(serverInfo->ai_family, 
                    serverInfo->ai_socktype, 
                    serverInfo->ai_protocol);

    if (sockfd == -1) {
        fprintf(stderr, "socket() error: %s\n", strerror(errno));
        exit(1);
    }


    // Bind socket file descriptor to port.
    if (bind(sockfd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
        fprintf(stderr, "bind() error: %s\n", strerror(errno));
        exit(1);
    }

    freeaddrinfo(serverInfo);

    // Listen on port.
    if (listen(sockfd, 5) == -1) {
        fprintf(stderr, "listen() error: %s\n", strerror(errno));
        exit(1);
    }

    // Control fd
    theirAddrSize = sizeof(theirAddr);
    controlfd = accept(sockfd, (struct sockaddr*)&theirAddr, &theirAddrSize);

    return 0;

}

int main(int argc, char** argv) {
    const char* portNum = "30072";
    startServer(portNum);

    return 0;
}
