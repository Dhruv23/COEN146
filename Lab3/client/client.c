/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab3 - Step 2 
*
* Description: 
*
* This is the client portion of the client/server lab 3. the client initiates a connection on the specified port and IP,
* and requests a file of any type that is on the server.
*
* to run the program use the following: 
*   ./executable <port> <ip address of server> <input filename> <output filename>\n
*   note: localhost ip address is 127.0.0.1   
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[]) {

    //check number of inputs 
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <port> <ip of server> <input filename> <output filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    //parse info
    int port = atoi(argv[1]);
    const char* server_ip = argv[2];
    const char* inFilename = argv[3];
    const char* outFilename = argv[4];
    int sock;
    struct sockaddr_in address;
    char buffer[1024] = {0};

    //create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return EXIT_FAILURE;
    }

    //address initialization
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return EXIT_FAILURE;
    }


    //start connection
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Connection Failed");
        return EXIT_FAILURE;
    }
    printf("Connected to the server successfully\n");

    //send filename to server
    send(sock, inFilename, strlen(inFilename), 0);
    send(sock, "\n", 1, 0);
    printf("Filename sent to server.\n");

    //open file to write received data
    FILE* file = fopen(outFilename, "wb");
    if (file == NULL) {
        perror("Cannot open file for writing");
        close(sock);
        return EXIT_FAILURE;
    }

    //receive data from server and write to file
    int bytes_read;
    while ((bytes_read = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_read, file);
    }

    //close
    fclose(file);
    printf("File received and written successfully.\n");

    close(sock);
    return 0;
}
