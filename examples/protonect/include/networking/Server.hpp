/*
 ** Server.hpp - a header file for the server class 
 */

#ifndef SERVER_HPP
#define SERVER_HPP

class Server
{
	int _sockfd; // TODO: differentiate to listener and conenction sockets
	const int BACKLOG;

	public:
		Server(const char* portNumber);

		void WaitForClient();
		int  SendMessage(const char* message, int length);
		void CloseConnection();
		int  SendMatrix(const char* matrix, int rowCount, int colCount);
		int  SendMatrix(const float* matrix, int rowCount, int colCount);

	private:
		void sigchld_handler(int s);
		void* get_in_addr(struct sockaddr *sa);

};

#endif
