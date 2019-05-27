/*******************************************************************************
 * Author: Tyler Freitas
 * Last Modified: 05/27/2019
 * Program Name: ftserver.c
 * Description: This program implements a file transfer server that is 
 * capable of listing the current directory contents and sending requested
 * text files from that directory.
*******************************************************************************/
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
#include "ftserver.h"

int main(int argc, char** argv) {
    if (argc > 2) {
        printf("Error, too many arguments\n");
    } 

    start_server(argv[1]);
    return 0;
}

/*******************************************************************************
 * Description: This function creates a socket for sending data from the ft
 * server. It passes the resulting file descriptor out of the function via
 * datafd.
 *
 * Preconditions: The function is passed a host url and port to build the 
 * socket for, as well as a location to store the resulting file descriptor.
 *
 * Postconditions: A socket corresponding to host:port will be created and
 * the referenced by the file descriptor stored in datafd.
*******************************************************************************/
void data_connection(char* host, char* port, int* datafd) {
    int status;
    struct addrinfo hints;
    struct addrinfo* res;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Generate address info for creating data socket.
    if ((status = getaddrinfo(host, 
                          port, 
                          &hints, 
                          &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    }

    struct addrinfo* p; 
    for (p = res; p != NULL; p = p->ai_next) {
        void* addr;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    }

    // Create data socket using address information generated above.
    int connect_stat;
    *datafd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if((connect_stat = connect(*datafd, res->ai_addr, res->ai_addrlen)) != 0) {
        fprintf(stderr, "connect: %d\nerror: %s\n", errno, strerror(errno));
    } 
    // Free address struct
    freeaddrinfo(res);
}

/*******************************************************************************
 * Description: This function handles requests to the server to list the
 * current directory contents. It receives a file descriptor to send errors and
 * acks, as well as host and prot values for building a data socket.
 *
 * Preconditions: The user must pass in a valid control file descriptor to 
 * controlfd and the desired port and host values for the data socket to 
 * dataport and data host.
 *
 * Postconditions: The function ack's the request for directory contents on
 * control fd and sends the results on a newly created data socket described
 * by dataport and datahost.
*******************************************************************************/
void list_contents(int controlfd, char* dataport, char* datahost) {
    struct dirent* entry;
    char lsBuff[BUFFER_SIZE];
    DIR* dr = opendir(".");
    bool success = true;
    char* ok = "ok";
    char* error = "ERROR LISTING CONTENTS";
    int lenok = strlen(ok);
    int lenerror = strlen(error);
    int datafd = 0;

    memset(lsBuff, 0, sizeof(lsBuff));
    printf("Connection from %s\n", datahost);
    printf("List directory requested on port %s\n", dataport);

    // Handle errors by setting success flag.
    if (dr == NULL) {
        perror("couldn't open dir");
        success = false;
        return;
    }

    if (success) {

        printf("Sending directory contents to %s:%s", datahost, dataport);
        // Ack the request for dir contents so client knows to listen for res.
        send(controlfd, ok, lenok, 0);

        // Build list of files in directory.
        while ((entry = readdir(dr)) != NULL) {
            strcat(lsBuff, entry->d_name);
            strcat(lsBuff, " ");
        }

        closedir(dr);
        sleep(1);
        // Build a data file descriptor for the passed dataport and datahost
        // values.
        data_connection(datahost, dataport, &datafd);
        // Send results
        send(datafd, lsBuff, sizeof(lsBuff), 0);
        close(datafd);
        printf("\n");
    } else {
        send(controlfd, error, lenerror, 0);
    }
    return;
}

/*******************************************************************************
 * Description: This file handles the requests to the server for a file. It
 * interfaces with the client through controlfd and sends the client the 
 * requested file (filename) over a new socket described by datahost:dataport.
 *
 * Preconditions: A valid control socket descriptor is passed to the function 
 * along with a valid url:port combaination for the data socket. A file name
 * and control port are also require.
 *
 * Postconditions: If the file is not found, an error is reported to the client
 * through the controlfd. If the file is found, it is sent in chunks to the 
 * client through the data socket described by datahost:dataport.
*******************************************************************************/
void get_file(int controlfd, char* filename, char* dataport, char* datahost, const char* controlport) {
    bool success = true;
    char* ok = "ok";
    char* error = "FILE NOT FOUND";
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

    printf("Connection from %s\n", datahost);
    printf("File \"%s\" requested on port %s\n", filename, dataport);

    // Record error if file isn't found.
    if (!fp) {
        printf("File not found. Sending error message to %s:%s\n", datahost, controlport);
        success = false;
    }

    if (success) {
        printf("Sending file \"%s\" to %s:%s\n", filename, datahost, dataport);
        send(controlfd, ok, lenok, 0);
        sleep(1);
        data_connection(datahost, dataport, &datafd);

        // Send chunks of file until the entire file has been transferred.
        while ((numRead = fread(fileBuffer, 1, sizeof(fileBuffer), fp)) > 0 ) {
            send(controlfd, ok, lenok, 0);
            sleep(1);
            send(datafd, fileBuffer, sizeof(fileBuffer), 0);
            memset(fileBuffer, 0, sizeof(fileBuffer));
        }
        send(controlfd, done, lendone, 0);
        close(datafd);
    } else {
        // If the file wasn't found, send an error to the control socket.
        send(controlfd, error, lenerror, 0);
    }

}

/*******************************************************************************
 * Description: This function parses the commands received by the server.
 *
 * Preconditions: The function receives a command string and an array to store
 * the tokenized commands in.
 *
 * Postconditions: The function splits the commandString at each space 
 * character and stores each command in the commandArray.
*******************************************************************************/
void parse_command(char* commandString, char** commandArray){
    char* token = strtok(commandString, " ");
    int i = 0;
    while (token != NULL) {
        commandArray[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
}

/*******************************************************************************
 * Description: This function orchestrates the handling of commands sent to 
 * the sever.
 *
 * Preconditions: a file descriptor for the control socket is passed to the
 * function along with a port number for controlfd for error logging.
 *
 * Postconditions: The command string will be tokenized and the correct 
 * command will be performed based on the flag set in the tokenized command 
 * (-l for listing directories and -g for getting files). 
*******************************************************************************/
bool recv_command(int controlfd, const char* portNum) {
    char    commandBuff[BUFFER_SIZE];
    char*   commands[4] = {NULL};
    
    memset(&commandBuff, 0,  sizeof(char) * BUFFER_SIZE);

    // Receive command through the socket.
    recv(controlfd, commandBuff, sizeof(char) * BUFFER_SIZE, 0);

    // parse the command 
    parse_command(commandBuff, commands);

    // Call appropriate function for passed command.
    if (strcmp(commands[0], "-l") == 0) { 
        list_contents(controlfd, commands[1], commands[2]);
    } else if (strcmp(commands[0], "-g") == 0) {
        get_file(controlfd, commands[1], commands[2], commands[3], portNum);
    }

    return true;
}

/*******************************************************************************
 * Description: This function starts the server by making it listen for 
 * connections on the passed portNum. The server will wait for connections
 * on the specified port and, upon receiving one, will enter a loop for
 * receiving and handling commands.
 *
 * Preconditions: A port number is passed to the funciton for starting the
 * server.
 *
 * Postconditions: The server enters the command handling loop when a user 
 * connects to the port it is listening on. On termination the server
 * closes its sockets.
*******************************************************************************/
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

    printf("Server open on %s\n", portNum);
    // Listen on port.
    if (listen(sockfd, 5) == -1) {
        fprintf(stderr, "listen() error: %s\n", strerror(errno));
        exit(1);
    }

    while(true) {
        // Loop for handling each command sent by the user.
        theirAddrSize = sizeof(theirAddr);
        controlfd = accept(sockfd, (struct sockaddr*)&theirAddr, &theirAddrSize);
        recv_command(controlfd, portNum);
        close(controlfd);
    }
    close(sockfd);

    return 0;

}
