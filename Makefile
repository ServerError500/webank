FLAG=-Werror -DDEBUG
STD=-std=gnu99 -pthread
CC=gcc


all:
	$(CC) $(STD) $(FLAG) client.c tools.c -o client
	$(CC) $(STD) $(FLAG) server.c tools.c -o server
clean:
	rm client server 
