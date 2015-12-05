#define tegra_accelerate

#include <stdio.h>	
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <cstdlib>
#include <turbojpeg.h>

#define FILE_NAME "MyImage.jpeg"
#define OUTPUT_FILE_NAME "Recompressed4.jpeg"

#define WIDTH  1920
#define HEIGHT 1080

typedef unsigned char uchar;

uchar* decompress(std::string filename);
long unsigned int compress(uchar* uncompressed, uchar*& compressed);

long readFileToBuffer(std::string fileName, uchar*& buffer);
long writeBufferToFile(uchar* buffer, unsigned long int bufferSize, std::string fileName);

int main()
{
	// prepare jpeg decompression objects
	std::cout << "Starting Jpeg example" << std::endl;

	// load the image and decode
	uchar* decompressed = decompress(FILE_NAME);
	if (decompressed == NULL) return 0;

	std::cout << "Successfullly loaded and decompressed the image: " << FILE_NAME << std::endl;

	cv::Mat image(HEIGHT, WIDTH, CV_8UC3, decompressed);

	// display the result
	cv::imshow(FILE_NAME, image);
	cv::waitKey(0);

	uchar* recompressed = NULL;

	unsigned long int bufferSize = compress(decompressed, recompressed);

	writeBufferToFile(recompressed, bufferSize, OUTPUT_FILE_NAME);

	delete[] decompressed;
	delete[] recompressed;

	return 0;
}

uchar* decompress(std::string fileName)
{
	uchar* compressed = NULL;
	long compressedLength = readFileToBuffer(fileName, compressed);
	if (compressedLength == 0 || compressed == NULL) return NULL;

	std::cout << "Successfully loaded file: " << fileName << std::endl;

	tjhandle decompressor = tjInitDecompress();
	if(decompressor == 0)
	{
		std::cerr << "failed to initialize turbojpeg decompressor! turbojpeg error: '" << tjGetErrorStr() << "'" << std::endl;
		return NULL;
	}
	
	uchar* decompressed = new uchar[WIDTH * HEIGHT * tjPixelSize[TJPF_BGR]]; // rgb => 3 bytes per pixel

	int r = tjDecompress2(decompressor, compressed, compressedLength, decompressed, WIDTH, WIDTH * tjPixelSize[TJPF_BGR], HEIGHT, TJPF_BGR, 0);

	if (r != 0)
	{
	      std::cerr << "failed to decompress rgb image! turbojpeg error: '" << tjGetErrorStr() << "'" << std::endl;
	      return NULL;
	}

	delete[] compressed;	

	return decompressed;
}

long unsigned int compress(uchar* uncompressed, uchar*& compressed)
{
	if (compressed != NULL) return 0;

	tjhandle compressor = tjInitCompress();
	compressed = new uchar[tjBufSize(WIDTH, HEIGHT, TJSAMP_420)];
	long unsigned int compressedSize = 0;

	tjCompress2(compressor, uncompressed, WIDTH, WIDTH * tjPixelSize[TJPF_BGR], HEIGHT, TJPF_BGR, &compressed, &compressedSize, TJSAMP_420, 25, TJFLAG_NOREALLOC);

	return compressedSize;
}

long readFileToBuffer(std::string fileName, uchar*& buffer)
{
	// read the image from file into a buffer
	FILE* inFile;
	
	if ((inFile = fopen(fileName.c_str(), "rb")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", fileName.c_str());
		return 0;
	}

	fseek(inFile, 0, SEEK_END); // go to file end
	long fileSize = ftell(inFile);
	fseek(inFile, 0, SEEK_SET); // go to file start

	buffer = new uchar[fileSize];

	size_t bytesRead = fread(buffer, 1, fileSize, inFile);
	
	fclose(inFile);
	
	return bytesRead;
}


long writeBufferToFile(uchar* buffer, long unsigned int bufferSize, std::string fileName)
{
	FILE* outFile;
	
	outFile = fopen(fileName.c_str(), "w");

	if (outFile == NULL)
	{
		fprintf(stderr, "can't open %s", fileName.c_str());
		return 0;	
	}
	
	size_t bytesWritten = fwrite(buffer, 1, bufferSize, outFile);

	fclose(outFile);

	return bytesWritten;
}
