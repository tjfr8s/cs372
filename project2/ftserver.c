#define _POSIX_C_SOURCE 200112L 
#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 16384

#include <stdio.h>
#include <stdbool.h>
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
#include <dirent.h>

void data_connection(char* host, char* port, int* datafd) {
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
    int connect_stat;
    *datafd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if((connect_stat = connect(*datafd, res->ai_addr, res->ai_addrlen)) != 0) {
        fprintf(stderr, "connect: %d\nerror: %s\n", errno, strerror(errno));
    } 
    // Free address struct
    freeaddrinfo(res);
}

// List directory contents: 
void list_contents(int controlfd) {
    struct dirent* entry;
    char lsBuff[BUFFER_SIZE];
    DIR* dr = opendir(".");
    bool success = true;
    char* ok = "ok";
    char* error = "error";
    int lenok = strlen(ok);
    int lenerror = strlen(error);
    int datafd = 0;

    memset(lsBuff, 0, sizeof(lsBuff));

    if (dr == NULL) {
        perror("couldn't open dir");
        success = false;
        return;
    }

    if (success) {

        send(controlfd, ok, lenok, 0);

        while ((entry = readdir(dr)) != NULL) {
            strcat(lsBuff, entry->d_name);
            strcat(lsBuff, " ");
        }

        closedir(dr);
        sleep(1);
        data_connection("flip1.engr.oregonstate.edu", "30073", &datafd);
        send(datafd, lsBuff, sizeof(lsBuff), 0);
        close(datafd);
        printf("\n");
    } else {
        send(controlfd, error, lenerror, 0);
    }
    return;
}

// Get file: 
// https://stackoverflow.com/questions/3463426/in-c-how-should-i-read-a-text-file-and-print-all-strings (reading chunks of file in c)
void get_file(int controlfd, char* filename) {
    bool success = true;
    char* ok = "ok";
    char* error = "error";
    char* done = "done";
    int lenok = strlen(ok);
    int lenerror = strlen(error);
    int lendone = strlen(done);
    FILE* fp;
    char fileBuffer[BUFFER_SIZE];
    int numRead;
    int datafd;

    memset(fileBuffer, 0, sizeof(fileBuffer));

    fp = fopen(filename, "rb");

    if (!fp) {
        perror("error accessing file\n");
        success = false;
    }

    if (success) {
        send(controlfd, ok, lenok, 0);
        sleep(1);
        data_connection("flip1.engr.oregonstate.edu", "30073", &datafd);

        while ((numRead = fread(fileBuffer, 1, sizeof(fileBuffer) - 1, fp)) > 0 ) {
            send(controlfd, ok, lenok, 0);
            sleep(1);
            send(datafd, fileBuffer, sizeof(fileBuffer), 0);
            memset(fileBuffer, 0, sizeof(fileBuffer));
        }

    } else {
        send(controlfd, error, lenerror, 0);
    }
}

// Parse command
void parse_command(char* commandString, char** commandArray){
    char* token = strtok(commandString, " ");
    int i = 0;
    while (token != NULL) {
        commandArray[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
}


// Receive and handle the command
bool recv_command(int controlfd) {
    char    commandBuff[BUFFER_SIZE];
    char*   commands[2] = {NULL};
    memset(&commandBuff, 0,  sizeof(char) * BUFFER_SIZE);

    // Receive command through the socket.
    recv(controlfd, commandBuff, sizeof(char) * BUFFER_SIZE, 0);
    if(errno!= 0) {
        fprintf(stderr, "recv: %d\n", errno);
    };

    // parse the command 
    parse_command(commandBuff, commands);


    /*
    int i;
    for (i = 0; i < 2; i++) {
        if(commands[i] != NULL) {
            printf("command%d: %s\n", i, commands[i]);
        }
    }
    */

    if (strcmp(commands[0], "-l") == 0) { 
        printf("list contents\n");
        list_contents(controlfd);
    } else if (strcmp(commands[0], "-g") == 0) {
        printf("get file\n");
        get_file(controlfd, commands[1]);
    }





    return true;
}

// Start server and listen for command
int start_server(const char* portNum) {
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
    int one = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

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

    printf("waiting for message...\n");
    // Listen on port.
    if (listen(sockfd, 5) == -1) {
        fprintf(stderr, "listen() error: %s\n", strerror(errno));
        exit(1);
    }

    while(true) {
        // Control fd
        theirAddrSize = sizeof(theirAddr);
        controlfd = accept(sockfd, (struct sockaddr*)&theirAddr, &theirAddrSize);
        recv_command(controlfd);
        close(controlfd);
    }
    close(sockfd);

    return 0;

}


int main(int argc, char** argv) {
    const char* portNum = "30072";
    start_server(portNum);

    return 0;
}
