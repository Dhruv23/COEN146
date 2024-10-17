#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int server_fd, new_socket;
struct sockaddr_in address;
int addrlen = sizeof(address);
char buffer[1024] = {0};

void connection(){
    //while(0){} << implement this to make it keep running

        //waiting for a connection
        printf("Waiting for a connection...\n");

        //accept connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
            //continue;
        }
        printf("Connection accepted from %s\n", inet_ntoa(address.sin_addr));

        // Read the output file name first
        char filename[256] = {0};
        int filename_length = 0;

        //code to read filename
        while (1) {
            char ch;
            int result = read(new_socket, &ch, 1);
            if (result < 1 || ch == '\n') break; //bc client sends a \n to end reading
            filename[filename_length++] = ch;
            if (filename_length >= sizeof(filename) - 1) break;
        }

        filename[filename_length] = '\0'; // Null-terminate the string for processing

        //open file
        FILE *fp = fopen(filename, "wb");
        if (fp == NULL) {
            perror("File cannot be opened");
            close(new_socket);
            //continue;
        }

        // Continue reading the rest of the data and write it to the file
        int bytes_read;
        while ((bytes_read = read(new_socket, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, 1, bytes_read, fp);
            fflush(fp);
        }

        fclose(fp);
        printf("Received and saved file %s\n", filename);
        
       
        if (bytes_read == 0) {
            printf("Connection closed by client\n");
        } else {
            perror("recv error");
        }
        close(new_socket);
}

int main(int argc, char *argv[]) {
    
    //implement server, declare, etc

	if(argc != 2){
		printf("usage: %s [port]", argv[0]);
		exit(0);
	}

    //get port
    if(atoi(argv[1]) <=0){
        printf("Invalid port number, try 8080 \n");
        exit(0);
    }
	int definedPort = atoi(argv[1]);

    //open socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //default arguments for address struct
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(definedPort);

    //bind
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	//start listening
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", definedPort);

    //wait for connection
    connection();
		
    
    return 0;
}