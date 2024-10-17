# do make server to build the server, executed as ./server <args>

server: server.o
	gcc $^ -o server
server.o: server.c
	gcc -c $<


# do make client to build the client, executed as ./client <args>
client: client.o
	gcc $^ -o client
client.o: client.c
	gcc -c $<


# do make clean to clear files.
clean:
	rm -f client
	rm -f server
	rm -f *.o
	rm -f out.*