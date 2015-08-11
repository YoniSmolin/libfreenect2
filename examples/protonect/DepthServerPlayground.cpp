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

#include <networking/Server.hpp>

#include <CUDA/Filter.h>

#define PORT "3490"
#define DEPTH_THRESHOLD (uchar) 100

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
		std::cout << "Usage: DepthServer <depth threshold in [mm]>" << std::endl;
		return -1;
	}

	float depthThreshold = strtof(argv[1], NULL);
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

	cv::moveWindow("Server",584,624) ;
	float depthFiltered[512*424]; 

	dev->setIrAndDepthFrameListener(&listener);
	dev->start();

	std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
	std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
	cv::Mat depthConverted ;

	Server server(PORT);
	std::cout << "Successfully initialized server" << std::endl;
	server.WaitForClient();

	int frameCount = 0;

	while(!protonect_shutdown)
	{
		startTiming() ;
		listener.waitForNewFrame(frames);
		libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

		cv::Mat depthMat = cv::Mat(depth->height, depth->width, CV_32FC1, depth->data);
		FilterGPU((float*)depthMat.data, depthFiltered, depth->height, depth->width, depthThreshold);
		depthMat = cv::Mat(depth->height, depth->width, CV_32FC1, depthFiltered) / 4500.0f;
		depthMat.convertTo(depthConverted,CV_8UC1,255,0);
		server.SendMatrix((char*)depthConverted.data, depth->height, depth->width);  

		frameCount++;
		protonect_shutdown = protonect_shutdown || (frameCount > 100); // shutdown on escape

		listener.release(frames);
		stopTiming() ;
	}

	// TODO: restarting ir stream doesn't work!
	// TODO: bad things will happen, if frame listeners are freed before dev->stop() :(
	dev->stop();
	dev->close();

	server.CloseConnection();
	std::cout << "Server connection closed" << std::endl;

	return 0;
}
