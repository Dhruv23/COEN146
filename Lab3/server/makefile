# do make server1 to build the single connection server, executed as ./server1

server1: server1.o
	gcc $^ -o server1
server1.o: server1.c
	gcc -c $<

# do make server2 to build the multi connection server, executed as ./server2
server2: server2.o
	gcc $^ -o server2
server2.o: server2.c
	gcc -c $<

# do make clean to clear all server related files.
clean:
	rm -f server1
	rm -f server2
	rm -f *.o