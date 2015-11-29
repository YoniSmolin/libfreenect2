/*
 ** JpegCompressor.hpp 
 */

#ifndef JPEGCOMPRESSOR_HPP
#define JPEGCOMPRESSOR_HPP

#include <string>

#include "nv_headers/jpeglib.h"

typedef unsigned char uchar;

class JpegCompressor
{
	public:
		JpegCompressor(int rowCount, int colCount);
		
		static void ExitOnError(j_common_ptr info);		
		static void AbortAndThrow(j_compress_ptr cinfo, const std::string buffer);

		int Compress(const uchar* image);
		const uchar* GetCompressed();

		~JpegCompressor();

	private:
		int _rows, _columns;
		
		uchar* _jpegBuffer;
		unsigned long  _jpegLength;

		struct jpeg_compress_struct* _cinfo;
		struct jpeg_error_mgr* _jerr;
};

#endif
