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
#include <unistd.h> // for read(), write(), and close(), and access()
#include <strings.h> // for bzero() (legacy BSD functions)
#include <ifaddrs.h> // for getifaddrs()
#include <string.h> // for strcpy()
#include <netdb.h> // used for struct in_addr
#include <arpa/inet.h> // used for inet_ntoa()
#include <dirent.h> // for struct dirent and use of directory commands

// set to 0 when done debugging program
#define TEST 1

// code sourced form web archive of:
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
	fprintf(stderr, "Server: %s\tPort: %d\n", server_addr[0], atoi(server_addr[1]));
	return server_addr;
}

// taken from cs372 project 1
// takes ip and port as args and forms an array containing both pieces of info
// returns string array
char **connectionAddress(char *connection_addr, char *connection_port)
{
    char **connection_address = NULL;
    connection_address = (char **)calloc((size_t)2, sizeof(char *));
    connection_address[0] = connection_addr;
    connection_address[1] = connection_port;
    return connection_address;
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
    if(listen(sockfd, 1) < 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not begin listening\n");
    }
    getHost(server_addr[1]);
	return sockfd;
}

// taken from cs344 project 4 (key-based encryption)
// connects to socket server
int socketConnect(char **connection_address)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    server = gethostbyname(connection_address[0]);
    int ftsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(ftsockfd < 0)
    {
        fprintf(stderr, "[chatclient] ERROR! opening socket connection\n");
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(connection_address[1]));
    if(connect(ftsockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "[chatclient] ERROR! connecting to server port %s:%s\n", connection_address[0], connection_address[1]);
        close(ftsockfd);
        return -1;
    }
    return ftsockfd;
}

