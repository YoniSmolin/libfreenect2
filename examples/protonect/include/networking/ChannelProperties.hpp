#ifndef CHANNEL_PROPERTIES_HPP
#define CHANNEL_PROPERTIES_HPP

#include <turbojpeg.h>
#include <libfreenect2/frame_listener.hpp>

// syntax comment: C++ does not require to typedef struct types to remove the 'struct' prefix

namespace Networking
{
	struct ChannelProperties 
	{
		libfreenect2::Frame::Type Type;
		unsigned int ImageWidth;
		unsigned int ImageHeight;
		unsigned int TimeToRun;

		ChannelProperties(libfreenect2::Frame::Type type, unsigned int width, unsigned int height) : Type(type), ImageWidth(width), ImageHeight(height), TimeToRun(10) {}
		virtual ~ChannelProperties() {};
	};

	struct ColorProperties : ChannelProperties
	{
		unsigned int ImageShrinkFactor;  // a positive integer, both dimensions of the input image will be scaled down by which
		unsigned int CompressionQuality; // must be between 1 and 100 (1 being worst quality)
		TJSAMP ChrominanceSubsampling;   // read more in '/usr/include/turbojpeg.h'
		
		ColorProperties() : ChannelProperties(libfreenect2::Frame::Color, 1920, 1080),
				    ImageShrinkFactor(1),
				    CompressionQuality(80),
				    ChrominanceSubsampling(TJSAMP_420) {}
	};

	struct IrProperties : ChannelProperties
	{
		float InputSaturationLevel; // any value above this in the IR input will be saturated
		float ProcessedMaxValue;    // this is the maximum value for IR on the network channel 
		IrProperties() : ChannelProperties(libfreenect2::Frame::Ir, 512, 424),
				 InputSaturationLevel(20000),
				 ProcessedMaxValue(255) {}
	};

	struct DepthProperties : ChannelProperties
	{
		float NearThreshold; // mm
		float FarThreshold;  // mm 
	 	float Resolution;   // mm (this is not the image dimensions, this is the resolution of depth values on the channel)	
		
		DepthProperties() : ChannelProperties(libfreenect2::Frame::Depth, 512, 424),
				    NearThreshold(500),
				    FarThreshold(4000),
			            Resolution(2.5f) {}
	};
}

#endif
