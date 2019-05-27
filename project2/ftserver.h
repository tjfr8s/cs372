/*******************************************************************************
 * Author: Tyler Freitas
 * Last Modified: 05/27/2019
 * Program Name: ftserver.c
 * Description: This program implements a file transfer server that is 
 * capable of listing the current directory contents and sending requested
 * text files from that directory.
*******************************************************************************/
#ifndef FTSERVER_H
#define FTSERVER_H
void data_connection(char* host, char* port, int* datafd);
void list_contents(int controlfd, char* dataport, char* datahost);
void get_file(int controlfd, char* filename, char* dataport, char* datahost, const char* controlport);
void parse_command(char* commandString, char** commandArray);
bool recv_command(int controlfd, const char* portNum);
int start_server(const char* portNum);
#endif
