/*
 ** DepthServer.hpp - a header file for the server class 
 */

#ifndef DEPTHSERVER_HPP
#define DEPTHSERVER_HPP

#define NO_COMPRESSION 0
#define DELTA_COMPRESSION 1
#define PNG_COMPRESSION 2 

#include "Server.hpp"

typedef unsigned char uchar;

class DepthServer : public Server
{
	public:
		DepthServer(const char* portNumber, int rowCount, int colCount);

		int  SendMatrix(const float* matrix);
		int  SendMatrixCompressedWithPNG(const uchar* toSend);
				
	private:
		int _rows, _columns;
};

#endif
