all: client.c server.c
	gcc -Wall -g -o client client.c -lpthread
	gcc -Wall -g -o server server.c -lpthread

clean:
	rm server
	rm client
