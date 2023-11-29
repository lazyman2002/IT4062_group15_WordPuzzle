resolver:
	gcc -o client tcp_client.c -lm
	gcc -o server tcp_server.c -lssl -lcrypto -lm
clean:
	rm -f *.0 resolver
