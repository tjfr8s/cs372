import socket

def get_server_port():
    port_input = 30027
    print('Getting port from user')

    return port_input 

def send_message(server_socket_connection, user_name):
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
    while True:
        data = server_socket_connection.recv(1024).decode()
        if data == '\quit':
            return 0
        else:
            print(data)

        if not send_message(server_socket_connection, user_name):
            return 0 

def create_socket(host, server_port, user_name):

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, server_port))
        s.listen(1)
        conn, addr = s.accept()

        exchange_messages(conn, user_name)

        return 


def handle_requests(server_socket):
    print('Handling requests...')
    return 

if __name__ == '__main__':
    # Request port from user.
    SERVER_PORT = get_server_port() 
    HOST = ''

    user_name = input('Please enter your username: ')

    print('Started message server on port %d...' % SERVER_PORT)
    # Start server on specified port.
    create_socket(HOST, SERVER_PORT, user_name)
    
