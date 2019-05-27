#!/bin/python3

# Author: Tyler Freitas
# Last Modified: 05/27/2019
# Program Name: ftclient.py
# Description: This program implements a file transfer client to be used with
# ftserver.c. It is capable of requesting the directory contents of the server
# as well as file that reside in that directory.
import socket
import sys

def send_command(control_socket, command, host, port):
    """
    Description: Handles the sending of a command to the server.

    Preconditions: The function receives a socket for sending the command
    as well as the command itself, and the host:port that describe the socket.

    Post conditions: The passed command is sent to the server via the passed
    socket. If the command is successful the funciton returns true, else
    it returns false.
    """
    # Join command components into a string.
    control_socket.sendall(" ".join(command).encode())
    # Send command and wait for response.
    response = control_socket.recv(1024)
    response = response.decode()

    # Handle potential error responses.
    if response == "ok":
        return True
    else:
        print(f'{host}:{port} says {response}')
        return False;

def create_control_socket(host, port):
    """
    Description: This function builds a socket for the passed host, port

    Preconditions: The user passes a host and port for the socket.

    Postconditions: the function returns a socket described by the host:port
    tuple.
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    return s

def create_data_socket(host, server_port):
    """
    Description: Creates a socket, binds it to a port, and listens for connections.

    Preconditions: host, port, and username are passed to function

    Postconditions:  a socket is created and used to exchange messages.
    """

    # Bind socket to target address and port
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, server_port))
        s.listen(1)
        conn, addr = s.accept()

        return conn

def recv_list_directory(conn):
    """
    Description: this function receives and prints the result of a request
    for directory contents to the server.

    Preconditions: a request for directory contents has been made to the server
    and the connection on which it was requested is passed to this function.

    Postconditions: the function receives the results and prints them.
    """
    message = conn.recv(1024).decode();
    print(message)
    return True

def recv_file(control_socket, conn, filename):
    """
    Description: this function receives the file request from the server. 
    
    Preconditions: a request for a file has been made to the server
    and the connection on which it was requested is passed to this function
    along with the control_socket and filename.

    Postconditions: The function receives chunks of the requested file and
    appends them to the output file in the current directory.
    """
    filename = "./" + filename 
    with open(filename, "w") as ofile:
        # Keep receiving file contents until the server says the entire file
        # has been transferred.
        while (control_socket.recv(1024).decode() == 'ok'):
            ofile.write(conn.recv(1024).decode())
        print("File transfer complete.")
    return True

def parse_args(argv):
    """
    Description: This function parses the commandline arguments, builds a control
    socket, sends the command, and handles the response from the server.

    Preconditions: Commandline arguments are passed to the function.
    
    Postconditions: The passed command is sent to the server and the results
    of that command are handled and an error is displayed if it wasn't successful.
    """
    DATA_HOST = ''   
    CONTROL_HOST = argv[1]
    CONTROL_PORT = int(argv[2])
    command = [argv[3]]
    if command[0] == "-l":
        DATA_PORT = int(argv[4])
        command.append(str(DATA_PORT))
        command.append(socket.gethostname())
    elif command[0] == "-g":
        command.append(argv[4])
        DATA_PORT = int(argv[5])
        command.append(str(DATA_PORT))
        command.append(socket.gethostname())
    else:
        print("incorrect command format")
        return
    
    control_socket = create_control_socket(CONTROL_HOST, CONTROL_PORT)
    was_successful = send_command(control_socket, command, CONTROL_HOST, CONTROL_PORT) 

    if was_successful and command[0] == "-l":
        #create data socket
        conn = create_data_socket(DATA_HOST, DATA_PORT)
        #recv_list_directory
        print(f'Receiving directory structure from {CONTROL_HOST}:{DATA_PORT}')
        recv_list_directory(conn);
    elif was_successful and command[0] == "-g":
        print(f'Receiving "{command[1]}" from {CONTROL_HOST}:{DATA_PORT}')
        #create data socket
        #recv_file
        conn = create_data_socket(DATA_HOST, DATA_PORT)
        recv_file(control_socket, conn, command[1]);


    control_socket.close()


if __name__ == '__main__':
    parse_args(sys.argv)

