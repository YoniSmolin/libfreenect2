/*
 ** Server.cpp - class implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <string.h>

#include <networking/DepthServer.hpp>

#define BYTES_PER_COMPRESSED_PIXEL 4
#define BYTES_IN_HEADER 3
#define BYTE 8
#define BITS_TWO_TO_NINE 0x000001fe

using namespace std;

DepthServer::DepthServer(const char* portNumber, bool useCompression, int rowCount, int colCount) : Server(portNumber), _rows(rowCount), _columns(colCount), _usingCompression(useCompression), _previousFrame(NULL), _expectingFirstFrame(true)
{
	if (useCompression)
	{	
		_compressedImageBuffer = new char[_rows * _columns * BYTES_PER_COMPRESSED_PIXEL + BYTES_IN_HEADER];
		_previousFrame = new uchar[_rows * _columns];
	}
}

int  DepthServer::SendMatrix(const uchar* matrix)
{
	// inform the client about the type of depth stream - with or without connection
	if (_expectingFirstFrame)
	{
		int numBytesSent = SendMessage((char*)&_usingCompression, sizeof(bool));
		if (numBytesSent != sizeof(bool))
			return 0;
	}

	if (_usingCompression)
		return SendMatrixCompressed(matrix);
	else	
		return SendMessage((char*)matrix, _rows * _columns);
}

int  DepthServer::SendMatrix(const float* matrix)
{
	if (sizeof(float) != 4)
		fprintf(stderr, "Server::SendMatrix - Float size must be 32 bits\n");

	return SendMessage((char*) matrix, _rows * _columns * sizeof(float));
}

int  DepthServer::SendMatrixCompressed(const uchar* toSend)
{
	int numBytesSent = 0;

	if (_expectingFirstFrame)
	{
		_expectingFirstFrame = false;
		numBytesSent =  SendMessage((char*) toSend, _rows * _columns);
	}
	else	
	{	
		int indexCompressed = BYTES_IN_HEADER;

		// compress data
		for (int row = 0; row < _rows; row++)
		{
			for (int col = 0; col < _columns; col++)
			{
				int indexOriginal = row * _columns + col;
				int difference = (int)toSend[indexOriginal] - (int)_previousFrame[indexOriginal]; // the casts here are important in order to preserve sign information
				bool sign = difference < 0;
				int absoluteValue = sign ? -difference : difference;

				if (difference != 0)
				{
					_compressedImageBuffer[indexCompressed++] = indexOriginal;      // store 8 LSBs 
					_compressedImageBuffer[indexCompressed++] = indexOriginal >> BYTE; // store following (more significant) 8 LSBs
					_compressedImageBuffer[indexCompressed++] = (indexOriginal >> 2*BYTE) + ((sign ? 1:0) << (BYTE - 1)); // final 2 bits of the index and the sign of the difference are stored here
					_compressedImageBuffer[indexCompressed++] = absoluteValue; 
				}

			}
		}

		// prepare image header - contains the length of the compressed image
		int compressedLength = indexCompressed - BYTES_IN_HEADER;
		_compressedImageBuffer[0] = compressedLength;
		_compressedImageBuffer[1] = compressedLength >> BYTE;
		_compressedImageBuffer[2] = compressedLength >> 2*BYTE;

		numBytesSent = SendMessage(_compressedImageBuffer, indexCompressed);
	}

	memcpy(_previousFrame, toSend, _rows * _columns);
	return numBytesSent;
}


DepthServer::~DepthServer()
{
	if (_compressedImageBuffer != NULL)
		delete[] _compressedImageBuffer;
}
