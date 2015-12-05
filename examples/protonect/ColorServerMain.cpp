
// includes
#include <iostream>
#include <signal.h>

#include <opencv2/opencv.hpp>
#include <opencv2/contrib/contrib.hpp>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/threading.h>

#include <networking/KinectServer.hpp>

#include <CUDA/Filter.h>
#include <libfreenect2/Timer.hpp>


// constant defines
#define PORT "3490"
#define ROWS 1080 // 1080p
#define COLS 1920 // 1080p

#define TIMER_WINDOW_SIZE 100

// globals
bool protonect_shutdown = false;

// signal handlers
void sigint_handler(int s)
{
	protonect_shutdown = true;
}

// print usage aux function
void printProgramUsage(char* programName)
{
	std::cout << "Usage: " << programName << " timeToRun" << std::endl;
}


int main(int argc, char *argv[])
{
	// process input parameters
	if (argc != 2)
	{
		printProgramUsage(argv[0]);
		return -1;
	}

	int timeToRun = atoi(argv[1]);

	// initalize device and frame listener objects
	libfreenect2::Freenect2 freenect2;
	libfreenect2::Freenect2Device *dev = freenect2.openDefaultDevice();

	if(dev == 0)
	{
		std::cout << "no device connected or failure opening the default one!" << std::endl;
		return -1;
	}

	std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
	std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;

	libfreenect2::SyncMultiFrameListener listener(libfreenect2::Frame::Color); //listener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth);
	libfreenect2::FrameMap frames;

	dev->setColorFrameListener(&listener);
	dev->start();

	// initialize global variables
	signal(SIGINT,sigint_handler);
	protonect_shutdown = false;

	// initialize main loop variables	
	double runTime = 0;

	// initialize Timer (for profiling)
	const char* sectionNames[] = { "Acquire Frame", "Compress send to client", "End of Iteration"};
	Timer timer(sectionNames, 3, TIMER_WINDOW_SIZE);

	// launch the server and wait for a client
	KinectServer server(PORT, ROWS, COLS);
	server.WaitForClient();
	
	// main loop
	while(!protonect_shutdown)
	{
		timer.FrameStart();
		
		// obtain frame from libfreenect runtime - 4 bytes per pixel 
		listener.waitForNewFrame(frames);
		libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color]; // rgba
		unsigned char **pprgba = reinterpret_cast<unsigned char **>(rgb->data);
		//cv::Mat bgr(ROWS, COLS, CV_8UC3);
		//cv::cvtColor(rgba, bgr, cv::COLOR_RGBA2BGR);
		timer.SectionEnd();
		
		int numBytesSent = server.SendMatrixCompressedWithJPEG(pprgba[0]);
		timer.SectionEnd();

		protonect_shutdown = protonect_shutdown || (timer.GetCurrentTime() >= timeToRun) || (numBytesSent == 0);

		listener.release(frames);
		timer.SectionEnd();
	}

	// wrap-up
	// TODO: restarting ir stream doesn't work!
	// TODO: bad things will happen, if frame listeners are freed before dev->stop() :(
	dev->stop();
	dev->close();

	server.CloseConnection();
	std::cout << "Server connection closed" << std::endl;

	return 0;
}
