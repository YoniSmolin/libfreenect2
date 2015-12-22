#include <networking/ChannelProperties.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Networking
{
	std::map<int, unsigned int> CreateMap()
	{
		std::map<int, unsigned int> result;
		result[CV_8UC1] = 255;
		result[CV_16UC1] = 65535;
		return result;
	}

	const std::map<int, unsigned int> PixelTypeToPixelMaxValue = CreateMap();
}
