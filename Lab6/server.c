/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab6 - server.c
*
* Description: 
*
* This is the server portion of the client/server interface. The server reads the filename that the client sends
* and recieves the input file. Then it creates an output file with the specified name, and stores the data from 
* the input file into the output file. Uses Ack/checksum/timeouts (on the client side) to confirm
*
* to run the program use the following: 
*   ./executable <port> <dstfile>
*   note: use ports over 1000, suggested to use 8080
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

//Define packet structure as in the client
typedef struct {
    int len;
    int seq_ack;
    unsigned int checksum;
} Header;
typedef struct {
    Header header;
    char data[1024];
} Packet;
void printPacket(Packet packet) {
    printf("Packet{ header: { len: %d, seq_ack: %d, checksum: %u }}\n", packet.header.len, packet.header.seq_ack, packet.header.checksum);
}
//Function to calculate checksum
unsigned int calculate_checksum(Packet packet) {
    unsigned int checksum = 0;
    char *buf = (char *)&packet;
    for (int i = 0; i < sizeof(packet.header) + packet.header.len; i++) {
        checksum += (unsigned char)buf[i];
    }
    return checksum;
}

//Main server function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <output file>\n", argv[0]);
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        return 1;
    }

    struct sockaddr_in servAddr, cliAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[1]));
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    Packet packet;
    socklen_t cliAddrLen = sizeof(cliAddr);
    int expectedSeqNum = 0;

    int fp = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fp < 0) {
        perror("File failed to open");
        exit(EXIT_FAILURE);
    }
    //listen....
    printf("Waiting for packets to come.....\n");
    int exit = 0;
    int iC = 0;
    int check = 0;
    while (1) {
        iC = 0;
        if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&cliAddr, &cliAddrLen) < 0) {
            perror("Error receiving packet");
            continue;
        }
        else{
            printPacket(packet);
        }
        unsigned int received_checksum = packet.header.checksum;
        packet.header.checksum = 0;  //Reset checksum field to calculate it afresh
        if ((received_checksum != calculate_checksum(packet) || packet.header.seq_ack != expectedSeqNum) && check != -1) {
            printf("Checksum calculated by server: %d \n", calculate_checksum(packet));
            printf("Recieved checksum: %d \n", received_checksum);
            printf("seq_ack recieved by server: %d \n", packet.header.seq_ack);
            printf("Expected ack: %d \n", expectedSeqNum);
            printf("Incorrect packet received, ignoring...\n");
            iC = 1;
            check = -1;     
        }
        if(iC == 0){
            printf("Packet received: ");
            if (packet.header.len > 0) {
                write(fp, packet.data, packet.header.len);
                printf("Received packet and wrote to file: %d bytes\n", packet.header.len);
            }
            else{
                exit = 1;
            }
            //Send ACK back
            packet.header.seq_ack = expectedSeqNum;
            packet.header.len = 0;  //ACK packet has no data
            packet.header.checksum = calculate_checksum(packet);
            sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&cliAddr, cliAddrLen);
            printf("Ack packet sent with ack = %d \n", expectedSeqNum);
            expectedSeqNum = 1 - expectedSeqNum;  //Toggle expected sequence number
            if(exit == 1){
                close(sockfd);
                return 0;
            }
        }else{
            packet.header.seq_ack = -1;
            packet.header.len = 0;  //ACK packet has no data
            packet.header.checksum = calculate_checksum(packet);
            sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&cliAddr, cliAddrLen);
            printf("Ack packet sent with ack = -1 \n");
        }
    }
    return 0;
}
