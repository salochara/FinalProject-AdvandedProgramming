#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>
#include "sockets.h"
#include "fatal_error.h"
#define MAX_QUEUE 5
#define TIMEOUT_POLL 50
#define BUFFER_SIZE 50

// Data that will be sent to each structure
typedef struct data_struct {
    // The file descriptor for the socket
    int connection_fd;
} thread_data_t;



// FUNCTION DEFINITIONS
void usage(char * program);
void waitForConnections(int server_fd);

int interrupt_exit = 0;

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;

    printf("\n=== SIMPLE BANK SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }


    // Show the IPs assigned to this computer
    printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
    // Listen for connections from the clients
    waitForConnections(server_fd);
    // Close the socket
    close(server_fd);



    // Finish the main thread
    pthread_exit(NULL);

    return 0;
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    thread_data_t * connection_data = NULL;
    int poll_response;

    // Create a structure array to hold the file descriptors to poll and fill the struct
    struct pollfd test_fds[1];
    test_fds[0].fd = server_fd;
    test_fds[0].events = POLLIN;

    while(!interrupt_exit)
    {
        // Get the size of the structure to store client information
        client_address_size = sizeof client_address;
        while (1)
        {
            // POLLING
            poll_response = poll(test_fds,1,TIMEOUT_POLL);

            if(poll_response == 0)
                continue;
            else if(poll_response == -1)
            {
                perror("poll");
                break;
            }else{
                if(test_fds[0].revents && POLL_IN)
                {
                    // ACCEPT
                    // Wait for a client connection
                    client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
                    if (client_fd == -1)
                        fatalError("ERROR: accept");

                    // Get the data from the client
                    inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
                    printf("Received incoming connection from %s on port %d\n", client_presentation, client_address.sin_port);
                    break;
                }
            }
        }
        // Dynamic allocation for the connection data. A pointer to the thread_data_t struct
        connection_data = malloc(sizeof(thread_data_t));

        // Prepare structure to send to thread.
        connection_data->connection_fd = client_fd;

        // CREATE A THREAD
        printf("Creating a new thread!\n");
        pthread_create(&new_tid,NULL,attentionThread,connection_data);


    }
    // Only if there were any connections
    if(connection_data != NULL)
    {
        // Show the number of total transactions
        printf("Total transactions done: %d from the user connected with (%s) on port %d\n", connection_data->bank_data->total_transactions, client_presentation, client_address.sin_port);

        // Store any changes in the file
        writeBankFile(connection_data);
    }
}

void * attentionThread(void * arg)
{
    char buffer[BUFFER_SIZE];

    thread_data_t * threadData = (thread_data_t*) arg;
    int connection_fd = threadData->connection_fd;


    while(1)
    {
        // Receive the request
        recvString(connection_fd,buffer,BUFFER_SIZE);

    }
}


void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}