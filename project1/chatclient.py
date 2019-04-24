import socket

def send_message(client_socket):
    message = input(': ')
    if not message == '\quit':
        message = message.encode()
        client_socket.sendall(message)
        return True
    else:
        message = message.encode()
        client_socket.sendall(message)
        return False

def create_client_socket(host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    return s

def exchange_messages(client_socket):
    while(True):
        if not send_message(client_socket):
            return 0
        data = client_socket.recv(1024)
        data = data.decode()
        if data == '\quit':
            return 0
        else:
            print(': %s' % data)

if __name__ == '__main__':

    HOST = 'flip1.engr.oregonstate.edu'   
    PORT = 30027

    client_socket = create_client_socket(HOST, PORT)
    exchange_messages(client_socket)
    client_socket.close()
