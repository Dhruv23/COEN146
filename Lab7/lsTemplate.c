/*
* Name: Dhruv Patel
* Date: Tuesday 2:15 (the day you have lab)
* Title: Lab7 - lsTemplate.c 
*
* Description: 
*
* This is the file used to calculate the min djkstras algorithm for our router/cost simulation
*
* to run the program use the following: 
*   ./executable <router_id> <num_nodes> <cost_file> <machines_file>
*   note: localhost ip address is 127.0.0.1   
*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

//defines
#define N 4
#define INFINITE 1000
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

//types
typedef struct routers
{
    char name[50];
    char ip[50];
    int port;

} ROUTERS;

//global variables
ROUTERS routers[N];
int costs[N][N];
int distances[N];
int myid, nodes;
int sock;
struct sockaddr_in addr;
struct sockaddr_in otheraddr;
socklen_t addr_size;
pthread_mutex_t lock;

//print costs
void print_costs (void)
{
    int i, j;
    printf("Cost table:\n");
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
            printf("%d\t", costs[i][j]);
        printf("\n");
    }
}

//receive info
void *receive_info(void *arg)
{
    int packet[3];
    int from, to, cost;

    while (1)
    {
        recvfrom(sock, packet, sizeof(packet), 0, NULL, NULL);
        from = ntohl(packet[0]);
        to = ntohl(packet[1]);
        cost = ntohl(packet[2]);

        pthread_mutex_lock(&lock);
        costs[from][to] = cost;
        costs[to][from] = cost;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

//run_link_state
void *run_link_state(void *arg)
{
    int taken[N];
    int min, spot;
    int i, j;
    int r;

    while (1)
    {
        r = rand() % 10;
        sleep(r);

        for (i = 0; i < N; i++)
        {
            taken[i] = 0;
            pthread_mutex_lock(&lock);
            distances[i] = costs[myid][i];
            pthread_mutex_unlock(&lock);
        }
        taken[myid] = 1;

        for (i = 1; i < N; i++)
        {
            //find closest node
            min = INFINITE;
            for (j = 0; j < N; j++)
            {
                if (!taken[j] && distances[j] < min)
                {
                    min = distances[j];
                    spot = j;
                }
            }
            taken[spot] = 1;

            //recalculate distances
            for (j = 0; j < N; j++)
            {
                if (!taken[j])
                {
                    pthread_mutex_lock(&lock);
                    distances[j] = MIN(distances[j], (distances[spot] + costs[spot][j]));
                    pthread_mutex_unlock(&lock);
                }
            }
        }

        printf("New distances:\n");
        for (i = 0; i < N; i++)
            printf("%d ", distances[i]);
        printf("\n");
    }
    return NULL;
}

//main()
int main(int argc, char *argv[])
{
    FILE *fp;
    int i, j;
    pthread_t thr1, thr2;
    int id, cost;
    int packet[3];

    if (argc != 5)
    {
        printf("Usage: %s <router_id> <num_nodes> <cost_file> <machines_file>\n", argv[0]);
        return -1;
    }

    myid = atoi(argv[1]);
    nodes = atoi(argv[2]);

    //Initialize the mutex
    pthread_mutex_init(&lock, NULL);

    //Read the cost table
    fp = fopen(argv[3], "r");
    for (i = 0; i < nodes; i++)
        for (j = 0; j < nodes; j++)
            fscanf(fp, "%d", &costs[i][j]);
    fclose(fp);

    //Read the routers table
    fp = fopen(argv[4], "r");
    for (i = 0; i < nodes; i++)
        fscanf(fp, "%s %s %d", routers[i].name, routers[i].ip, &routers[i].port);
    fclose(fp);

    print_costs();

    //Create a socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)routers[myid].port);
    inet_pton(AF_INET, routers[myid].ip, &addr.sin_addr.s_addr);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    //Create threads
    pthread_create(&thr1, NULL, receive_info, NULL);
    pthread_create(&thr2, NULL, run_link_state, NULL);

    //Read changes from the keyboard
    for (i = 0; i < 3; i++)
    {
        printf("Any changes? ");
        scanf("%d%d", &id, &cost);
        if (id >= nodes || id == myid)
        {
            printf("Wrong id\n");
            break;
        }

        pthread_mutex_lock(&lock);
        costs[myid][id] = cost;
        costs[id][myid] = cost;
        pthread_mutex_unlock(&lock);
        print_costs();

        packet[0] = htonl(myid);
        packet[1] = htonl(id);
        packet[2] = htonl(cost);
        otheraddr.sin_family = AF_INET;
        addr_size = sizeof(otheraddr);

        for (j = 0; j < nodes; j++)
        {
            if (j != myid)
            {
                otheraddr.sin_port = htons((short)routers[j].port);
                inet_pton(AF_INET, routers[j].ip, &otheraddr.sin_addr.s_addr);
                sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&otheraddr, addr_size);
            }
        }
        printf("Sent\n");
    }

    sleep(20);
    return 0;
}
