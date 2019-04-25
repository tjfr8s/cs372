import socket

def send_message(client_socket, user_name):
    message = input(f'{user_name}> ')
    if not message == '\quit':
        message_with_handle = f'{user_name}> {message}' 
        message_with_handle = message_with_handle.encode()
        client_socket.sendall(message_with_handle)
        return True
    else:
        message = message.encode()
        client_socket.sendall(message)
        return False

def create_client_socket(host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    return s

def exchange_messages(client_socket, user_name):
    while(True):
        if not send_message(client_socket, user_name):
            return 0
        data = client_socket.recv(1024)
        data = data.decode()
        if data == '\quit':
            return 0
        else:
            print(data)

if __name__ == '__main__':

    HOST = 'flip1.engr.oregonstate.edu'   
    PORT = 30027

    user_name = input("Please enter your username: ")
    client_socket = create_client_socket(HOST, PORT)
    exchange_messages(client_socket, user_name)
    client_socket.close()
