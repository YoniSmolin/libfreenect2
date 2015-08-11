/*
 ** Server.cpp - class implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <signal.h>
#include <sys/wait.h>

#include <iostream>
#include <string.h>

#include <networking/Server.hpp>

using namespace std;

Server::Server(const char* portNumber) : BACKLOG(1)
{ 
	struct addrinfo hints, *servinfo, *p; 
	int yes=1; 
	int rv;

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_UNSPEC; // either IPv4 or IPv6 
	hints.ai_socktype = SOCK_STREAM; // TCP stream
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, portNumber, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(_sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(_sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) 
	{
		fprintf(stderr, "server: failed to bind\n"); 
		exit(1); // TODO: throw an exception here
	}

	freeaddrinfo(servinfo);
}

void Server::WaitForClient()
{
	//struct sigaction sa;
	socklen_t sin_size;
	struct sockaddr_storage their_addr;
	char s[INET6_ADDRSTRLEN];

	if (listen(_sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	/* This will be relevant when we expect more than one client:
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	} */
	
	cout << "server: waiting for connection..." << endl;

	sin_size = sizeof their_addr;
	int newfd = accept(_sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if (_sockfd == -1)
	{
		perror("accept");
	}


	inet_ntop(their_addr.ss_family,	get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
	cout << "server: got connection from " << s << endl;
	
	close(_sockfd);
	
	_sockfd = newfd; // TODO: in the future, we would like to keep both sockets open,
			 //       one for listening and the other for sending data. 
}

int  Server::SendMessage(const char* message, int length)
{
	int bytesSent = send(_sockfd, message, length, 0);
	if (bytesSent == -1)
	{ 
		perror("send");
		exit(1);
	}
	
	return bytesSent;
}

int  Server::SendMatrix(const char* matrix, int rowCount, int colCount)
{
	int bytesSent = 0, bytesRemaining = rowCount * colCount;

	while(bytesSent	< bytesRemaining)
	{
		bytesSent += SendMessage(matrix + bytesSent, bytesRemaining);
		bytesRemaining -= bytesSent;
	}

	return bytesSent;
}


void Server::CloseConnection()
{
	close(_sockfd);
}

void Server::sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void* Server::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
