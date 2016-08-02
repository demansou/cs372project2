/**************************************************************************************
 * Author: Daniel Mansour
 * Creation Date: 08-01-2016
 * File Name: ftserver.c
 * Description: Server which listens for client connection requests on a port,
 * 		establishes a TCP control connection on port with client, and listens
 * 		to commands from client. When command is sent from client, depending
 * 		on command sent, server creates TCP data connection to client and sends
 * 		error message, directory listing, or sends file. Server closes TCP data
 * 		connection. Then client closes TCP control connection. Server waits for
 * 		new connections until terminated by SIGINT.
 **************************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h> // for socklen_t
#include <sys/types.h> // for size_t, getifaddrs()
#include <netinet/in.h> // for struct sockaddr_in
#include <unistd.h> // for read(), write(), and close()
#include <strings.h> // for bzero() (legacy BSD functions)
#include <ifaddrs.h> // for getifaddrs()
//#include <string.h>
#include <netdb.h> // used for struct in_addr
#include <arpa/inet.h> // used for inet_ntoa()

// set to 0 when done debugging program
#define TEST 1

char *getHost();

// returns server IP address (localhost) and listening port from command line
// returns as array of strings
// listening port = argv[1] (needs atoi())
char **getPort(int argc, char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "[ftserver] ERROR! Correct format 'ftserver [listening port]'\n");
		exit(2);
	}
	char **server_addr = NULL;
	server_addr = (char **)calloc((size_t)2, sizeof(char *));
	server_addr[0] = getHost();
	server_addr[1] = argv[1];
#if TEST
	fprintf(stderr, "[DEBUG] Server: %s\tPort: %d\n", server_addr[0], atoi(server_addr[1]));
#endif
	return server_addr;
}

// code sourced from web archive of:
// http://guy-lecky-thompson.suite101.com/socket-programming-gethostbyname-a19557
// gets host by hostname and retrieves ip address from hostname
char *getHost()
{
    char hostname[255];
    gethostname(hostname, 255);
    struct hostent *host_entry;
    host_entry = gethostbyname(hostname);
    char *localip;
    localip = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);
    return localip;
}

// creates endpoint for network communication on listening port
// returns socket identifier
int createSocket(int argc, char *argv[])
{
	char **server_addr = NULL;
	server_addr = getPort(argc, argv);
	// create endpoint for communication (returned as descriptor)
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "[ftserver] ERROR! socket endpoint not created\n");
		exit(1);
	}
	// set endpoint listening port
	struct sockaddr_in serv_addr;
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_addr[0]);
	serv_addr.sin_port = htons(atoi(server_addr[1]));
	if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "[ftserver] ERROR! did not bind to port\n");
		close(sockfd);
		exit(1);
	}
    // listen on port
    if(listen(sockfd, 1) == -1)
    {
        fprintf(stderr, "ftserver] ERROR! did not begin listening\n");
    }
    getHost(server_addr[1]);
	return sockfd;
}

// reads incoming data from client up to 128 bytes of data
// returns received data as string or NULL if no message received
char *readCommand(int newsockfd)
{
    int readResult;
	char *clientCommand = NULL;
	clientCommand = (char *)calloc(128, sizeof(char));
	readResult = read(newsockfd, &clientCommand, 128);
    if(readResult < 0)
	{
		return NULL;
	}
#if TEST
	fprintf(stderr, "[DEBUG] Received message: %s\n", clientCommand);
#endif
	return clientCommand;
}

// listens for inbound connection requests on server's listening port
// if connection created, reads messages from client and creates response
// closes client connection after every interaction
// server ends only after SIGINT
void runServer(int argc, char *argv[])
{
    // create socket and listen
	int sockfd = createSocket(argc, argv);
	int newsockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	clilen = sizeof(cli_addr);
	char *clientCommand = NULL;
	while(1)
	{
		fprintf(stderr, "[ftserver] Waiting for connection from client...\n");
		// open new data connection with incoming client connection request
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		// if failure to accept incoming client connection request, close connection and listen
		if(newsockfd < 0)
		{
			fprintf(stderr, "[ftserver] ERROR! did not accept connection from client\n");
			close(newsockfd);
			continue;
		}
		// read command sent from client
		clientCommand = readCommand(newsockfd);
		// if there is no command received, close connection and listen
		if(clientCommand == NULL)
		{
			fprintf(stderr, "[ftserver] ERROR! did not receive command from client\n");
			close(newsockfd);
			continue;
		}
		fprintf(stderr, "[ftserver] Closing connection from client...\n");
		// close data connection
		close(newsockfd);
	}
	close(sockfd);
	return;	
}

// main function takes args from command line
// format: ftserver [listening port]
int main(int argc, char *argv[])
{
	runServer(argc, argv);
	return 0;
}
