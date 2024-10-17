#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <port> <ip of server> <inputfile> <outputfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    //get port, server, nput, output.
    int port = atoi(argv[1]);
    const char* server_ip = argv[2];
    const char* inputfile = argv[3];
    const char* outputfile = argv[4];

    //declarations for sending
    int sock = 0;
    struct sockaddr_in address;
    char buffer[1024] = {0};


    //create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return EXIT_FAILURE;
    }

    //define address struct
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    
    //check if valid address
    if (inet_pton(AF_INET, server_ip, &address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return EXIT_FAILURE;
    }

    //connect
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Connection Failed");
        return EXIT_FAILURE;
    }
    printf("Connected to the server successfully\n");


    // First send the output file name
    send(sock, outputfile, strlen(outputfile), 0);
    send(sock, "\n", 1, 0);  // send a newline as a ending

    // Open the file to send its contents
    FILE* file = fopen(inputfile, "rb");
    if (file == NULL) {
        perror("Cannot open input file");
        close(sock);
        return EXIT_FAILURE;
    }

    //send one byte at a time
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(sock, buffer, bytes_read, 0);
    }
    fclose(file);
    printf("File sent successfully\n");

    close(sock);
    return 0;
}