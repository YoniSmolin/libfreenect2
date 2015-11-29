// License
/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2011 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */

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
#define ROWS 424
#define COLS 512

#define DEPTH_MIN 500.0f // [mm]
#define DEPTH_MAX 4000.0f // [mm]
#define DEPTH_RESOLUTION 2.5f // [mm]

#define MEDIAN_FILTER_SIZE 3

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

	libfreenect2::SyncMultiFrameListener listener(libfreenect2::Frame::Depth); //listener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth);
	libfreenect2::FrameMap frames;

	dev->setIrAndDepthFrameListener(&listener);
	dev->start();

	// initialize global variables
	signal(SIGINT,sigint_handler);
	protonect_shutdown = false;

	// initialize main loop variables	
	cv::Mat currentDepth;
	cv::Mat depthDenoised(ROWS, COLS, CV_8UC1);
	float depthFiltered[ROWS*COLS];
	double runTime = 0;

	// initialize Timer (for profiling)
	const char* sectionNames[] = { "Acquire Frame", "Filter Raw Frame", "Quantize", "Send to client", "End of Iteration"};
	Timer timer(sectionNames, 5, TIMER_WINDOW_SIZE);

	// launch the server and wait for a client
	KinectServer server(PORT, ROWS, COLS);
	std::cout << "Successfully initialized server" << std::endl;
	server.WaitForClient();
	
	// main loop
	while(!protonect_shutdown)
	{
		timer.FrameStart();
		
		// obtain frame from libfreenect runtime - 4 bytes per pixel - values are floating point in units of [mm]
		listener.waitForNewFrame(frames);
		libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
		cv::Mat depthMat = cv::Mat(depth->height, depth->width, CV_32FC1, depth->data);
		timer.SectionEnd();
		
		// filter the frame in CUDA to remove values outside of the desired range
		FilterGPU((float*)depthMat.data, depthFiltered, depth->height, depth->width, DEPTH_MAX);
		timer.SectionEnd();
		
		// map pixel values to 1-byte values in [0,(2^8)-1]
		depthMat = (cv::Mat(depth->height, depth->width, CV_32FC1, depthFiltered) - DEPTH_MIN ) / (DEPTH_MAX - DEPTH_MIN);
		depthMat.convertTo(currentDepth, CV_16UC1, (DEPTH_MAX - DEPTH_MIN)/DEPTH_RESOLUTION , 0);// 65535, 0); //
		cv::Mat matrixToSend = currentDepth;
		timer.SectionEnd();

		int numBytesSent = server.SendMatrixCompressedWithPNG(matrixToSend.data);
		timer.SectionEnd();

		protonect_shutdown = protonect_shutdown || (timer.GetCurrentTime() >= timeToRun) || !numBytesSent; // shutdown on escape

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
