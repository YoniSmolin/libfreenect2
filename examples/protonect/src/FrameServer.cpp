#include <networking/FrameServer.hpp>
#include <iostream>

namespace Networking
{
	bool FrameServer::SendMetadata(libfreenect2::Frame::Type type)
	{
		unsigned char metadata[MetadataSizeInBytes];
		metadata[0] = (unsigned char) type;

		return SendMessage((const char*) metadata, MetadataSizeInBytes) ? true : false;
	}

	bool FrameServer::SendPacket(const NetworkPacket& packet)
	{
		// prepare the header
		char header[HeaderSizeInBytes];
		header[0] = packet.Size;
		header[1] = packet.Size >> ByteSizeInBits;
		header[2] = packet.Size >> 2*ByteSizeInBits;
		
		// send the timestammp
	        time_t secondsSinceEpoch = packet.Timestamp.tv_sec;
		int sentBytesSeconds = SendMessage((const char*)&secondsSinceEpoch, sizeof(time_t));		
		if (sentBytesSeconds < sizeof(time_t)) return false;

		long residualMilliseconds = round(packet.Timestamp.tv_nsec / 1.0e6);
		int sentBytesMilliseconds = SendMessage((const char*)&residualMilliseconds, sizeof(long));
		if (sentBytesMilliseconds < sizeof(long)) return false;		

		// send the header
		int sentBytesHeader = SendMessage(header, HeaderSizeInBytes);
		if (sentBytesHeader < HeaderSizeInBytes) return false;

		// send the body
		int sentBytesBody = SendMessage((const char*)packet.Data, packet.Size);
		if (sentBytesBody < packet.Size) return false;

		return true;
	}			
}
