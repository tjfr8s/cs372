### Author: Tyler Freitas
### Date: 05/01/2019

### Testing machine: flip1.engr.oregonstate.edu

### Starting the chatserver:

- run the chat server using the following commands: 
`python3 chatserve.py PORTNUM` or `./chatserve.py PORTNUM`
    - PORTNUM is the port the server should listen on. This is specified as a 
commandline argument.

- the user will be prompted to enter a user name.

- an incomming message will appear when a client connects to the server and
initiates a conversation. The two users will then be able to alternate sending
messages until one of them enters `\quit`. When a user quits the server remain
listening on the same port and will ask for a user name again.

- messages are sent using the enter key.

- use `ctl + C` to terminate the server.


### Starting the chatclient:

- build the client program by running `make` in the parent directory of
`chatclient.py`

- start the client with the following command: 
`./chatclient HOST_ADDRESS PORT`. `HOST_ADDRESS` and `PORT` are the address
and port you wish to make a connection with.

- the user will be prompted to enter a user name.

- once the client has connected to the server, the user will compose their
first message and send it with the enter key.  This step initiates a 
conversation. The two users will then be able to alternate sending
messages until one of them enters `\quit`. a user name again.


### Citations:
- https://beej.us/guide/bgnet/
- https://docs.python.org/3/library/socket.html
