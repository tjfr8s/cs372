/*
 * Author: Tyler Freitas
 * Last Modified: 05/01/2019
 * Program Name: chatclient.c
 * Description: This program implements a chat client that creates a tcp
 * socket with the server address and port combination specified as the first
 * two command line arguments respectively.
 * Course: cs372
 *
 */
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

/*
 * This function allows the user to send a message via the TCP socket using
 * stdin.
 *
 * @param: sockfd       file descriptor of the tcp socket being used.
 * @param: user_name    user name string to prepend to messages. 
 *
 * @return              the function returns false if the user quits and true
 *                      otherwise.
 *
 * @preconditions:      
 *  - a socket has been set up and its file descriptor is passed to the function
 *
 * @postconditions:
 *  - the user's message is sent through the socket
 */
bool send_message(int sockfd, char* user_name) {
    char message[BUFFER_SIZE]; 
    char message_tagged[BUFFER_SIZE]; 
    memset(&message, 0, sizeof(char) * BUFFER_SIZE);

    // Get user name
    printf("%s> ", user_name);
    fgets(message, 1000, stdin);
    strtok(message, "\n");
    int len;

    // Send a message to the socket and handle the case in which the user 
    // chooses to quit.
    if(!strcmp(message, "\\quit")) {
        len = strlen(message);
        send(sockfd, message, len, 0);
        return false;
    } else {
        sprintf(message_tagged, "%s> %s", user_name, message);
        len = strlen(message_tagged);
        send(sockfd, message_tagged, len, 0);
        return true;
    }
    if(errno != 0) {
        fprintf(stderr, "send: %d\n", errno);
    };
}

/*
 * This function receives a message through the tcp socket.
 *
 * @param: sockfd       file descriptor of the tcp socket being used.
 *
 * @return              the function returns true if the user quits and false 
 *                      otherwise.
 *
 * @preconditions:      
 * - a socket has been set up and its file descriptor is passed to the function
 *
 * @postconditions:
 * - the program waits to receive a message through the socket, then displays
 *   the received message. 
 */
bool recv_message(int sockfd) {
    char buff[BUFFER_SIZE];
    memset(&buff, 0,  sizeof(char) * BUFFER_SIZE);

    // Receive a message through the socket.
    recv(sockfd, buff, sizeof(char) * BUFFER_SIZE, 0);
    if(errno!= 0) {
        fprintf(stderr, "recv: %d\n", errno);
    };

    // Detect whether the remote user chose to quit.
    if (!strcmp(buff, "\\quit")) {
        return true;        
    } else {
        printf("%s\n", buff);
        return false;
    }
}

void start_connection(char* host, char* port) {
    int status;
    struct addrinfo hints;
    struct addrinfo* res;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Generate address info for creating socket.
    printf("Connecting to %s on port %s", host, port);
    if ((status = getaddrinfo(host, 
                          port, 
                          &hints, 
                          &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
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

    // Create socket using address information generated above.
    int sockfd;
    int connect_stat;
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if((connect_stat = connect(sockfd, res->ai_addr, res->ai_addrlen)) != 0) {
        fprintf(stderr, "connect: %d\n", errno);
    } else {
        char user_name[25];
        memset(&user_name, 0, sizeof(char) * 25);
        printf("enter your name: ");
        fgets(user_name, 10, stdin);
        strtok(user_name, "\n");

        // Alternate sending and receivng of messages until someone quits.
        int user_quit = false;
        while(!user_quit && send_message(sockfd, user_name)) {
            if (recv_message(sockfd)) {
                user_quit = true;
            }
        }
    }


    // Free address struct

    freeaddrinfo(res);
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("You did not specify host and port correctly");
    } else {
        start_connection(argv[1], argv[2]);
    }
    return 0;
}
