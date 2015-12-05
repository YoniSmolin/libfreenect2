/*
 ** JpegCompressor.hpp 
 */

#ifndef JPEGCOMPRESSOR_HPP
#define JPEGCOMPRESSOR_HPP

#include <string>

#include <turbojpeg.h>

typedef unsigned char uchar;

class JpegCompressor
{
	public:
		JpegCompressor(int rowCount, int colCount, int jpegQuality = 80); // jpeg Quality is between 0 and 100
		
		int Compress(const uchar* image);
		const uchar* GetCompressed();

		~JpegCompressor();

	private:
		int _rows, _columns;
		
		int _jpegQuality;
		uchar* _jpegBuffer;
		unsigned long _jpegCompressedSize;

		tjhandle _compressor;
};

#endif
