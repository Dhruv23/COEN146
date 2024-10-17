# do make client to make the client executable as ./client



client: client.o
	gcc $^ -o client
client.o: client.c
	gcc -c $<

# do make clean to clear all client related files, and all old output files
clean:
	rm -f client
	rm -f *.o
	rm -f *.dat