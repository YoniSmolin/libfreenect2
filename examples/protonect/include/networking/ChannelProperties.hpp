#ifndef CHANNEL_PROPERTIES_HPP
#define CHANNEL_PROPERTIES_HPP

#include <turbojpeg.h>
#include <libfreenect2/frame_listener.hpp>
#include <opencv2/highgui/highgui.hpp>

// syntax comment: C++ does not require to typedef struct types to remove the 'struct' prefix

namespace Networking
{
	extern const std::map<int, unsigned int> PixelTypeToPixelMaxValue;

	struct ChannelProperties 
	{
		libfreenect2::Frame::Type Type;
		unsigned int ImageWidth;
		unsigned int ImageHeight;
		unsigned int TimeToRun;
		int PixelType;

		ChannelProperties(libfreenect2::Frame::Type type, unsigned int width, unsigned int height, int pixelType) :
						 Type(type), ImageWidth(width), ImageHeight(height), TimeToRun(10), PixelType(pixelType) {}
		virtual ~ChannelProperties() {};
	};

	struct ColorProperties : ChannelProperties
	{
		unsigned int ImageShrinkFactor;  // a positive integer, both dimensions of the input image will be scaled down by which
		unsigned int CompressionQuality; // must be between 1 and 100 (1 being worst quality)
		TJSAMP ChrominanceSubsampling;   // read more in '/usr/include/turbojpeg.h'
		
		ColorProperties() : ChannelProperties(libfreenect2::Frame::Color, 1920, 1080, CV_8UC4),
				    ImageShrinkFactor(1),
				    CompressionQuality(80),
				    ChrominanceSubsampling(TJSAMP_420) {}
	};

	struct IrProperties : ChannelProperties
	{
		float InputSaturationLevel; // any value above this in the IR input will be saturated
		IrProperties() : ChannelProperties(libfreenect2::Frame::Ir, 512, 424, CV_8UC1),
				 InputSaturationLevel(20000) {}
	};

	struct DepthProperties : ChannelProperties
	{
		float NearThreshold; // mm
		float FarThreshold;  // mm 
	 	float Resolution;   // mm (this is not the image dimensions, this is the resolution of depth values on the channel)	
		
		DepthProperties() : ChannelProperties(libfreenect2::Frame::Depth, 512, 424, CV_16UC1),
				    NearThreshold(500),
				    FarThreshold(4000),
			            Resolution(2.5f) {}
	};
}

#endif
