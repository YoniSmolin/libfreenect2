#define TEGRA_ACCELERATE

#include <stdio.h>	
#include <stdexcept>
#include <nv_headers/jpeglib.h>
#include <opencv2/opencv.hpp>
#include <cstdlib>

#define FILE_NAME "MyImage.jpeg"
#define OUTPUT_FILE_NAME "Recompressed.jpeg"

#define WIDTH  1920
#define HEIGHT 1080
#define PITCH  7680

typedef unsigned char uchar;

uchar* Decompress(std::string fileName);
void Compress(uchar* image, std::string fileName);

void abort_jpeg_error(j_common_ptr info, const char *msg);

struct jpeg_error_mgr jerr_d;
struct jpeg_error_mgr jerr_c;
struct jpeg_decompress_struct dinfo;
struct jpeg_compress_struct cinfo;

int main()
{
	// prepare JPEG decompression objects
	dinfo.err = jpeg_std_error(&jerr_d);
	jpeg_create_decompress(&dinfo);

	// load the image and decode
	uchar* decompressed = Decompress(FILE_NAME);
	if (decompressed == NULL) return 0;

//	cv::Mat image(HEIGHT, WIDTH, CV_8UC4, decompressed);

	// display the result
//	cv::imshow(FILE_NAME, image);
//	cv::waitKey(0);

	// conver RGBA to RGB
//	cv::Mat imageRGB(HEIGHT, WIDTH, CV_8UC3);
//	cv::cvtColor(image, imageRGB, cv::COLOR_RGBA2RGB);
	
	// display result
	//cv::imshow("RGBA", imageRGB);
	//cv::waitKey(0);

	// prepare JPEG compression objects
	cinfo.err = jpeg_std_error(&jerr_c); 	
	jpeg_create_compress(&cinfo);

	// pass the decompressed image for compression
	uchar* something = NULL;
	Compress(decompressed, OUTPUT_FILE_NAME);	

	// destoy JPEG (de)compression objects
	jpeg_destroy_decompress(&dinfo);
	jpeg_destroy_compress(&cinfo);

	return 0;
}

uchar* Decompress(std::string fileName)
{
	// read the image from file into a buffer
	FILE* infile;

	if ((infile = fopen(fileName.c_str(), "r")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", fileName.c_str());
		return NULL;
	}

	jpeg_stdio_src(&dinfo, infile);

	jpeg_read_header(&dinfo, TRUE);

	if (dinfo.progressive_mode)
		abort_jpeg_error((jpeg_common_struct*)&dinfo, "Tegra HW doesn't support progressive JPEG; use TurboJPEG");

	if (!dinfo.tegra_acceleration)
		abort_jpeg_error((jpeg_common_struct*)&dinfo, "Tegra HW acceleration is disabled unexpectedly");

	if (dinfo.image_width != WIDTH || dinfo.image_height != HEIGHT)
		abort_jpeg_error((jpeg_common_struct*)&dinfo, "image dimensions does not match preset");

	dinfo.out_color_space = JCS_RGBA_8888;

	jpeg_start_decompress(&dinfo);

	// Hardware acceleration returns the entire surface in one go.
	// The normal way with software decoding uses jpeg_read_scanlines with loop.
	if (jpeg_read_scanlines(&dinfo, NULL, 0) != dinfo.output_height)
		abort_jpeg_error((jpeg_common_struct*)&dinfo, "Incomplete decoding result");

	// Empirically: 1 surface for RGBA; 3 surfaces for YUV 
	size_t pitch = dinfo.jpegTegraMgr->pitch[0];
	unsigned char *surface = dinfo.jpegTegraMgr->buff[0];
	if (pitch == 0 || surface == NULL)
		abort_jpeg_error((jpeg_common_struct*)&dinfo, "Empty result buffer");

	//XXX this won't work for grayscale output where itch != width*bpp for grayscale output, 
	if (pitch != PITCH || dinfo.output_height != HEIGHT)
		abort_jpeg_error((jpeg_common_struct*)&dinfo, "buffer size mismatch");

	jpeg_finish_decompress(&dinfo);
	fclose(infile);

	return surface;
}

void Compress(uchar* image, std::string fileName)
{
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
//	int row_stride;		/* physical row width in image buffer */

	if ((outfile = fopen(fileName.c_str(), "w")) == NULL) {
		fprintf(stderr, "can't open %s\n", fileName.c_str());
		exit(1);
	}
	
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = WIDTH; 	/* image width and height, in pixels */
	cinfo.image_height = HEIGHT;
	cinfo.input_components = 4;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGBA_8888; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	
	jpeg_start_compress(&cinfo, TRUE);

	row_pointer[0] = image;
	if (jpeg_write_scanlines(&cinfo, row_pointer, 0) != cinfo.image_height)
		abort_jpeg_error((jpeg_common_struct*)&cinfo, "Incomplete encoding result");

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

		/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	 *    * loop counter, so that we don't have to keep track ourselves.
	 *       * To keep things simple, we pass one scanline per call; you can pass
	 *          * more if you wish, though.
	 *             */
	//row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

	//while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 *      * Here the array is only one element long, but you could pass
		 *           * more than one scanline at a time if that's more convenient.
		 *                */
		//row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		//(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);

	}

	void abort_jpeg_error(j_common_ptr info, const char *msg)
	{
		jpeg_abort(info);
		throw std::runtime_error(msg);
	}

	void my_error_exit(j_common_ptr info)
	{
		char buffer[JMSG_LENGTH_MAX];
		info->err->format_message(info, buffer);
		abort_jpeg_error(info, buffer);
	}
