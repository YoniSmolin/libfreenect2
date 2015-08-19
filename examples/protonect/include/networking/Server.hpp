/*
 ** Server.hpp - a header file for the server class 
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <exception>

typedef unsigned char uchar;

class CompressionError : public std::exception
{
	virtual const char* what() const throw()
	{
		return "Overflow - found a pixel with difference greater than 32[cm]";
	}
};

class Server
{
	int _sockfd; // TODO: differentiate to listener and conenction sockets
	const int BACKLOG;

	public:
		Server(const char* portNumber, int rowCount, int colCount);

		void WaitForClient();
		int  SendMessage(const char* message, int length);
		void CloseConnection();
		int  SendMatrix(const char* matrix);
		int  SendMatrix(const float* matrix);
		int  SendCompressed(const uchar* reference, uchar* toSend);
		
		~Server();
	private:
		void sigchld_handler(int s);
		void* get_in_addr(struct sockaddr *sa);
		int _rows, _columns;
		char* _compressedImageBuffer;
};

#endif
