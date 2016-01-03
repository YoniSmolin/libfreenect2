#include <iostream> // cannot compile libfreenect2.hpp without this (which is included by the next line of code)
#include <libfreenect2/FrameGrabber.hpp>
#include <stdexcept>

namespace libfreenect2
{
	FrameGrabber::FrameGrabber(Frame::Type channelType, bool verbose) : _type(channelType), _frameListener(channelType), _verbose(verbose)
	{
		_kinectDevice = _freenect2.openDefaultDevice();

		if (_kinectDevice == 0) throw std::runtime_error("Could not initialize Kinect2 device");

		_type == Frame::Color ? _kinectDevice->setColorFrameListener(&_frameListener) : _kinectDevice->setIrAndDepthFrameListener(&_frameListener);
		_kinectDevice->start();

		if (_verbose)
		{
			std::cout << "device serial: " << _kinectDevice->getSerialNumber() << std::endl;
			std::cout << "device firmware: " << _kinectDevice->getFirmwareVersion() << std::endl;
			PrintIntrinsics();
		}
	}

	const Frame* FrameGrabber::GrabFrame()
	{
		_frameListener.waitForNewFrame(_frameMap);
		return _frameMap[_type];
	}
	
	void FrameGrabber::ReleaseFrame()
	{
		_frameListener.release(_frameMap);
	}

	FrameGrabber::~FrameGrabber()
	{
		// TODO: restarting ir stream doesn't work!
		// TODO: bad things will happen, if frame listeners are freed before dev->stop() :(
		_kinectDevice->stop();
		_kinectDevice->close();
	}
	
	void FrameGrabber::PrintIntrinsics()
	{
		if (_type == Frame::Color)
		{
			Freenect2Device::ColorCameraParams intrinsics =  _kinectDevice->getColorCameraParams();
			std::cout << "Color camera intrinsics:" << std::endl;
			std::cout << "fx: " << intrinsics.fx << std::endl;
			std::cout << "fy: " << intrinsics.fy << std::endl;
			std::cout << "cx: " << intrinsics.cx << std::endl;
			std::cout << "cy: " << intrinsics.cy << std::endl;
		}
		else
		{
			Freenect2Device::IrCameraParams intrinsics = _kinectDevice->getIrCameraParams();
			std::cout << "IR camera intrinsics:" << std::endl;
			std::cout << "fx: " << intrinsics.fx << std::endl;
			std::cout << "fy: " << intrinsics.fy << std::endl;
			std::cout << "cx: " << intrinsics.cx << std::endl;
			std::cout << "cy: " << intrinsics.cy << std::endl;
			std::cout << "k1: " << intrinsics.k1 << std::endl;
			std::cout << "k2: " << intrinsics.k2 << std::endl;
			std::cout << "k3: " << intrinsics.k3 << std::endl;
			std::cout << "p1: " << intrinsics.p1 << std::endl;
			std::cout << "p2: " << intrinsics.p2 << std::endl;
		}
	}
}