// taken from cs344 project 4 (key-based encryption)
// gets length of file (number of characters)
int getfilelength(FILE *fp)
{
    int filelength = -1;
    if(fp == NULL)
    {
        fprintf(stderr, "ftserver] ERROR! unable to read file to get file length\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    filelength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return filelength;
}

// taken from cs344 project 4 (key-based encryption)
// gets contents of text file and returns contents as string
// if there is no file or if an error opening file or getting file contents happens,
// returns a null to catch error rather than having a memory error
// using idea for access gained from post on stackoverflow
// http://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform
// post by Graeme Perrow
char *getFileContents(char *filename)
{
    // access checks that the file exists before attempting to open file through program which would cause
    // a memory error and a program crash
    if(access(filename, F_OK) != -1)
    {
        FILE *fp = NULL;
        char *filecontents = NULL;
        int filelength = -1;
        fp = fopen(filename, "r");
        if(fp == NULL)
        {
            fprintf(stderr, "[ftserver] ERROR! unable to open file for sending\n");
            return NULL;
        }
        filelength = getfilelength(fp);
        if(filelength <= 0)
        {
            return NULL;
        }
        filecontents = (char *)calloc((size_t)filelength, sizeof(char));
        if(filecontents == NULL)
        {
            fprintf(stderr, "[ftserver] ERROR! unable to allocate memory for storing text\n");
            fclose(fp);
            return NULL;
        }
        fread(filecontents, 1, filelength, fp);
        fclose(fp);
        return filecontents;
    }
    else
    {
        return NULL;
    }
}

// reads incoming data from client up to 128 bytes of data
// if command is '-l', returns 0
// if command is '-g', returns 1
int readCommands(int newsockfd)
{
    int readResult, sendResult;
    char clientCommand[3];
    bzero(&clientCommand, (size_t)3);
	if(read(newsockfd, &clientCommand, (size_t)2) <= 0)
	{
        fprintf(stderr, "[ftserver] ERROR! did not receive client command\n");
        return -1;
	}
#if TEST
    fprintf(stderr, "[DEBUG] Received command: %s\n", clientCommand);
#endif
    // if command received is '-l', respond to client with '-l'
    if(strcmp(clientCommand, "-l") == 0)
    {
        printf("Directory requested from client...\n");
        if(write(newsockfd, "-l", (size_t)2) <= 0)
        {
            fprintf(stderr, "[ftserver] ERROR! did not respond to client command request\n");
            return -1;
        }
        return 0;
    }
    // if command received is '-g', respond to client with '-g'
    else if(strcmp(clientCommand, "-g") == 0)
    {
        printf("File transfer requested from client...\n");
        if(write(newsockfd, "-g", (size_t)2) <= 0)
        {
            fprintf(stderr, "[ftserver] ERROR! did not respond to client command request\n");
            return -1;
        }
        return 1;
    }
    // if command received is invalid or not received, respond to client with error message
    else
    {
        write(newsockfd, "[ftserver] ERROR! did not receive valid command. use '-g' or '-l'\n", (size_t)64);
        return -1;
    }
}

// sends list of files currently in directory across socket connection
// up to 10 files
void sendList(int newsockfd)
{
    DIR *dirp = NULL;
    struct dirent *ptr = NULL;
    dirp = opendir(".");
    while((ptr = readdir(dirp)) != NULL)
    {
        if(strstr(ptr->d_name, ".txt") != NULL)
        {
            if(write(newsockfd, ptr->d_name, (size_t)strlen(ptr->d_name)) <= 0)
            {
                fprintf(stderr, "[ftserver] ERROR! did not write filename to client\n");
                return;
            }
            if(read(newsockfd, ptr->d_name, (size_t)strlen(ptr->d_name)) <= 0)
            {
                fprintf(stderr, "[ftserver] ERROR! did not read filename response from client\n");
            }
#if TEST    
            fprintf(stderr, "[DEBUG]%s\n", ptr->d_name);
#endif
        }
    }
    if(write(newsockfd, "endlist", (size_t)8) <= 0)
    {
        fprintf(stderr, "[ftserver] did not successfully send end of text file list\n");
    }
    return;
}

// sends file present in directory across socket connection
void sendFile(int ftsockfd)
{
    char clientFileRequest[64];
    // char clientDataPort[6];
    // char clientHostname[16];
    // char clientDataPort2[6];
    bzero(&clientFileRequest, (size_t)64);
    // bzero(&clientDataPort, (size_t)6);
    // bzero(&clientHostname, (size_t)16);
    // bzero(&clientDataPort2, (size_t)6);
    // read file request from client
    if(read(ftsockfd, &clientFileRequest, (size_t)64) <= 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not receive client file request\n");
        return;
    }
#if TEST
    fprintf(stderr, "[DEBUG] received file request: %s\n", clientFileRequest);
#endif
    // respond to client with file request
    if(write(ftsockfd, &clientFileRequest, (size_t)64) <= 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not respond to client file request\n");
        return;
    }
    /*
    // read client data port from client
    if(read(newsockfd, &clientDataPort, (size_t)6) <= 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not receive client data port request\n");
        return;
    }
#if TEST
    fprintf(stderr, "[DEBUG] received data port request: %s\n", clientDataPort);
#endif
    // respond to client with data port
    if(write(newsockfd, &clientDataPort, (size_t)6) <= 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not respond to client data port request\n");
        return;
    }
    // read client ip from client
    if(read(newsockfd, &clientHostname, (size_t)16) <= 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not receive client host name\n");
        return;
    }
#if TEST
    fprintf(stderr, "[DEBUG] received client hostname: %s\n", clientHostname);
#endif
    // respond to client with host name
    if(write(newsockfd, &clientHostname, (size_t)16) <= 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not resond to client host name\n");
        return;
    }
#if TEST
    fprintf(stderr, "[DEBUG] (TCP DATA CONNECTION CONNECTS HERE)\n");
#endif
    strtok(clientHostname, "\0");
    char **connection_address = connectionAddress(clientHostname, clientDataPort);
    if(read(newsockfd, &clientDataPort2, (size_t)6) <= 0)
    {
        fprintf(stderr, "[ftserver] ERROR! did not receive client server listening notification\n");
        return;
    }
    if(strcmp(clientDataPort, clientDataPort2) != 0)
    {
        fprintf(stderr, "[ftserver] ERROR %s != %s\n", clientDataPort, clientDataPort2);
        return;
    }
    int ftsockfd = socketConnect(connection_address);
#if TEST
    fprintf(stderr, "[DEBUG] clientHostname %s strlen %d clientDataPort %s strlen %d\n", clientHostname, strlen(clientHostname), clientDataPort, strlen(clientDataPort));
    fprintf(stderr, "[DEBUG] newsockfd %d ftsockfd %d\n", newsockfd, ftsockfd);
    fprintf(stderr, "[DEBUG] %s == %s\n", clientDataPort, clientDataPort2);
    fprintf(stderr, "[DEBUG] (TCP DATA CONNECTION CONNECTED HERE)\n");
#endif
    */
    // get and send file contents
    char *fileContents = NULL;
    fileContents = getFileContents(clientFileRequest);
    if(fileContents == NULL)
    {
        printf("File not found. Sending error message to client...\n");
        char *errmsg = "Server says FILE NOT FOUND";
        size_t errmsgLength = strlen(errmsg);
        if(write(ftsockfd, errmsg, errmsgLength) <= 0)
        {
            fprintf(stderr, "[ftserver] ERROR! did not send error message to client\n");
        }
    }
    else
    {
        printf("Sending requested file to client...\n");
#if TEST
        fprintf(stderr, "%s\n", fileContents);
#endif
        size_t fileContentsLength = strlen(fileContents);
#if TEST
        fprintf(stderr, "%d\n", (int)fileContentsLength);
#endif
        if(write(ftsockfd, fileContents, fileContentsLength) < 0)
//        if(write(newsockfd, fileContents, fileContentsLength) <= 0)
        {
            fprintf(stderr, "[ftserver] ERROR! did not send file to client\n");
            free(fileContents);
            return;
        }
        close(ftsockfd);
        free(fileContents);
    }
    close(ftsockfd);
    return;
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
	int procedure;
    // for data connection
    int ftsockfd;
    char **connection_address = NULL;
    char clientHostname[16];
    char clientDataPort[6];
    char clientReady[7];
    bzero(&clientHostname, (size_t)16);
    bzero(&clientDataPort, (size_t)6);
    bzero(&clientReady, (size_t)7);
	while(1)
	{
		printf("Waiting for connection from client...\n");
		// open new data connection with incoming client connection request
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		// if failure to accept incoming client connection request, close connection and listen
		if(newsockfd < 0)
		{
			fprintf(stderr, "[ftserver] ERROR! did not accept connection from client\n");
			close(newsockfd);
			continue;
		}
        printf("Accepted connection from client...\n");
		// read command sent from client
        procedure = readCommands(newsockfd);
        // if '-l' or '-g' is client command, gather data port information
        if(procedure == 0 || procedure == 1)
        {
            if(read(newsockfd, &clientHostname, (size_t)16) <= 0)
            {
                fprintf(stderr, "[ftserver] ERROR! did not receive client host name\n");
                close(newsockfd);
                continue;
            }
            if(write(newsockfd, &clientHostname, (size_t)16) <=0)
            {
                fprintf(stderr, "[ftserver] ERROR! did not respond to client host name\n");
                close(newsockfd);
                continue;
            }
            if(read(newsockfd, &clientDataPort, (size_t)6) <=0)
            {
                fprintf(stderr, "[ftserver] ERROR! did not receive client data port\n");
                close(newsockfd);
                continue;
            }
            if(write(newsockfd, &clientDataPort, (size_t)6) <=0)
            {
                fprintf(stderr, "[ftserver] ERROR! did not respond to client data port\n");
                close(newsockfd);
                continue;
            }
            if(read(newsockfd, &clientReady, (size_t)7) <=0)
            {
                fprintf(stderr, "[ftserver] ERROR! did not receive ready message from client\n");
                close(newsockfd);
                continue;
            }
            connection_address = connectionAddress(clientHostname, clientDataPort);
            ftsockfd = socketConnect(connection_address);
#if TEST
            fprintf(stderr, "[DEBUG] (DATA CONNECTION) connected to %s %s\n", clientHostname, clientDataPort);
#endif
        }
        if(procedure == 0)
        {
            printf("Sending directory list to client...\n");
            sendList(ftsockfd);
        }
        // otherwise, if command received is to return a file, runs function to return file
        else if(procedure == 1)
        {
            sendFile(ftsockfd);
        }
		printf("Closing connection from client...\n");
		// close data connection
        if(procedure == 0 || procedure == 1)
        {
            close(ftsockfd);
        }
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
