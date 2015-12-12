#ifndef FRAME_SERVER_HPP
#define FRAME_SERVER_HPP

#include <networking/Server.hpp>
#include <networking/NetworkFrameProcessor.hpp>

namespace Networking
{
	// The class does not manage any dynamically allocated memory
	class FrameServer : public Server
	{
		const unsigned int HeaderSizeInBytes;				
		const unsigned int ByteSizeInBits;

		public:
			FrameServer(const char* portNumber) : Server(portNumber), HeaderSizeInBytes(3), ByteSizeInBits(8) {}
			
			bool SendPacket(const NetworkPacket& packet);
	};
}

#endif
