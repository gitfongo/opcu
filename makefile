all:
	gcc -Wall -o client open62541.c client.c

clean:
	rm -f client

