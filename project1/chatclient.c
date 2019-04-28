#define _POSIX_C_SOURCE 200112L
#define BUFFER_SIZE 1024
#include <stdio.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <errno.h>
#include <stdbool.h>

bool send_message(int sockfd, char* user_name) {
    char message[BUFFER_SIZE]; 
    char message_tagged[BUFFER_SIZE]; 
    memset(&message, 0, sizeof(char) * BUFFER_SIZE);
    printf("%s> ", user_name);
    fgets(message, 1000, stdin);
    strtok(message, "\n");
    int len;
    int bytes_sent;
    if(errno != 0) {
        fprintf(stderr, "send: %d\n", errno);
    };

    if(!strcmp(message, "\\quit")) {
        len = strlen(message);
        bytes_sent = send(sockfd, message, len, 0);
        return false;
    } else {
        sprintf(message_tagged, "%s> %s", user_name, message);
        len = strlen(message_tagged);
        bytes_sent = send(sockfd, message_tagged, len, 0);
        return true;
    }
}

bool recv_message(int sockfd) {
    int bytes_recvd;
    char buff[BUFFER_SIZE];
    memset(&buff, 0,  sizeof(char) * BUFFER_SIZE);
    bytes_recvd = recv(sockfd, buff, sizeof(char) * BUFFER_SIZE, 0);
    if(errno!= 0) {
        fprintf(stderr, "recv: %d\n", errno);
    };

    if (!strcmp(buff, "\\quit")) {
        return true;        
    } else {
        printf("%s\n", buff);
        return false;
    }
}

int main(int argc, char* argv[]) {
    int status;
    struct addrinfo hints;
    struct addrinfo* res;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // generate address info for creating socket.
    if ((status = getaddrinfo("flip1.engr.oregonstate.edu", 
                          "30027", 
                          &hints, 
                          &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;    
    }

    struct addrinfo* p; 
    for (p = res; p != NULL; p = p->ai_next) {
        void* addr;
        char* ipver;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("    %s: %s\n", ipver, ipstr);
    }

    // create socket using address information generated above.
    int sockfd;
    int connect_stat;
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if((connect_stat = connect(sockfd, res->ai_addr, res->ai_addrlen)) != 0) {
        fprintf(stderr, "connect: %d\n", errno);
    } else {
        char user_name[25];
        memset(&user_name, 0, sizeof(char) * 25);
        printf("enter your name: ");
        fgets(user_name, 24, stdin);
        strtok(user_name, "\n");

        int user_quit = false;
        while(!user_quit && send_message(sockfd, user_name)) {
            if (recv_message(sockfd)) {
                user_quit = true;
            }
        }
    }



    freeaddrinfo(res);
    return 0;
}
