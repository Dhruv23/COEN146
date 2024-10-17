/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab3 - Step 4 
*
* Description: 
*
* This is the concurrent server portion of the client/server interface. The server reads the filename that the client requests
* and sends the requested file, up to 5 client connections are supported via threads.
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
#include <pthread.h>

#define MAX_THREADS 5
int server_fd;
struct sockaddr_in address;
int addrlen = sizeof(address);
char buffer[1024] = {0};
int threadCount = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* connection(void* sock) {

    //use thread to handle connection
    pthread_mutex_lock(&lock);
    threadCount++;
    pthread_mutex_unlock(&lock);
    int new_socket = *(int*)sock;
    free(sock); 

    //copy filename
    char filename[256] = {0};
    int filename_length = 0;
    while (1) {
        char ch;
        int result = read(new_socket, &ch, 1);
        if (result < 1 || ch == '\n') break;
        filename[filename_length++] = ch;
        if (filename_length >= sizeof(filename) - 1) break;
    }
    filename[filename_length] = '\0'; //mull-terminate the string

    //print which thread is working
    printf("Thread: %d handling request for file: %s \n", threadCount, filename);

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("File cannot be opened");
        close(new_socket);
        return NULL;
    }

    //send requested file
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(new_socket, buffer, bytes_read, 0);
    }
    fclose(fp);
    printf("File %s sent to client\n", filename);
    if(threadCount == 5){
            close(server_fd);
            printf("Server shutting down after handling %d connections.\n", threadCount); //max of 5 threads
            exit(0);
        }
    close(new_socket);
    pthread_exit(NULL); // Ensure thread exits cleanly
}

int main(int argc, char *argv[]) {
    //check # of args
    if (argc != 2) {
        printf("Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //define port
    int definedPort = atoi(argv[1]);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    //define address info

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(definedPort);

    //bind to socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    //listen
    if (listen(server_fd, MAX_THREADS) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", definedPort);

    //threads logic
    while (1) {
        pthread_mutex_lock(&lock);
        if (threadCount < MAX_THREADS) {
            pthread_mutex_unlock(&lock);
            int* new_socket = malloc(sizeof(int));
            *new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

            pthread_t thread_id;
            if (pthread_create(&thread_id, NULL, connection, new_socket) != 0) {
                perror("Failed to create thread");
                close(*new_socket);
                free(new_socket);
            } else {
                pthread_detach(thread_id);
            }
        } else {
            pthread_mutex_unlock(&lock);
            break; // Exit the loop and terminate the server after handling 5 connections
        }
    }

    close(server_fd);
    // printf("Server shutting down after handling %d connections.\n", threadCount);
    return 0;
}
