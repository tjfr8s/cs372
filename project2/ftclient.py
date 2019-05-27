import socket
import sys

def send_command(control_socket, command, host, port):
    control_socket.sendall(" ".join(command).encode())
    response = control_socket.recv(1024)
    response = response.decode()

    if response == "ok":
        return True
    else:
        print(f'{host}:{port} says {response}')
        return False;


    # if response is error print it and return false
    
    # if response is sending directory contents or file return true

def create_control_socket(host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    return s

def create_data_socket(host, server_port):
    """
    Creates a socket, binds it to a port, and listens for connections.

    @param  host            empty string to indicate that we should  bind to all 
                            interfaces.
    @param  server_port     port to bind socket to.

    @preconditions:     
     - host, port, and username are passed to function

    @postconditions:
     - a socket is created and used to exchange messages.
    """

    # Bind socket to target address and port
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, server_port))
        s.listen(1)
        conn, addr = s.accept()

        return conn

def recv_list_directory(conn):
    message = conn.recv(1024).decode();
    print(message)
    return True

def recv_file(control_socket, conn, filename):
    filename = "./" + filename 
    with open(filename, "w") as ofile:
        while (control_socket.recv(1024).decode() == 'ok'):
            ofile.write(conn.recv(1024).decode())
        print("File transfer complete.")
    return True

def parse_args(argv):
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

