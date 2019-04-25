CC=gcc
CFLAGS=-g -Wall -std=c99

all: chatclient 

chatclient: chatclient.o 
	$(CC) $^ -o $@

clean:
	-rm *.o

cleanall: clean
	-rm chatclient
