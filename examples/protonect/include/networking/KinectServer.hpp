/*
 ** KinectServer.hpp - a header file for the server class 
 */

#ifndef DEPTHSERVER_HPP
#define DEPTHSERVER_HPP

#include "Server.hpp"
#include "libfreenect2/JpegCompressor.hpp"

typedef unsigned char uchar;

class KinectServer : public Server
{
	public:
		KinectServer(const char* portNumber, int rowCount, int colCount);
		~KinectServer();

		int  SendMatrix(const float* matrix);
		int  SendMatrixCompressedWithPNG(const uchar* toSend);
		int  SendMatrixCompressedWithJPEG(const uchar* uncompressed);

	private:
		int _rows, _columns;
		JpegCompressor* _jpegCompressor;
};

#endif
