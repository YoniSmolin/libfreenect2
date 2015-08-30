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


#include <iostream>
#include <signal.h>

#include <opencv2/opencv.hpp>
#include <opencv2/contrib/contrib.hpp>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/threading.h>

#include <networking/DepthServer.hpp>

#include <CUDA/Filter.h>

#define PORT "3490"
#define ROWS 424
#define COLS 512

#define DEPTH_MIN 500.0f // [mm]
#define DEPTH_MAX 3050.0f // [mm]

bool protonect_shutdown = false;

void sigint_handler(int s)
{
	protonect_shutdown = true;
}

double timing_acc;
double timing_acc_n;

double timing_current_start;


void startTiming()
{
	timing_current_start = cv::getTickCount();
}

void stopTiming()
{
	timing_acc += (cv::getTickCount() - timing_current_start) / cv::getTickFrequency();
	timing_acc_n += 1.0;

	if(timing_acc_n >= 100.0)
	{
		double avg = (timing_acc / timing_acc_n);
		std::cout << "[FPS] avg. time: " << (avg * 1000) << "ms -> ~" << (1.0/avg) << "Hz" << std::endl;
		timing_acc = 0.0;
		timing_acc_n = 0.0;
	}
}


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << "timeToRun [-c]" << std::endl;
		return -1;
	}

	int timeToRun = atoi(argv[1]);

	bool useCompression = false;
	if (argc == 3)
		useCompression = true;	
	
	std::string program_path(argv[0]);
	size_t executable_name_idx = program_path.rfind("Protonect");

	std::string binpath = "/";

	if(executable_name_idx != std::string::npos)
	{
		binpath = program_path.substr(0, executable_name_idx);
	}


	libfreenect2::Freenect2 freenect2;
	libfreenect2::Freenect2Device *dev = freenect2.openDefaultDevice();

	if(dev == 0)
	{
		std::cout << "no device connected or failure opening the default one!" << std::endl;
		return -1;
	}

	timing_acc = 0.0;
	timing_acc_n = 0.0;
	timing_current_start = 0.0;

	signal(SIGINT,sigint_handler);
	protonect_shutdown = false;

	libfreenect2::SyncMultiFrameListener listener(libfreenect2::Frame::Depth); //listener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth);
	libfreenect2::FrameMap frames;

	cv::moveWindow("Server",585,624) ;

	dev->setIrAndDepthFrameListener(&listener);
	dev->start();

	std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
	std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;

	cv::Mat currentDepth;
	float depthFiltered[ROWS*COLS];

	DepthServer server(PORT, useCompression, ROWS, COLS);
	std::cout << "Successfully initialized server" << std::endl;
	server.WaitForClient();
	double runTime = 0;

	while(!protonect_shutdown)
	{
		startTiming() ;
		listener.waitForNewFrame(frames);
		libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

		cv::Mat depthMat = cv::Mat(depth->height, depth->width, CV_32FC1, depth->data);

		FilterGPU((float*)depthMat.data, depthFiltered, depth->height, depth->width, DEPTH_MAX);

		depthMat = (cv::Mat(depth->height, depth->width, CV_32FC1, depthFiltered) - DEPTH_MIN ) / (DEPTH_MAX - DEPTH_MIN);
		depthMat.convertTo(currentDepth, CV_8UC1, 255, 0);
	
		int numBytesSent = server.SendMatrix(currentDepth.data);

		protonect_shutdown = protonect_shutdown || (runTime >= timeToRun) || !numBytesSent; // shutdown on escape

		listener.release(frames);
		stopTiming();

		runTime += (cv::getTickCount() - timing_current_start) / cv::getTickFrequency();

	}

	// TODO: restarting ir stream doesn't work!
	// TODO: bad things will happen, if frame listeners are freed before dev->stop() :(
	dev->stop();
	dev->close();

	server.CloseConnection();
	std::cout << "Server connection closed" << std::endl;

	return 0;
}
