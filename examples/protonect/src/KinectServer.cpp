/*
 ** Server.cpp - class implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <string.h>
#include <stdexcept> // exceptions
#include <networking/KinectServer.hpp>

#include <opencv2/highgui/highgui.hpp> // PNG compression

#define BYTES_PER_COMPRESSED_PIXEL 2
#define BYTES_IN_HEADER 3
#define BYTE 8
#define BITS_TWO_TO_NINE 0x000001fe

using namespace std;
using namespace cv;

KinectServer::KinectServer(const char* portNumber, int rowCount, int colCount) : Server(portNumber), _rows(rowCount), _columns(colCount) {}

int  KinectServer::SendMatrix(const float* matrix)
{
	if (sizeof(float) != 4)
		fprintf(stderr, "Server::SendMatrix - Float size must be 32 bits\n");

	return SendMessage((char*) matrix, _rows * _columns * sizeof(float));
}

int  KinectServer::SendMatrixCompressedWithPNG(const uchar* toSend)
{
	Mat toSendMat = Mat(_rows, _columns, CV_16UC1, const_cast<uchar*>(toSend)); 

	// wrap the compressed image buffer with a vector
	vector<uchar> compressed;
	// encode the image to PNG
	imencode(".png", toSendMat, compressed);	
	
	// prepare the header
	char header[BYTES_IN_HEADER];
	int compressedSize = compressed.size();
	header[0] = compressedSize;
	header[1] = compressedSize >> BYTE;
	header[2] = compressedSize >> 2*BYTE;

	// send the header
	int sentBytesHeader = SendMessage(header, BYTES_IN_HEADER);
	if (sentBytesHeader < BYTES_IN_HEADER) return 0;

	// send the PNG body
	return sentBytesHeader + SendMessage((char*)&compressed[0], compressedSize);
}
