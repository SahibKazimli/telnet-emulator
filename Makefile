server: server.c
	gcc -o server server.c

client: client.c
	gcc -o client client.c

all: server client

clean:
	rm -rf server client