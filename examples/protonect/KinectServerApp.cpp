// includes
#include <iostream>
#include <signal.h>

#include <libfreenect2/threading.h>

#include <libfreenect2/FrameGrabber.hpp>
#include <networking/ChannelProperties.hpp>
#include <networking/NetworkFrameProcessor.hpp>
#include <networking/FrameServer.hpp>
#include <networking/UserInputProcessor.hpp>

#include <libfreenect2/Timer.hpp>

// constant defines
#define PORT "3490"

#define TIMER_WINDOW_SIZE 100

// globals
bool protonect_shutdown = false;

// signal handlers
void sigint_handler(int s)
{
	protonect_shutdown = true;
}

int main(int argc, char *argv[])
{
	// process input arguments
	Networking::UserInputProcessor inputProcessor;
	Networking::ChannelProperties* properties;
	if(inputProcessor.TryParse(argc, (const char**) argv))
	{
		properties = inputProcessor.GetProperties();
	}
	else
	{
		inputProcessor.PrintUsage(argv[0]);
		return -1;
	}

	// initialize global variables
	signal(SIGINT,sigint_handler);
	protonect_shutdown = false;

	// initalize device and frame listener objects
	libfreenect2::FrameGrabber frameGrabber(properties->Type, true);

	// initialize frame processor
	Networking::NetworkFrameProcessor frameProcessor(properties);

	// launch the server
	Networking::FrameServer server(PORT);
	
	// initialize main loop variables	
	double runTime = 0;

	// initialize Timer (for profiling)
	const char* sectionNames[] = { "Acquire Frame", "Process Frame", "Send to client", "End of Iteration"};
	Timer timer(sectionNames, 4, TIMER_WINDOW_SIZE);

	server.WaitForClient();

	// main loop
	while(!protonect_shutdown)
	{
		timer.FrameStart();
		
		// obtain frame from libfreenect runtime - 4 bytes per pixel 
		const libfreenect2::Frame* frame = frameGrabber.GrabFrame(); // rgba
		timer.SectionEnd();

		const Networking::NetworkPacket packet = frameProcessor.ProcessFrame(frame);
		timer.SectionEnd();
		
		bool sentSuccessfully = server.SendPacket(packet);
		timer.SectionEnd();

		frameGrabber.ReleaseFrame();
		protonect_shutdown = protonect_shutdown || (timer.GetCurrentTime() >= properties->TimeToRun) || !sentSuccessfully;		
		timer.SectionEnd();
	}

	// wrap-up

	server.CloseConnection();
	std::cout << "Server connection closed" << std::endl;

	return 0;
}
