server1: server1.o
	gcc $^ -o server1
server1.o: server1.c
	gcc -c $<


server2: server2.o
	gcc $^ -o server2
server2.o: server2.c
	gcc -c $<

client: client.o
	gcc $^ -o client
client.o: client.c
	gcc -c $<



clean:
	rm -f client
	rm -f server1
	rm -f server2
	rm -f *.o