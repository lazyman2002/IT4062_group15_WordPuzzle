resolver:
	gcc -o client tcp_client.c -pthread `pkg-config --cflags --libs gtk+-3.0`
	gcc -o server tcp_server.c -lssl -lcrypto -lm
clean:
	rm -f *.0 resolver
