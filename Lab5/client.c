/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab4 - client.c 
*
* Description: 
*
* This is the client portion of the client/server lab 3. the client initiates a connection on the specified port and IP,
* and sends a file of any type to the server.
*
* to run the program use the following: 
*   ./executable <server IP> <port> <input file>
*   note: localhost ip address is 127.0.0.1   
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
int nextAck = 0;
//define header struct for use in packet
typedef struct {
    int len;
    int ack;
} Header;

//define packet struct
typedef struct {
    Header header;
    char data[1024];
    char checksum[1024];
} Packet;

//prints packet 
void printPacket(Packet packet) {
    printf("Packet{ header: { len: %d }, ack: \"%d\" }\n", packet.header.len, packet.header.ack);
}

//sends data
int clientSend(int sockfd, const struct sockaddr_in *servAddr, Packet packet) {
    sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)servAddr, sizeof(*servAddr));
    printPacket(packet);
    printf("Client sending packet\n");
    int i;
    // Receiving ACK from the server
    Packet recvpacket;
    socklen_t addr_len = sizeof(*servAddr);
    recvfrom(sockfd, &recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)servAddr, &addr_len);
    printPacket(recvpacket);
    for(i=0; i<10; i++){
        if(recvpacket.checksum[i] != packet.checksum[i]){
            printf("\n checksum does not match, retransmitting \n");
            return 0;
        }
    }

    if(recvpacket.header.ack == -1){
        printf("ack packet not recieved, retransmitting \n");
        return 1;
    }else if(recvpacket.header.ack == 1){
        printf("Server acknowledged\n");
        nextAck = 0;
        return 0;
    }else if(recvpacket.header.ack == 0){
        printf("Server acknowledged\n");
        nextAck = 1;
        return 0;
    }
    return 0;
}


int main(int argc, char **argv) {

    //check args 
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server IP> <port> <input file>\n", argv[0]);
        exit(1);
    }

    //create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        return 1;
    }
    //servAddr args
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }

    //open file
    int fp = open(argv[3], O_RDONLY);
    if (fp < 0) {
        perror("Failed to open input file");
        return 1;
    }

    //send server
    Packet packet;
    ssize_t bytes_read;
    int i;
    while ((bytes_read = read(fp, packet.data, sizeof(packet.data) - 1)) > 0) {
        packet.data[bytes_read] = '\0';  // Ensure null-terminated string
        packet.header.len = bytes_read;
        for(i=0; i<10; i++){
            packet.checksum[i] = ~packet.data[i];
        }
        packet.header.ack = nextAck;
        if(clientSend(sockfd, &servAddr, packet) == 1){
            clientSend(sockfd, &servAddr, packet);
        }
        
    }


    //close file pointer and socket
    close(fp);
    close(sockfd);
    return 0;
}
