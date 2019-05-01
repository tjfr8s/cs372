"""
Author: Tyler Freitas
Date: 05/01/2019
Description: This program creates a chat server listening on the port specified
as the first (and only) command line argument.
"""
import socket
import sys

def send_message(server_socket_connection, user_name):
    """
    Gets user input and sends it to the passed socket as a message
    with the user's name attached.

    @param  server_socket_connection    target socket connection.
    @param  user_name                   user name string to be added to 
                                        messages.

    @return                             returns true if the user quits and
                                        false otherwise.
    """
    message = input(f'{user_name}> ')
    if not message == '\quit':
        message_with_handle = f'{user_name}> {message}'
        message_with_handle = message_with_handle.encode()
        server_socket_connection.send(message_with_handle)
        return True
    else:
        message = message.encode()
        server_socket_connection.send(message)
        return False

def exchange_messages(server_socket_connection, user_name):
    """
    Coordinates the exchange of messages through the socket.

    @param  server_socket_connection    target socket connection.
    @param  user_name                   user name string to be added to 
                                        messages.

    """
    while True:
        data = server_socket_connection.recv(1024).decode()
        if data == '\quit':
            return 0
        else:
            print(data)

        if not send_message(server_socket_connection, user_name):
            return 0 

def create_socket(host, server_port, user_name):
    """
    Creates a socket, binds it to a port, and listens for connections.

    @param  host            empty string to indicate that we should  bind to all 
                            interfaces.
    @param  server_port     port to bind socket to.
    """

    # Bind socket to target address and port
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, server_port))
        s.listen(1)
        conn, addr = s.accept()

        exchange_messages(conn, user_name)

        return 

if __name__ == '__main__':
    if(len(sys.argv) < 2):
        print("You did not specify a port")

    else:
        while(True):
            # Request port from user.
            SERVER_PORT = int(sys.argv[1])
            HOST = ''

            user_name = input('Please enter your username: ')

            # Start server on specified port.
            print('Started message server on port %d...' % SERVER_PORT)
            create_socket(HOST, SERVER_PORT, user_name)
    
