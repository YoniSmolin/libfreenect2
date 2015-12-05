/*
 ** KinectServer.cpp - a server for Kinect outputs
 */

#include <stdio.h>
#include <stdexcept> // exceptions
#include <iostream>

#include <libfreenect2/JpegCompressor.hpp>

/////////////////////////////////////   public 	 //////////////////////////////////////////

JpegCompressor::JpegCompressor(int rowCount, int colCount, int jpegQuality) : _rows(rowCount), _columns(colCount), _jpegQuality(jpegQuality)
{
	_compressor = tjInitCompress();
	if(_compressor == 0)
	{
		std::cerr << "failed to initialize turbojpeg decompressor! turbojpeg error: '" << tjGetErrorStr() << "'" << std::endl;
		return;
	}

	_jpegBuffer = new uchar[tjBufSize(_columns, _rows, TJSAMP_420)];

	std::cout << "JPEG compression will be performed with one chrominance component for every 2x2 block of pixels in the source image. To change this, edit src/JpegCompressor.cpp."
	     << std::endl;
}

JpegCompressor::~JpegCompressor()
{
	tjDestroy(_compressor);
	delete[] _jpegBuffer;
}

int JpegCompressor::Compress(const uchar* uncompressed)
{
	int result = tjCompress2(_compressor, const_cast<uchar*>(uncompressed), _columns, _columns * tjPixelSize[TJPF_RGBA], _rows, TJPF_RGBA,
				 &_jpegBuffer, &_jpegCompressedSize, TJSAMP_420, _jpegQuality, TJFLAG_NOREALLOC);

	if (result != 0)
	{
	      std::cerr << "failed to decompress rgb image! turbojpeg error: '" << tjGetErrorStr() << "'" << std::endl;
	      return 0;
	}

	return _jpegCompressedSize;
}

const uchar* JpegCompressor::GetCompressed()
{
	return _jpegBuffer;
}
