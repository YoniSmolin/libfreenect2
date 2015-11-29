/*
 ** KinectServer.cpp - a server for Kinect outputs
 */

#include <stdio.h>
#include <stdexcept> // exceptions
#include <iostream>

#include <libfreenect2/JpegCompressor.hpp>

/////////////////////////////////////   public 	 //////////////////////////////////////////

JpegCompressor::JpegCompressor(int rowCount, int colCount) : _rows(rowCount), _columns(colCount)
{
	_cinfo = new jpeg_compress_struct;
	_jerr = new jpeg_error_mgr;

	_cinfo->err = jpeg_std_error(_jerr);	
	_jerr->error_exit = ExitOnError;

	jpeg_create_compress(_cinfo);
}

JpegCompressor::~JpegCompressor()
{
	jpeg_destroy_compress(_cinfo);
	delete _cinfo;
	delete _jerr;
	delete[] _jpegBuffer;
}

int JpegCompressor::Compress(const uchar* uncompressed)
{
	_jpegBuffer = new uchar[1920*1080*4];

	// destination - where to write the JPEG ?
	jpeg_mem_dest(_cinfo, &_jpegBuffer, &_jpegLength);

	// check for trouble
	if (_cinfo->progressive_mode)
		AbortAndThrow(_cinfo, "Tegra HW doesn't support progressive JPEG; use TurboJPEG");

	if (!_cinfo->tegra_acceleration)
		AbortAndThrow(_cinfo, "Tegra HW acceleration is disabled unexpectedly");

	// input image parameters
	_cinfo->image_width = _columns;
	std::cout << "#### Control: width = " << _columns << std::endl;
	_cinfo->image_height = _rows;
	std::cout << "#### Control: height = " << _rows << std::endl;
	_cinfo->input_components = 3; // color components per pixel
	_cinfo->in_color_space = JCS_RGB;
	std::cout << "#### Control: Jpeg compression parameters set successfully !" << std::endl;
	// perform 2x scaling (shrink every dimension by a factor of 2)
	_cinfo->scale_num = 1;
	_cinfo->scale_denom = 2;

	// set default parameteres
	jpeg_set_defaults(_cinfo);
	std::cout << "#### Control: Default parameters set successfully" << std::endl;
	// write the jpeg header
	jpeg_start_compress(_cinfo, TRUE);

	std::cout << "#### Control: compression started successfully" << std::endl;
	// do the actual compression
        if (jpeg_write_scanlines(_cinfo, const_cast<uchar**>(&uncompressed), _rows) != _cinfo->image_height)
		AbortAndThrow(_cinfo, "Incomplete encoding result");

	jpeg_finish_compress(_cinfo);

	return (int)_jpegLength;
}

const uchar* JpegCompressor::GetCompressed()
{
	return _jpegBuffer;
}

void JpegCompressor::ExitOnError(j_common_ptr info)
{
  char buffer[JMSG_LENGTH_MAX];
  info->err->format_message(info, buffer);
  AbortAndThrow((j_compress_ptr)info, buffer);
}

void JpegCompressor::AbortAndThrow(j_compress_ptr cinfo, const std::string buffer)
{
  jpeg_abort_compress(cinfo);
  throw std::runtime_error(buffer);
}
