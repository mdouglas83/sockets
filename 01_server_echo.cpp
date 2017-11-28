#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"

static const int MAXPENDING = 5;
static const int BUFSIZE = 1024;

void HandleTCPClient(int cIntSocket);

int main(int argc, char *argv[]) {
	
	if (argc != 2)
		DieWithUserMessage("Parameter(s)", "<Server Port>");

	in_port_t servPort = atoi(argv[1]); // First arg: local port

	// Create socket for incoming conections
	int servSock; // Socket descriptor for server
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithSystemMessage("socket() failed");

	// Construct local address structure
	struct sockaddr_in servAddr;					// Local address
	memset(&servAddr, 0, sizeof(servAddr));			// Zero out structure
	servAddr.sin_family = AF_INET;					// IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);	// Any incoming interface
	servAddr.sin_port = htons(servPort);			// Local port

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
		DieWithSystemMessage("bind() failed");

	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0)
		DieWithSystemMessage("listen() failed");

	for (;;) { // Run forever
		struct sockaddr_in cIntAddr; // Client address
		
		// Set length of client address structure (in-out parameter)
		socklen_t cIntAddrLen = sizeof(cIntAddr);

		// Wait for client to connect
		int cIntSock = accept(servSock, (struct sockaddr *) &cIntAddr, &cIntAddrLen);
		if (cIntSock < 0)
			DieWithSystemMessage("accept() failed");

		// cIntSock is connected to a client!

		char cIntName[INET_ADDRSTRLEN]; // String to contain client address
		if (inet_ntop(AF_INET, &cIntAddr.sin_addr.s_addr, cIntName, sizeof(cIntName)) != NULL)
			printf("Handling client %s/%d\n", cIntName, ntohs(cIntAddr.sin_port));
		else
			puts("Unable to get client address");

		HandleTCPClient(cIntSock);
	}
}

void HandleTCPClient(int cIntSocket) {
	char buffer[BUFSIZE];	// Buffer for echo string

	// Receive message from client
	ssize_t numBytesRecd = recv(cIntSocket, buffer, BUFSIZE, 0);
	if (numBytesRecd < 0)
		DieWithSystemMessage("recv() failed");

	// Send received string and receive again until end of stream
	while (numBytesRecd > 0) {	// 0 indicates end of stream
		fputs(buffer, stdout);	// MD: Print the echo buffer
		// Echo message back to client
		ssize_t numBytesSent = send(cIntSocket, buffer, numBytesRecd, 0);
		if (numBytesSent < 0)
			DieWithSystemMessage("send() failed");
		else if (numBytesSent != numBytesRecd)
			DieWithUserMessage("send()", "sent unexpected number of bytes");

		// See if there is more data to receive
		numBytesRecd = recv(cIntSocket, buffer, BUFSIZE, 0);
		if (numBytesRecd < 0)
			DieWithSystemMessage("recv() failed");
	}

	fputc('\n', stdout);	// MD: Print a final linefeed
	close(cIntSocket);		// Close client socket
}