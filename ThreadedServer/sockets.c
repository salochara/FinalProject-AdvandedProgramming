/*
    Helper methods to use with sockets
    The basic functions that should be done on any client / server program
    - Creation of the socket on the server and binding
    - Printing the local addresses
    - Creation of a socket on a client
    - Error validation when sending or receiving messages

    Gilberto Echeverria
    gilecheverria@yahoo.com
    31/03/2018
*/

#include "sockets.h"

/*
	Show the local IP addresses, to allow testing
	Based on code from:
		https://stackoverflow.com/questions/20800319/how-do-i-get-my-ip-address-in-c-on-linux
*/
void printLocalIPs()
{
	struct ifaddrs * addrs;
	struct ifaddrs * tmp;

    // Get the list of IP addresses used by this machine
	getifaddrs(&addrs);
	tmp = addrs;

    printf("Server IP addresses:\n");

	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
            // Get the address structure casting as IPv4
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            // Print the ip address of the local machine
			printf("%s: %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
		}
        // Advance in the linked list
		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
}

/*
    Prepare and open the listening socket
    Returns the file descriptor for the socket
    Remember to close the socket when finished
*/
int initServer(char * port, int max_queue)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int server_fd;
    int reuse = 1;

    // Prepare the hints structure
    // Clear the structure for the server configuration
    bzero(&hints, sizeof hints);
    // Set to use IPV4
    hints.ai_family = AF_INET;
    // Use stream sockets
    hints.ai_socktype = SOCK_STREAM;
    // Get the local IP address automatically
    hints.ai_flags = AI_PASSIVE;

    // GETADDRINFO
    // Use the presets to get the actual information for the socket
    // The result is stored in 'server_info'
    if (getaddrinfo(NULL, port, &hints, &server_info) == -1)
    {
        fatalError("ERROR: getaddrinfo");
    }

    // SOCKET
    // Open the socket using the information obtained
    server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_fd == -1) 
    {
        close(server_fd);
        fatalError("ERROR: socket");
    }

    // SETSOCKOPT
    // Allow reuse of the same port even when the server does not close correctly
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (int)) == -1)
    {
        fatalError("ERROR: setsockopt");
    }

    // BIND
    // Connect the port with the desired port
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        fatalError("ERROR: bind");
    }

    // LISTEN
    // Start listening for incomming connections
    if (listen(server_fd, max_queue) == -1)
    {
        fatalError("ERROR: listen");
    }

    // FREEADDRINFO
    // Free the memory used for the address info
    freeaddrinfo(server_info);

    printf("Server ready\n");

    return server_fd;
}

/*
    Open and connect the socket to the server
    Returns the file descriptor for the socket
    Remember to close the socket when finished
*/
int connectSocket(char * address, char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int connection_fd;

    // Prepare the hints structure
    // Clear the structure for the server configuration
    bzero(&hints, sizeof hints);
    // Set to use IPV4
    hints.ai_family = AF_INET;
    // Use stream sockets
    hints.ai_socktype = SOCK_STREAM;
    // Get the local IP address automatically
    hints.ai_flags = AI_PASSIVE;

    // GETADDRINFO
    // Use the presets to get the actual information for the socket
    // The result is stored in 'server_info'
    if (getaddrinfo(address, port, &hints, &server_info) == -1)
    {
        fatalError("ERROR: getaddrinfo");
    }

    // SOCKET
    // Open the socket using the information obtained
    connection_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (connection_fd == -1) 
    {
        close(connection_fd);
        fatalError("ERROR: socket");
    }

    // CONNECT
    // Connect to the server
    if (connect(connection_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        close(connection_fd);
        fatalError("ERROR: connect");
    }

    // FREEADDRINFO
    // Free the memory used for the address info
    freeaddrinfo(server_info);

    return connection_fd;
}

/*
    Receive a stream of data from a socket
    Receive the file descriptor of the socket, a pointer to where to store the data and the maximum size avaliable
    Returns 1 on successful receipt, or 0 if the connection has finished
*/
int recvString(int connection_fd, void * buffer, int size)
{
    int chars_read;

    // Clear the buffer
    bzero(buffer, size);

    // Read the request from the client
    chars_read = recv(connection_fd, buffer, size, 0);
    // Error when reading
    if ( chars_read == -1 )
    {
        fatalError("ERROR: recv");
    }
    // Connection finished
    if ( chars_read == 0 )
    {
        printf("Connection disconnected\n");
        return 0;
    }

    return 1;
}

/*
    Send a message with error validation
    Receive the file descriptor, the pointer to the data, and the size of the data to send
*/
void sendString(int connection_fd, void * buffer, int size)
{
    // Send a message to the client, including an extra character for the '\0'
    if ( send(connection_fd, buffer, size, 0) == -1 )
    {
        fatalError("ERROR: send");
    }
}
