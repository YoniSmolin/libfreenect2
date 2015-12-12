#include <networking/FrameServer.hpp>

namespace Networking
{
	bool FrameServer::SendPacket(const NetworkPacket& packet)
	{
		// prepare the header
		char header[HeaderSizeInBytes];
		header[0] = packet.Size;
		header[1] = packet.Size >> ByteSizeInBits;
		header[2] = packet.Size >> 2*ByteSizeInBits;

		// send the header
		int sentBytesHeader = SendMessage(header, HeaderSizeInBytes);
		if (sentBytesHeader < HeaderSizeInBytes) return false;

		// send the PNG body
		int sentBytesBody = SendMessage((const char*)packet.Data, packet.Size);
		if (sentBytesBody < packet.Size) return false;

		return true;
	}			
}
