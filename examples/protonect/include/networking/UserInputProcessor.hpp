#ifndef USER_INPUT_PROCESSOR_HPP
#define USER_INPUT_PROCESSOR_HPP

#include <networking/ChannelProperties.hpp>
#include <libfreenect2/frame_listener.hpp>

namespace Networking
{
	// This class manages its own memory
	class UserInputProcessor
	{
		ChannelProperties* _properties;

		public:
			UserInputProcessor() : _properties(NULL) {}

			bool TryParse(int argc, const char** argv);	
			void PrintUsage(const char* programName);
			ChannelProperties* GetProperties(); // call after TryParse()

			~UserInputProcessor();
		private:
			
			bool TryParseColor(int argc, const char** argv);
			bool TryParseIr(int argc, const char** argv);
			bool TryParseDepth(int argc, const char** argv);
	};
}

#endif
