/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab5 - server.c
*
* Description: 
*
* This is the server portion of the client/server interface. The server reads the filename that the client sends
* and recieves the input file. Then it creates an output file with the specified name, and stores the data from 
* the input file into the output file. Uses Ack/checksum to confirm
*
* to run the program use the following: 
*   ./executable <port> <dstfile>
*   note: use ports over 1000, suggested to use 8080
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

//define header struct
typedef struct {
    int len;
    int ack;
} Header;

//define Packet struct
typedef struct {
    Header header;
    char data[1024];
    char checksum[1024];
} Packet;


//define address structs
struct sockaddr_in servAddr, clienAddr;

//random function to send incorrect checksum
int randomNum(){
    int N = 10;
    int value = rand() & (N + 1);
    return value;
}
//function to print packets
void printPacket(Packet packet) {
    printf("Packet{ header: { len: %d }, data: \"", packet.header.len);
    fwrite(packet.data, (size_t)packet.header.len, 1, stdout);
    printf("\" }\n");
}

//function to send to client from server
void serverSend(int sockfd, const struct sockaddr_in *clienAddr, Packet toSend) {
    toSend.header.len = 0;
    sendto(sockfd, &toSend, sizeof(toSend), 0, (struct sockaddr *)clienAddr, sizeof(*clienAddr));
    printPacket(toSend);
    printf("Server sending %d\n", toSend.header.ack);
}


int main(int argc, char *argv[]) {
    //make sure arguments are correct 
    if (argc != 3) {
        printf("Usage: %s <port> <dstfile>\n", argv[0]);
        exit(0);
    }
    
    //set port
    int definedPort = atoi(argv[1]);
    if (definedPort <= 0) {
        printf("Invalid port number, try 8080\n");
        exit(EXIT_FAILURE);
    }

    //create socket with UDP protocol
    int sockfd; 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }


    //allocate size for servAddr
    memset(&servAddr, 0, sizeof(servAddr));

    //set args 
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(definedPort);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    //bind socket 
    if (bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind failed");
        return 1;
    }

    //create output file
    int fp = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fp < 0) {
        perror("File failed to open");
        exit(EXIT_FAILURE);
    }


    //listen....
    printf("Waiting for packets to come.....\n");
    int i;
    int ackNum = 0;
    int random;
    //recieve packets
    while (1) {
        socklen_t clienLen = sizeof(clienAddr);
        Packet recvpacket;
        int len = recvfrom(sockfd, &recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&clienAddr, &clienLen);
        if (len > 0 && ackNum == 0) {
            write(fp, recvpacket.data, recvpacket.header.len);
            printf("Received packet and wrote to file: %d bytes\n", recvpacket.header.len);
            Packet sendPacket;
            for(i = 0; i<10; i++){
                random = randomNum();
                //printf("Random number was %d \n", random);
                if(random == 2){
                    sendPacket.checksum[i] = recvpacket.checksum[i-1];
                }else{
                    sendPacket.checksum[i] = recvpacket.checksum[i];
                }
                
            }
            sendPacket.header.ack = ackNum;
            ackNum = 1;
            serverSend(sockfd, &clienAddr, sendPacket);
        }else if (len > 0 && ackNum == 1) {
            write(fp, recvpacket.data, recvpacket.header.len);
            printf("Received packet and wrote to file: %d bytes\n", recvpacket.header.len);
            Packet sendPacket;
            for(i = 0; i<10; i++){
                random = randomNum();
                if(random == 5){
                    printf("Random number was %d \n", random);
                    sendPacket.checksum[i] = recvpacket.checksum[i-1];
                }else{
                    sendPacket.checksum[i] = recvpacket.checksum[i];
                }
                
            }
            sendPacket.header.ack = ackNum;
            ackNum = 0;
            serverSend(sockfd, &clienAddr, sendPacket);
        }
        else{
            write(fp, recvpacket.data, recvpacket.header.len);
            printf("Received packet and wrote to file: %d bytes\n", recvpacket.header.len);
            Packet sendPacket;
            for(i = 0; i<10; i++){
                sendPacket.checksum[i] = recvpacket.checksum[i];
            }
            sendPacket.header.ack = -1;
            serverSend(sockfd, &clienAddr, sendPacket);
        }
    }


    //close file pointer and socket
    close(fp);
    close(sockfd);

    //exit 
    return 0;
}
