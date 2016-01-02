#ifndef NETWORK_FRAME_PROCESSOR_HPP
#define NETWORK_FRAME_PROCESSOR_HPP

#include <libfreenect2/frame_listener.hpp>
#include <networking/ChannelProperties.hpp>
#include <networking/JpegCompressor.hpp>

#include <vector>
#include <map>
#include <time.h>

namespace Networking 
{
	struct NetworkPacket
	{
		libfreenect2::Frame::Type Type;
		size_t Size;
		const char* Data;
		struct timespec Timestamp;
	};

	// The class manages its own memory, releases everything upon destruction
	class NetworkFrameProcessor
	{
		ChannelProperties* _properties;
		JpegCompressor* _colorCompressor; // jpeg compressor for the color channel
		std::vector<unsigned char> _compressedPNG;// contains compressed PNG for the depth/IR compression

		public:
			NetworkFrameProcessor(ChannelProperties* properties);

			NetworkPacket ProcessFrame(const libfreenect2::Frame* frame);

			~NetworkFrameProcessor();

		private:
			NetworkPacket ProcessColorFrame(const libfreenect2::Frame* frame);
			NetworkPacket ProcessIrFrame(const libfreenect2::Frame* frame);
			NetworkPacket ProcessDepthFrame(const libfreenect2::Frame* frame);
	};
}

#endif
