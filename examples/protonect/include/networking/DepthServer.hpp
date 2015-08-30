/*
 ** DepthServer.hpp - a header file for the server class 
 */

#ifndef DEPTHSERVER_HPP
#define DEPTHSERVER_HPP

#include "Server.hpp"

typedef unsigned char uchar;

class DepthServer : public Server
{
	public:
		DepthServer(const char* portNumber, bool useCompression, int rowCount, int colCount);

		int  SendMatrix(const uchar* matrix);
		int  SendMatrix(const float* matrix);
				
		~DepthServer();
	private:
		int  SendMatrixCompressed(const uchar* toSend);
		
		int _rows, _columns;
		char* _compressedImageBuffer;
		uchar* _previousFrame;
		bool _usingCompression, _expectingFirstFrame;
};

#endif
