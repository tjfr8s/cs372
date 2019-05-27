### Author: Tyler Freitas
### Date: 05/27/2019

### Testing machine: client: flip1  server: flip2

### Starting the ftserver:

- build the ftserver by running `make` in the containing directory.

- in order to test the program the ftserver should be located in a separate 
directory from the client as the client will be attempting to save the file it
requested in the same location as the server is sourcing the file.

- the server can be started with the following command where PORTNUM is the 
port you want it to listen on.
    `./ftserver SERVER_PORT`

- terminate the server by sending `ctl + C`

### Starting the ftclient:

- the ftclient can be run with the following commands:

    - to list the directory contents:
    `python3 ftclient.py  SERVER_HOST SERVER_PORT -l  DATA_PORT` 
            or `./ftclient.py  SERVER_HOST SERVER_PORT -l  DATA_PORT`
    - to get a file:
    `python3 ftclient.py  SERVER_HOST SERVER_PORT -g FILE_NAME  DATA_PORT` 
            or `./ftclient.py  SERVER_HOST SERVER_PORT -g FILE_NAME  DATA_PORT`
    
### Citations:
- https://beej.us/guide/bgnet/
- https://docs.python.org/3/library/socket.html
- https://stackoverflow.com/questions/3463426/in-c-how-should-i-read-a-text-file-and-print-all-strings (reading chunks of file in c)
- Some code was also taken from my own Project 1 for this class.
