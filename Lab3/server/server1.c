/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab3 - Step 1 
*
* Description: 
*
* This is the server portion of the client/server interface. The server reads the filename that the client requests
* and sends the requested file. This version of the server will terminate after one client request
*
* to run the program use the following: 
*   ./executable <port> 
*   note: use ports over 1000, suggested to use 8080
*/
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

void connection() {


    printf("Waiting for a connection...\n");

    //connection requested

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("Connection accepted from %s\n", inet_ntoa(address.sin_addr));


    //read the requested filename
    char filename[256] = {0};
    int filename_length = 0;
    while (1) {
        char ch;
        int result = read(new_socket, &ch, 1);
        if (result < 1 || ch == '\n') break;
        filename[filename_length++] = ch;
        if (filename_length >= sizeof(filename) - 1) break;
    }

    filename[filename_length] = '\0';
    printf("Filename received: %s\n", filename);


    //open the requested file
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("File cannot be opened");
        close(new_socket);
        return;
    }

    //send the requested file
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(new_socket, buffer, bytes_read, 0);
    }

    //close all open memory
    fclose(fp);
    close(new_socket);
    printf("File %s sent to client\n", filename);
}

int main(int argc, char *argv[]) {

    //check number of inputs
    if (argc != 2) {
        printf("Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //validate port number
    int definedPort = atoi(argv[1]);
    if (definedPort <= 0) {
        printf("Invalid port number, try 8080\n");
        exit(EXIT_FAILURE);
    }

    //create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //define address struct and info
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(definedPort);

    //bind server to socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //start listening for a connection 
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", definedPort);
    
    connection();  //if connection is successful, then handle it 
    

    return 0;
}
