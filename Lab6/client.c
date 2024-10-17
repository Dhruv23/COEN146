/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab5 - client.c 
*
* Description: 
*
* This is the client portion of the client/server lab 6. the client initiates a connection on the specified port and IP,
* and sends a file of any type to the server. Uses Ack packets, checksum, timeouts
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
#include <time.h>

//Define packet structure with sequence number and checksum
typedef struct {
    int len;
    int seq_ack;
    unsigned int checksum;
} Header;
typedef struct {
    Header header;
    char data[1024];
} Packet;

//Function to calculate checksum
unsigned int calculate_checksum(Packet packet) {
    unsigned int checksum = 0;
    char *buf = (char *)&packet;
    for (int i = 0; i < sizeof(packet.header) + packet.header.len; i++) {
        checksum += (unsigned char)buf[i];
    }
    return checksum;
}


int randomNum(){
    int N = 10;
    int value = rand() % (N + 1);
    return value;
}

//Prints packet details
void printPacket(Packet packet) {
    printf("Packet{ header: { len: %d, seq_ack: %d, checksum: %u } }\n", packet.header.len, packet.header.seq_ack, packet.header.checksum);
}

//Sends data and handles ACKs with retransmission
void clientSend(int sockfd, struct sockaddr_in *servAddr, Packet packet, int seqNum) {
    int random;
    packet.header.seq_ack = seqNum;  //Set sequence number
    packet.header.checksum = 0;  //Reset checksum
    random = randomNum();
    if(random == 2){
        packet.header.checksum = calculate_checksum(packet);
        packet.header.checksum = packet.header.checksum - 1; //randomize checksum 
    }else{
        packet.header.checksum = calculate_checksum(packet);  //Calculate checksum
    }
    
    fd_set fds;
    struct timeval tv;
    int rv;

    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);

    tv.tv_sec = 2;  //Timeout after 2 seconds
    tv.tv_usec = 0;

    //Send packet
    sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)servAddr, sizeof(*servAddr));
    printPacket(packet);
    printf("Client sending packet with sequence number %d\n", seqNum);
    
    while (1) {
        rv = select(sockfd + 1, &fds, NULL, NULL, &tv);
        if (rv == 0) { //Timeout, no data
            printf("Timeout occurred, resending packet...\n");
            sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)servAddr, sizeof(*servAddr));
            FD_SET(sockfd, &fds); //Reset fds for next select call
            tv.tv_sec = 2;  //Reset timer
            continue;
        }

        if (rv > 0) { //Data available
            Packet recvpacket;
            socklen_t addr_len = sizeof(*servAddr);
            recvfrom(sockfd, &recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)servAddr, &addr_len);
            if (recvpacket.header.seq_ack == seqNum) {
                printf("ACK received for sequence number %d\n", seqNum);
                break;  //Correct ACK received
            } else if(recvpacket.header.seq_ack == -1){
                printf("Recieved ACK: %d, expected ACK %d, resending packet...\n", recvpacket.header.seq_ack, seqNum);
                packet.header.checksum = calculate_checksum(packet);
                sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)servAddr, sizeof(*servAddr));
                FD_SET(sockfd, &fds); //Reset fds for next select call
                tv.tv_sec = 2;  //Reset timer
                continue;
            }
            if(recvpacket.header.checksum == calculate_checksum(recvpacket)){
                printf("Checksum received: %d\n", recvpacket.header.checksum);
            }
            else {
                printf("Recieved checksum: %d, expected checksum %d, resending packet...\n", recvpacket.header.checksum, calculate_checksum(recvpacket));
                sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)servAddr, sizeof(*servAddr));
                FD_SET(sockfd, &fds); //Reset fds for next select call
                tv.tv_sec = 2;  //Reset timer
                continue;
            }
        }
    }
}

int main(int argc, char **argv) {
    //Check args
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server IP> <port> <input file>\n", argv[0]);
        exit(1);
    }

    //Create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        return 1;
    }

    //Set server address
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address\n");
        return 1;
    }

    //Open file
    int filefd = open(argv[3], O_RDONLY);
    if (filefd < 0) {
        perror("Failed to open file");
        return 1;
    }

    char buf[1024];
    int len;
    int seqNum = 0;

    //Read from file and send via UDP
    while ((len = read(filefd, buf, sizeof(buf))) > 0) {
        Packet packet;
        packet.header.len = len;
        memcpy(packet.data, buf, len);

        clientSend(sockfd, &servAddr, packet, seqNum);

        seqNum = 1 - seqNum;  //Alternate between 0 and 1 for sequence numbers
    }
    Packet end;
    end.header.len = 0;
    clientSend(sockfd, &servAddr, end, seqNum);
    close(filefd);
    close(sockfd);

    return 0;
}