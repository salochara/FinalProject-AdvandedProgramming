/*
        Gilberto Echeverria
        gilecheverria@yahoo.com

    Roberto Alejandro Gutierrez Guillen
    A01019608
*/

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

// Custom libraries
#include "sockets.h"
#include "fatal_error.h"
#include "myqueue.h"

#define MAX_QUEUE 5
#define TIMEOUT 1000
#define POOLSIZE 20

pthread_t thread_pool[POOLSIZE];

char response[] = "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                  "<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
                  "<style>body { background-color: #111 }"
                  "h1 { font-size:4cm; text-align: center; color: black;"
                  " text-shadow: 0 0 2mm red}</style></head>"
                  "<body><h1>Goodbye, world!</h1></body></html>\r\n";

// Global variables for signal handlers
int interrupted = 0;

// Data that will be sent to each structure
typedef struct data_struct {
    // The file descriptor for the socket
    int connection_fd;
    //char response[1024];
} thread_data_t;

///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void waitForConnections(int server_fd);
void * attentionThread(void * arg);

void detectInterruption(int signal);
void setupMask();

///// MAIN FUNCTION
int main(int argc, char * argv[]){
    int server_fd;

    printf("\n=== SIMPLE BANK SERVER ===\n");

    // Check the correct arguments
    if (argc != 2){
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();
    setupMask();

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

///// FUNCTION DEFINITIONS
/*Explanation to the user of the parameters required to run the program*/
void usage(char * program){
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*Modify the signal handlers for specific events*/
void setupHandlers(){
    struct sigaction new_action;

    // Prepare structure, block all signals
    new_action.sa_handler = detectInterruption;
    // Clear Flags
    new_action.sa_flags = 0;
    sigfillset(&new_action.sa_mask);

    // Catch signal CTRL-C
    sigaction(SIGINT, &new_action, NULL); // Do i put old action?
}

// Signal handler
void detectInterruption(int signal){
    printf("Detected CTRL-C\n");

    interrupted = 1; // Change global variable, only way to do it =(
}

// Modify the signal mask. Define to block SIGINT
void setupMask(){
    sigset_t mask;

    // Add all signals to block
    sigfillset(&mask);
    sigdelset(&mask, SIGINT); // Dont block SIGINT

    // Apply the set to the program. The program has a "default" set (think of it as a "third set"), and it is now replaced by the new_set
    sigprocmask(SIG_BLOCK,&mask,NULL);
}

/*Main loop to wait for incomming connections*/
void waitForConnections(int server_fd){ //TODO
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    thread_data_t * connection_data = NULL;
    int poll_response;

    // Create a structure array to hold the file descriptors to poll
    struct pollfd test_fds[1];
    // Fill in the structure
    test_fds[0].fd = server_fd;
    test_fds[0].events = POLLIN;    // Check for incomming data

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

    // Create Threads for the pool
    for (int i = 0; i < POOLSIZE; i++){
            
    }
    

    while (!interrupted){
        while(!interrupted){ // While to keep polling
            poll_response = poll(test_fds,1, TIMEOUT);
            if(poll_response == 0){ // timeout ended
                printf(".");
                fflush(stdout); // mandatory print
            }else if(poll_response > 0){  // check if
                if(test_fds[0].revents & POLLIN){ // check bitmask returned events if it was CTRL - C
                    // ACCEPT
                    // Wait for a client connection
                    client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
                    if (client_fd == -1){
                        fatalError("ERROR: accept");
                    }
                    break;
                }
            }else{ // if less than 0, poll error
                perror("Poll failed server");
                break;
            }
        }
        if(!interrupted){
            int *pcclient = dequeue();
            if(pcclient != NULL){
                attentionThread(pc)
            }
            // Get the data from the client
            inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
            printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

            // Prepare the structure to send to the thread, thread t
            connection_data = malloc(sizeof(thread_data_t));
            connection_data->connection_fd = client_fd;
            
            //sprintf(connection_data->response, "%s",  temp);//finish the connection
            // CREATE A THREAD
            if (pthread_create(&new_tid, NULL, attentionThread, connection_data) == 0)
                printf("Thread created\n");
        }
    }

}

/*Hear the request from the client and send an answer, every thread executes this*/
void * attentionThread(void * arg){
    // Receive the data for the bank, mutexes and socket file descriptor
    thread_data_t* data = arg;

    // Poll stuff
    int poll_response;

    // Create a structure array to hold the file descriptors to poll
    struct pollfd test_fds[1];
    // Fill in the structure
    test_fds[0].fd = data->connection_fd;
    test_fds[0].events = POLLIN;    // Check for incomming data

    int in = 0;

    // check if account exists, then do operations
    while (!interrupted) {
        // Loop to listen for messages from the client
        while(!interrupted){ // While to keep polling /
            poll_response = poll(test_fds,1, TIMEOUT);
            if(poll_response == 0){ // timeout ended
                fflush(stdout); // mandatory print
            }else if(poll_response > 0){  // check if
                if(test_fds[0].revents & POLLIN){ // check bitmask returned events if it was CTRL - C
                    in = 1;
                    break;
                }
            }else{ // if less than 0, poll error
                perror("Poll failed server, attention thread");
                break;
            }
        }

        if(!interrupted && in) { // does the stuff

            // sprintf(buffer, "%i 0",  BYE);//finish the connection
            //sendString(data->connection_fd,response, sizeof(response) + 1);
            write(data->connection_fd, response, sizeof(response) + 1); /*-1:'\0'*/
        }
    }

    free(data);
    pthread_exit(NULL);
}

